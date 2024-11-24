#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <ctime>

using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int CELL_SIZE = 20;

enum class Direction { UP, DOWN, LEFT, RIGHT };
enum GameState { MENU, LEVEL_MENU, PLAYING, PAUSED, GAME_OVER, EXIT };

struct SnakeSegment {
    int x, y;
};

struct Food {
    int x, y;
    bool isBonus;
};

struct Obstacle {
    int x, y;
    Direction direction;
};

void initSDL();
void closeSDL();
void generateFood(bool isBonus = false);
void render();
void handleEvents();
void update();
bool checkCollision(int x, int y);
void gameOver();
void resetGame(bool showMenu);
void renderText(const std::string& message, int x, int y, SDL_Color color);
void generateObstacles();

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;
Mix_Chunk* eatSound = nullptr;
Mix_Chunk* bonusSound = nullptr;
Mix_Chunk* gameoverSound = nullptr;
Mix_Chunk* bonusAppearSound = nullptr;
SDL_Texture* backgroundTexture = nullptr;
SDL_Texture* snakeBodyTexture = nullptr;
SDL_Texture* fruitTexture = nullptr;
SDL_Texture* bonusFruitTexture = nullptr;
SDL_Texture* obstacleTexture = nullptr;

std::vector<SnakeSegment> snake;
std::vector<Obstacle> obstacles;
Food food;
Direction snakeDirection = Direction::RIGHT;
GameState gameState = MENU;
int score = 0;
int level = 1;
bool quit = false;

int main(int argc, char* argv[]) {
    srand(static_cast<unsigned>(time(0)));
    initSDL();
    resetGame(true);

    while (!quit) {
        handleEvents();
        if (gameState == PLAYING) {
            update();
        }
        if (gameState != EXIT) {
            render();
        }
        SDL_Delay(100);
    }

    closeSDL();
    return 0;
}

void initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        cout<< "SDL could not initialize! SDL Error: " << SDL_GetError() ; cout<<endl;
        exit(1);
    }

    if (TTF_Init() == -1) {
        cout<< "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() ; cout<<endl;
        exit(1);
    }

    if (IMG_Init(IMG_INIT_PNG) == 0) {
        cout<< "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() ; cout<<endl;
        exit(1);
    }

    window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        cout << "Window could not be created! SDL Error: " << SDL_GetError(); cout<< endl;
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        cout<< "Renderer could not be created! SDL Error: " << SDL_GetError() ; cout<< endl;
        exit(1);
    }

    font = TTF_OpenFont("font.ttf", 28);
    if (font == nullptr) {
        cout<<  "Failed to load font! SDL_ttf Error: " << TTF_GetError() ; cout<< endl;
        exit(1);
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
       cout<<  "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() ; cout<< endl;
        exit(1);
    }

    eatSound = Mix_LoadWAV("eating-sound-effect-36186.mp3");
    if (eatSound == nullptr) {
       cout<< "Failed to load sound effect! SDL_mixer Error: " << Mix_GetError() ; cout<< endl;
        exit(1);
    }

    gameoverSound = Mix_LoadWAV("gameover.mp3");
    if (gameoverSound == nullptr) {
        cout<< "Failed to load sound effect! SDL_mixer Error: " << Mix_GetError()  ; cout<< endl;
        exit(1);
    }

    bonusSound = Mix_LoadWAV("bonus_eat.mp3"); 
    if (bonusSound == nullptr) {
       cout<<  "Failed to load sound effect! SDL_mixer Error: " << Mix_GetError() ; cout<< endl;
        exit(1);
    }

    bonusAppearSound = Mix_LoadWAV("bonus.mp3");
    if (bonusAppearSound == nullptr) {
        cout<<  "Failed to load sound effect! SDL_mixer Error: " << Mix_GetError()  ; cout<< endl;
        exit(1);
    }

    SDL_Surface* loadedSurface = IMG_Load("back.png");
    if (loadedSurface == nullptr) {
        cout<<  "Unable to load image! SDL_image Error: " << IMG_GetError();  cout<< endl;
        exit(1);
    }
    backgroundTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    if (backgroundTexture == nullptr) {
        cout<<  "Unable to create texture from image! SDL Error: " << SDL_GetError(); cout<< endl;
        exit(1);
    }

    loadedSurface = IMG_Load("snakebody.jpg");
    if (loadedSurface == nullptr) {
        cout<< "Unable to load image! SDL_image Error: " << IMG_GetError() ; cout<< endl;
        exit(1);
    }
    snakeBodyTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    if (snakeBodyTexture == nullptr) {
       cout<<  "Unable to create texture from image! SDL Error: " << SDL_GetError() ; cout<< endl;
        exit(1);
    }

    loadedSurface = IMG_Load("fruit3.png");
    if (loadedSurface == nullptr) {
       cout<<  "Unable to load image! SDL_image Error: " << IMG_GetError() ; cout<< endl;
        exit(1);
    }
    fruitTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    if (fruitTexture == nullptr) {
        cout<<  "Unable to create texture from image! SDL Error: " << SDL_GetError() ; cout<< endl;
        exit(1);
    }

    loadedSurface = IMG_Load("bonusfruit.png");
    if (loadedSurface == nullptr) {
        cout<<  "Unable to load image! SDL_image Error: " << IMG_GetError() ; cout<< endl;
        exit(1);
    }
    bonusFruitTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    if (bonusFruitTexture == nullptr) {
        cout<<  "Unable to create texture from image! SDL Error: " << SDL_GetError() ; cout<< endl;
        exit(1);
    }

    loadedSurface = IMG_Load("obstacle.jpg");
    if (loadedSurface == nullptr) {
       cout<<  "Unable to load image! SDL_image Error: " << IMG_GetError() ; cout<< endl;
        exit(1);
    }
    obstacleTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    if (obstacleTexture == nullptr) {
       cout<< "Unable to create texture from image! SDL Error: " << SDL_GetError() ; cout<< endl;
        exit(1);
    }
}

