#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>
#include <vector>
#include <time.h>

sf::Vector2f getPixelPosition(
    const sf::Vector2i& pos,
    const sf::Vector2u& blockSize) {
    return sf::Vector2f(float(pos.x * blockSize.x), float(pos.y * blockSize.y));
}

int main() {
    const int fieldWidth = 6;
    const int fieldHeight = 10;

    srand((unsigned int)time(NULL));
    bool field[fieldWidth][fieldHeight] = {};  // the place status
    for (int y = 3; y < fieldHeight; y++) {
        for (int k = 1; k <= 3; k++) {  // draw obstacle
            field[rand() % fieldWidth][y] = true;
        }
    }
    // [F, F, F, F, F, F]
    // [F, F, F, F, F, F]
    // [F, F, F, F, F, F]
    // [F, F, F, F, F, F]
    // [F, F, F, F, F, F]
    // [F, F, F, F, F, F]
    // [F, F, F, F, F, F]
    // [F, F, F, F, F, F]
    // [F, F, F, F, F, F]
    // [F, F, F, F, F, F]
    sf::Texture blockTecture;
    if (!blockTecture.loadFromFile("block.png")) {
        std::cout << "block.png is not found." << std::endl;
        return EXIT_FAILURE;
    }
    sf::Sprite block(blockTecture);
    sf::Vector2u blockSize(blockTecture.getSize());
    const int windowWidth = blockSize.x * fieldWidth;
    const int windowHeight = blockSize.y * fieldHeight;
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cout << "arial.ttf is not found." << std::endl;
        return EXIT_FAILURE;
    }

    enum class GameState { RUNNING, PAUSE, LOSE };
    GameState gameState = GameState::RUNNING;
    sf::Text text;
    text.setFont(font);
    sf::String title("Line Match Game");
    sf::VideoMode mode(windowWidth, windowHeight);
    sf::RenderWindow window(mode, title);
    sf::Vector2i origin(fieldWidth / 2, 0);
    sf::Vector2i pos(origin);  // origin position
    block.setPosition(getPixelPosition(pos, blockSize));
    sf::Clock clock;
    float timer = 0, delay = 0.5;

    bool noBlock = false;
    bool gameOver = false;
    while (window.isOpen()) {
        float time = clock.getElapsedTime().asSeconds();  // time start
        clock.restart();  // time end
        timer += time;  // the time from time start to end

        enum class Action {
            Hold,
            MoveLeft,
            MoveRight,
            MoveDown,
            QuickDown,
        };
        Action action = Action::Hold;
        sf::Event evt;
        if (window.pollEvent(evt)) {
            if (evt.type == sf::Event::Closed) {
                window.close();
            }
            if (evt.type == sf::Event::KeyPressed) {
                switch (evt.key.code) {
                case sf::Keyboard::Left:
                    action = Action::MoveLeft;
                    break;
                case sf::Keyboard::Right:
                    action = Action::MoveRight;
                    break;
                case sf::Keyboard::Down:
                    action = Action::MoveDown;
                    break;
                case sf::Keyboard::Space:
                    action = Action::QuickDown;
                    break;
                case sf::Keyboard::P:
                case sf::Keyboard::Pause:
                    if (gameState == GameState::RUNNING) {
                        gameState = GameState::PAUSE;
                    }
                    else if (gameState == GameState::PAUSE) {
                        gameState = GameState::RUNNING;
                    }
                    break;
                }
            }
        }

        //if (clock.getElapsedTime().asSeconds() >= 0.5f) {
        //    action = Action::MoveDown;
        //    clock.restart();
        //}
        if (gameState != GameState::PAUSE && gameState != GameState::LOSE) {
            if (timer > delay) {
                action = Action::MoveDown;
                timer = 0;
            }
        }

        // check next position
        sf::Vector2i nextPos = pos;  // next position cover the origin position
        if (gameState != GameState::PAUSE && gameState != GameState::LOSE) {  // when game paused or game over, block cannot move
            // check the block position this round
            switch (action) {
            case Action::Hold:
                break;
            case Action::MoveLeft:
                // Set the left and right borders are connected to each other
                if (nextPos.x <= 0) {
                    nextPos.x = fieldWidth;
                }
                // move left
                nextPos.x--;
                break;
            case Action::MoveRight:
                // Set the left and right borders are connected to each other
                if (nextPos.x >= fieldWidth - 1) {
                    nextPos.x = -1;
                }
                // move right
                nextPos.x++;
                break;
            case Action::MoveDown:
                nextPos.y++;
                break;
            case Action::QuickDown:
                std::vector<int> vec;
                std::vector<int> new_vec;
                for (int x = 0; x < fieldWidth; x++) {
                    for (int y = 0; y < fieldHeight; y++) {
                        if (field[pos.x][y] == true) {
                            vec.push_back(y);
                            for (size_t n = 0; n < vec.size(); n++) {
                                if (pos.y < vec[n]) {
                                    new_vec.push_back(vec[n]);
                                    int obstacle = *std::min_element(new_vec.begin(), new_vec.end());
                                    nextPos.y = obstacle - 1;
                                }
                            }
                        }
                        if (vec.empty()) {
                            nextPos.y = fieldHeight - 1;
                        }
                    }
                }
                //std::cout << "(" << nextPos.x << ", " << nextPos.y << ")" << std::endl;
                break;
            }
        }

        // Check if the next position can place the block
        // If the block cannot be placed, the position will not change
        // The next block cannot be placed over bounds
        if (nextPos.x >= 0 && nextPos.x < fieldWidth &&
            nextPos.y < fieldHeight &&
            field[nextPos.x][nextPos.y] == false) {
            pos = nextPos;  // if next position cannot place block, the position will not change
        }
        else {
            if (action == Action::MoveDown || action == Action::QuickDown) {
                if (pos.x >= 0 && pos.x < fieldWidth &&
                    pos.y >= 0 && pos.y < fieldHeight) {
                    field[pos.x][pos.y] = true;  // record status
                }
                // check row is full
                bool isFull = true;
                for (int x = 0; x < fieldWidth; x++) {
                    if (pos.x >= 0 && pos.x < fieldWidth &&
                        pos.y >= 0 && pos.y < fieldHeight) {
                        if (field[x][pos.y] == false) {
                            isFull = false;
                        }
                    }
                }
                if (isFull) {
                    // Move all rows above pos.y down one cell 
                    // [F, F, F, F, F, F]
                    // [F, F, F, F, F, F]
                    // [F, F, F, F, F, F]
                    // [F, F, F, F, F, F]
                    // [T, T, F, T, F, F]            // [F, F, F, F, F, F]
                    // [T, T, T, T, T, T] <= pos.y   // [T, T, F, T, F, F]
                    // [F, F, F, F, F, F]
                    // [F, F, F, F, F, F]
                    // [F, F, F, F, F, F]
                    // [F, F, F, F, F, F]
                    for (int y = pos.y; y > 0; y--) {
                        for (int x = 0; x < fieldWidth; x++) {
                            field[x][y] = field[x][y - 1];
                        }
                    }
                    // the toppest row will reset to zero
                    for (int x = 0; x < fieldWidth; x++) {
                        field[x][0] = false;
                    }

                    // count block exist
                    int count = 0;
                    for (int y = fieldHeight; y > 0; y--) {
                        for (int x = 0; x < fieldWidth; x++) {
                            if (field[x][y] == true) {
                                count++;
                            }
                        }
                    }
                    // check if clear all line
                    if (count == 0) {
                        noBlock = true;
                    }
                }
                pos = origin;
            }
        }

        if (noBlock) {
            window.close();
        }

        for (int x = 0; x < fieldWidth; x++) {
            if (field[x][0] == true) {
                gameState = GameState::LOSE;
            }
        }

        switch (gameState) {
        case GameState::RUNNING:
            break;
        case GameState::PAUSE:
            clock.restart();
            break;
        case GameState::LOSE:
            text.setString("Game Over!");
            text.setCharacterSize(30);
            text.setStyle(sf::Text::Bold);
            text.setFillColor(sf::Color(25, 25, 112));
            text.setPosition(5.f, 5.f);
            break;
        }

        if (gameState == GameState::LOSE) {
            window.clear(sf::Color(255, 0, 0));
        } else {
            window.clear();
        }

        // current controled block
        block.setPosition(getPixelPosition(pos, blockSize));
        window.draw(block);

        // draw place
        for (int x = 0; x < fieldWidth; x++) {
            for (int y = 0; y < fieldHeight; y++) {
                if (field[x][y] == true) {
                    sf::Vector2i p(x, y);
                    block.setPosition(getPixelPosition(p, blockSize));
                    window.draw(block);
                }
            }
        }
        window.draw(text);
        window.display();
    }
    return EXIT_SUCCESS;
}