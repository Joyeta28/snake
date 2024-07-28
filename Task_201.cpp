#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <ctime>
#include <string>

using namespace std;

const int SCREEN_WIDTH = 800;    
const int SCREEN_HEIGHT = 600;   
const int CELL_SIZE = 20;
const int OBSTACLE_SIZE = 40;   

enum class Direction { UP, DOWN, LEFT, RIGHT };

struct SnakeSegment {
    int x, y;
};

struct Fruit {
    int x, y;
};

struct Obstacle {
    int x, y, w, h;
};

void initSDL();
void closeSDL();
void render();
void handleEvents();
void update();
bool checkCollision(int x, int y);
bool checkObstacleCollision(int x, int y);
void generateFruit();
void updateScoreTexture();

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;
SDL_Texture* scoreTexture = nullptr;
int score = 0;

vector<SnakeSegment> snake;
Direction snakeDirection = Direction::RIGHT;
bool quit = false;

Fruit fruit;
vector<Obstacle> obstacles;

int main(int argc, char* argv[]) {
    srand(static_cast<unsigned>(time(0)));
    initSDL();

    snake.push_back({SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2});
    snake.push_back({SCREEN_WIDTH / 2 - CELL_SIZE, SCREEN_HEIGHT / 2});
    snake.push_back({SCREEN_WIDTH / 2 - 2 * CELL_SIZE, SCREEN_HEIGHT / 2});

    for (int x = 0; x < SCREEN_WIDTH; x += OBSTACLE_SIZE) {
        obstacles.push_back({x, 0, OBSTACLE_SIZE, OBSTACLE_SIZE});  
        obstacles.push_back({x, SCREEN_HEIGHT - OBSTACLE_SIZE, OBSTACLE_SIZE, OBSTACLE_SIZE});  
    }
    for (int y = OBSTACLE_SIZE; y < SCREEN_HEIGHT - OBSTACLE_SIZE; y += OBSTACLE_SIZE) {
        obstacles.push_back({0, y, OBSTACLE_SIZE, OBSTACLE_SIZE});  
        obstacles.push_back({SCREEN_WIDTH - OBSTACLE_SIZE, y, OBSTACLE_SIZE, OBSTACLE_SIZE}); 
    }

    generateFruit();
    updateScoreTexture();

    while (!quit) {
        handleEvents();
        update();
        render();
        SDL_Delay(100);
    }

    closeSDL();
    return 0;
}

void initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << endl;
        exit(1);
    }

    window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        cerr << "Window could not be created! SDL Error: " << SDL_GetError() << endl;
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << endl;
        exit(1);
    }

    if (TTF_Init() == -1) {
        cerr << "SDL_ttf could not initialize! TTF Error: " << TTF_GetError() << endl;
        exit(1);
    }

    font = TTF_OpenFont("Playground.ttf", 28);
    if (font == nullptr) {
        cerr << "Failed to load font! TTF Error: " << TTF_GetError() << endl;
        exit(1);
    }
}

void closeSDL() {
    SDL_DestroyTexture(scoreTexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (const auto& obstacle : obstacles) {
        SDL_Rect rect = {obstacle.x, obstacle.y, obstacle.w, obstacle.h};
        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect fruitRect = {fruit.x, fruit.y, CELL_SIZE, CELL_SIZE};
    SDL_RenderFillRect(renderer, &fruitRect);

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (const auto& segment : snake) {
        SDL_Rect rect = {segment.x, segment.y, CELL_SIZE, CELL_SIZE};
        SDL_RenderFillRect(renderer, &rect);
    }

    int texW = SCREEN_WIDTH/2;
    int texH = SCREEN_HEIGHT/2;
    SDL_QueryTexture(scoreTexture, nullptr, nullptr, &texW, &texH);
    SDL_Rect dstrect = {5, 5, texW, texH};
    SDL_RenderCopy(renderer, scoreTexture, nullptr, &dstrect);

    SDL_RenderPresent(renderer);
}

void handleEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
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
            }
        }
    }
}

void update() {
    int nextX = snake[0].x;
    int nextY = snake[0].y;
    switch (snakeDirection) {
        case Direction::UP:
            nextY -= CELL_SIZE;
            break;
        case Direction::DOWN:
            nextY += CELL_SIZE;
            break;
        case Direction::LEFT:
            nextX -= CELL_SIZE;
            break;
        case Direction::RIGHT:
            nextX += CELL_SIZE;
            break;
    }

    if (checkObstacleCollision(nextX, nextY) || checkCollision(nextX, nextY)) {
        quit = true; 
        return;
    }

    snake.insert(snake.begin(), {nextX, nextY});

    if (nextX == fruit.x && nextY == fruit.y) {
        generateFruit();
        score++;
        cout << "Score: " << score << endl;
        updateScoreTexture();
    } else {
        snake.pop_back();
    }
}

bool checkCollision(int x, int y) {
    for (size_t i = 1; i < snake.size(); ++i) {
        if (snake[i].x == x && snake[i].y == y) {
            return true;
        }
    }
    return false;
}

bool checkObstacleCollision(int x, int y) {
    for (const auto& obstacle : obstacles) {
        if (x >= obstacle.x && x < obstacle.x + obstacle.w && y >= obstacle.y && y < obstacle.y + obstacle.h) {
            return true;
        }
    }
    return false;
}

void generateFruit() {
    bool collision;
    do {
        collision = false;
        fruit.x = rand() % (SCREEN_WIDTH / CELL_SIZE) * CELL_SIZE;
        fruit.y = rand() % (SCREEN_HEIGHT / CELL_SIZE) * CELL_SIZE;
        if (checkObstacleCollision(fruit.x, fruit.y) || checkCollision(fruit.x, fruit.y)) {
            collision = true;
        }
    } while (collision);
}

void updateScoreTexture() {
    SDL_DestroyTexture(scoreTexture);
    SDL_Color textColor = {255, 0, 0, 255};
    string scoreText = "Score: " + to_string(score);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText.c_str(), textColor);
    if (textSurface == nullptr) {
        cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << endl;
        return;
    }
    scoreTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (scoreTexture == nullptr) {
        cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << endl;
    }
    SDL_FreeSurface(textSurface);
}