void closeSDL() {
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(snakeBodyTexture);
    SDL_DestroyTexture(fruitTexture);
    SDL_DestroyTexture(bonusFruitTexture);
    SDL_DestroyTexture(obstacleTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_FreeChunk(eatSound);
    Mix_FreeChunk(gameoverSound);
    Mix_FreeChunk(bonusSound); 
    Mix_FreeChunk(bonusAppearSound); 
    TTF_CloseFont(font);
    IMG_Quit();
    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}

void generateFood(bool isBonus) {
    bool validPosition = false;
    while (!validPosition) {
        validPosition = true;
        food.x = rand() % (SCREEN_WIDTH / CELL_SIZE) * CELL_SIZE;
        food.y = rand() % (SCREEN_HEIGHT / CELL_SIZE) * CELL_SIZE;
        
        for (const auto &obstacle : obstacles) {
            if (food.x == obstacle.x && food.y == obstacle.y) {
                validPosition = false;
                break;
            }
        }

        for (const auto& segment : snake) {
            if (food.x == segment.x && food.y == segment.y) {
                validPosition = false;
                break;
            }
        }
    }
    food.isBonus = isBonus;
    if (isBonus) {
        Mix_PlayChannel(-1, bonusAppearSound, 0); 
    }
}

void render() {
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);

    SDL_Rect renderQuad = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderCopy(renderer, backgroundTexture, NULL, &renderQuad);

    if (gameState == MENU) {
        renderText(" Snake Game ", SCREEN_WIDTH / 2 - 85, SCREEN_HEIGHT / 2 - 100,{255, 255, 153, 255});
        renderText("Level Selection", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 20,  {255, 255, 153, 255});
        renderText("Quit", SCREEN_WIDTH / 2 - 30, SCREEN_HEIGHT / 2 + 100,  {255, 255, 153, 255});
    }
    else if (gameState == LEVEL_MENU) {
        renderText("Select Level", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 100,  {255, 255, 153, 255});
        renderText("1. Level 1", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, {255, 255, 153, 255});
        renderText("2. Level 2", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2,  {255, 255, 153, 255});
        renderText("Main Menu", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 100,  {255, 255, 153, 255});
    }
    else if (gameState == PLAYING) {
        for (const auto &segment : snake) {
            SDL_Rect fillRect = { segment.x, segment.y, CELL_SIZE, CELL_SIZE };
            SDL_RenderCopy(renderer, snakeBodyTexture, NULL, &fillRect);
        }

        SDL_Rect foodRect = { food.x, food.y, CELL_SIZE, CELL_SIZE };
        if (food.isBonus) {
            SDL_RenderCopy(renderer, bonusFruitTexture, NULL, &foodRect);
        }
        else {
            SDL_RenderCopy(renderer, fruitTexture, NULL, &foodRect);
        }

        for (const auto& obstacle : obstacles) {
            SDL_Rect obstacleRect = { obstacle.x, obstacle.y, CELL_SIZE, CELL_SIZE };
            SDL_RenderCopy(renderer, obstacleTexture, NULL, &obstacleRect);
        }
        renderText("Score: " + std::to_string(score), 10, 10,  {255, 255, 153, 255});
    }
    else if (gameState == GAME_OVER) {
        renderText("Game Over", SCREEN_WIDTH / 2 - 75, SCREEN_HEIGHT / 2 - 100,  {255, 255, 153, 255});
        renderText("Restart", SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2,  {255, 255, 153, 255});
        renderText("Quit", SCREEN_WIDTH / 2 - 45, SCREEN_HEIGHT / 2 + 50, {255, 255, 153, 255});
        renderText("Final Score: " + std::to_string(score), SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, {255, 255, 153, 255});
        renderText("Main Menu", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 100,  {255, 255, 153, 255});
    }

    SDL_RenderPresent(renderer);
}

void handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_q:
                    quit = true;
                    break;
                case SDLK_UP:
                    if (snakeDirection != Direction::DOWN) snakeDirection = Direction::UP;
                    break;
                case SDLK_DOWN:
                    if (snakeDirection != Direction::UP) snakeDirection = Direction::DOWN;
                    break;
                case SDLK_LEFT:
                    if (snakeDirection != Direction::RIGHT) snakeDirection = Direction::LEFT;
                    break;
                case SDLK_RIGHT:
                    if (snakeDirection != Direction::LEFT) snakeDirection = Direction::RIGHT;
                    break;
                case SDLK_p:
                    if (gameState == PLAYING) {
                        gameState = PAUSED;
                    } else if (gameState == PAUSED) {
                        gameState = PLAYING;
                    }
                    break;
            }
        }else if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mouseX = e.button.x;
            int mouseY = e.button.y;

            if (gameState == MENU) {
                 if (mouseX >= SCREEN_WIDTH / 2 - 100 && mouseX <= SCREEN_WIDTH / 2 + 100  &&
                         mouseY >= SCREEN_HEIGHT / 2 + 20 && mouseY <= SCREEN_HEIGHT / 2 + 60) {
                    gameState = LEVEL_MENU;
                }
                else if (mouseX >= SCREEN_WIDTH / 2 - 100 && mouseX <= SCREEN_WIDTH / 2 + 100 &&
                         mouseY >= SCREEN_HEIGHT / 2 + 100 && mouseY <= SCREEN_HEIGHT / 2 + 140) {
                    quit = true;
                }
            } else if (gameState == LEVEL_MENU) {
                if (mouseX >= SCREEN_WIDTH / 2 - 150 && mouseX <= SCREEN_WIDTH / 2 + 150 &&
                    mouseY >= SCREEN_HEIGHT / 2 - 50 && mouseY <= SCREEN_HEIGHT / 2) {
                    level = 1;
                    gameState = PLAYING;
                    resetGame(false);
                }
                else if (mouseX >= SCREEN_WIDTH / 2 - 100 && mouseX <= SCREEN_WIDTH / 2 + 150 &&
                         mouseY >= SCREEN_HEIGHT / 2 + 10 && mouseY <= SCREEN_HEIGHT / 2 + 60) {
                    level = 2;
                    gameState = PLAYING;
                    resetGame(false);
                }
                else if (mouseX >= SCREEN_WIDTH / 2 - 100 && mouseX <= SCREEN_WIDTH / 2 + 150 &&
                         mouseY >= SCREEN_HEIGHT / 2 + 100 && mouseY <= SCREEN_HEIGHT / 2 + 180) {
                    gameState = MENU;
                }
            } else if (gameState == GAME_OVER) {
                if (mouseX >= SCREEN_WIDTH / 2 - 60 && mouseX <= SCREEN_WIDTH / 2 + 120 &&
                    mouseY >= SCREEN_HEIGHT / 2 && mouseY <= SCREEN_HEIGHT / 2 + 50) {
                    gameState = PLAYING;
                    resetGame(false);
                }
                else if (mouseX >= SCREEN_WIDTH / 2 - 50 && mouseX <= SCREEN_WIDTH / 2 + 100 &&
                         mouseY >= SCREEN_HEIGHT / 2 + 60 && mouseY <= SCREEN_HEIGHT / 2 + 90) {
                    quit = true;
                } else if (mouseX >= SCREEN_WIDTH / 2 - 100 && mouseX <= SCREEN_WIDTH / 2 + 150 &&
                         mouseY >= SCREEN_HEIGHT / 2 + 100 && mouseY <= SCREEN_HEIGHT / 2 + 180) {
                    gameState = MENU;
                }
            }
        }
    }
}


void update() {
    SnakeSegment newHead = snake.front();

    switch (snakeDirection) {
        case Direction::UP:
            newHead.y -= CELL_SIZE;
            break;
        case Direction::DOWN:
            newHead.y += CELL_SIZE;
            break;
        case Direction::LEFT:
            newHead.x -= CELL_SIZE;
            break;
        case Direction::RIGHT:
            newHead.x += CELL_SIZE;
            break;
    }

    if (newHead.x < 0 || newHead.x >= SCREEN_WIDTH || newHead.y < 0 || newHead.y >= SCREEN_HEIGHT || checkCollision(newHead.x, newHead.y)) {
        gameOver();
        return;
    }

    snake.insert(snake.begin(), newHead);

    if (newHead.x == food.x && newHead.y == food.y) {
        score += (food.isBonus) ? 5 : 1;
        Mix_PlayChannel(-1, food.isBonus ? bonusSound : eatSound, 0);
        generateFood(rand() % 10 == 0);
    } else {
        snake.pop_back();
    }

    for (auto& obstacle : obstacles) {

        if (newHead.x == obstacle.x && newHead.y == obstacle.y) {
            gameOver();
            return;
        }
    }
}


bool checkCollision(int x, int y) {
    for (const auto &segment : snake) {
        if (segment.x == x && segment.y == y) {
            return true;
        }
    }
    return false;
}

void gameOver() {
    Mix_PlayChannel(-1, gameoverSound, 0);
    gameState = GAME_OVER;
}

void resetGame(bool showMenu) {
    snake.clear();
    snake.push_back({ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 });
    snake.push_back({ SCREEN_WIDTH / 2 - CELL_SIZE, SCREEN_HEIGHT / 2 });
    snake.push_back({ SCREEN_WIDTH / 2 - 2 * CELL_SIZE, SCREEN_HEIGHT / 2 });
    snakeDirection = Direction::RIGHT;
    score = 0;
    obstacles.clear();
    generateObstacles();
    generateFood();
    if (showMenu) {
        gameState = MENU;
    }
}

void renderText(const std::string& message, int x, int y, SDL_Color color) {
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, message.c_str(), color);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect renderQuad = { x, y, textSurface->w, textSurface->h };
    SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void generateObstacles() {
    obstacles.clear(); 

    int numObstacles;
    if (level == 1) {
        numObstacles = 0; 
    } 
    if (level == 2) {
        numObstacles = 10; 
    }

    for (int i = 0; i < numObstacles; i++) {
        Obstacle obstacle;
        bool validPosition = false;
        while (!validPosition) {
            validPosition = true;
            obstacle.x = rand() % (SCREEN_WIDTH / CELL_SIZE) * CELL_SIZE;
            obstacle.y = rand() % (SCREEN_HEIGHT / CELL_SIZE) * CELL_SIZE;

            for (const auto &segment : snake) {
                if (obstacle.x == segment.x && obstacle.y == segment.y) {
                    validPosition = false;
                    break;
                }
            }
            for (const auto& existingObstacle : obstacles) {
                if (obstacle.x == existingObstacle.x && obstacle.y == existingObstacle.y) {
                    validPosition = false;
                    break;
                }
            }
        }

        obstacles.push_back(obstacle);
    }
}