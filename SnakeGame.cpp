#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <time.h>

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const int GRID_SIZE = 20;
const int GAME_SPEED = 150;

int snakeX[100], snakeY[100];
int snakeLength;
int fruitX, fruitY;
int score;
int direction;
bool gameOver;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* backgroundTexture = NULL;
SDL_Texture* fruitTexture = NULL;
SDL_Texture* headTextures[4] = {NULL};
SDL_Texture* bodyTextures[6] = {NULL};
SDL_Texture* tailTextures[4] = {NULL};

SDL_Texture* loadTexture(const char* path) {
    SDL_Surface* loadedSurface = IMG_Load(path);
    if (loadedSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError());
        return NULL;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    return texture;
}

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    backgroundTexture = loadTexture("Graphics/background.png");
    fruitTexture = loadTexture("Graphics/apple.png");
    headTextures[0] = loadTexture("Graphics/head_up.png");
    headTextures[1] = loadTexture("Graphics/head_right.png");
    headTextures[2] = loadTexture("Graphics/head_down.png");
    headTextures[3] = loadTexture("Graphics/head_left.png");
    bodyTextures[0] = loadTexture("Graphics/body_horizontal.png");
    bodyTextures[1] = loadTexture("Graphics/body_vertical.png");
    bodyTextures[2] = loadTexture("Graphics/body_topleft.png");
    bodyTextures[3] = loadTexture("Graphics/body_topright.png");
    bodyTextures[4] = loadTexture("Graphics/body_bottomleft.png");
    bodyTextures[5] = loadTexture("Graphics/body_bottomright.png");
    tailTextures[0] = loadTexture("Graphics/tail_up.png");
    tailTextures[1] = loadTexture("Graphics/tail_right.png");
    tailTextures[2] = loadTexture("Graphics/tail_down.png");
    tailTextures[3] = loadTexture("Graphics/tail_left.png");

    if (!backgroundTexture || !fruitTexture) {
        return false;
    }
    for (int i = 0; i < 4; ++i) {
        if (!headTextures[i] || !tailTextures[i]) {
            return false;
        }
    }
    for (int i = 0; i < 6; ++i) {
        if (!bodyTextures[i]) {
            return false;
        }
    }

    return true;
}

void close() {
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(fruitTexture);
    for (int i = 0; i < 4; ++i) {
        SDL_DestroyTexture(headTextures[i]);
        SDL_DestroyTexture(tailTextures[i]);
    }
    for (int i = 0; i < 6; ++i) {
        SDL_DestroyTexture(bodyTextures[i]);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

void handleInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            gameOver = true;
        } else if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_UP: if (direction != 2) direction = 0; break;
                case SDLK_DOWN: if (direction != 0) direction = 2; break;
                case SDLK_LEFT: if (direction != 1) direction = 3; break;
                case SDLK_RIGHT: if (direction != 3) direction = 1; break;
            }
        }
    }
}

void generateFruit() {
    fruitX = rand() % (WINDOW_WIDTH / GRID_SIZE);
    fruitY = rand() % (WINDOW_HEIGHT / GRID_SIZE);
}

void moveSnake() {
    for (int i = snakeLength; i > 0; --i) {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
    }

    switch (direction) {
        case 0: snakeY[0] -= 1; break;
        case 1: snakeX[0] += 1; break;
        case 2: snakeY[0] += 1; break;
        case 3: snakeX[0] -= 1; break;
    }

    if (snakeX[0] < 0 || snakeX[0] >= WINDOW_WIDTH / GRID_SIZE || snakeY[0] < 0 || snakeY[0] >= WINDOW_HEIGHT / GRID_SIZE) {
        gameOver = true;
    }

    for (int i = 1; i < snakeLength; ++i) {
        if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
            gameOver = true;
        }
    }

    if (snakeX[0] == fruitX && snakeY[0] == fruitY) {
        score += 1;
        snakeLength++;
        generateFruit();
    }
}

void render() {
    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

    for (int i = 0; i < snakeLength; ++i) {
        SDL_Rect rect = {snakeX[i] * GRID_SIZE, snakeY[i] * GRID_SIZE, GRID_SIZE, GRID_SIZE};
        if (i == 0) {
            SDL_RenderCopy(renderer, headTextures[direction], NULL, &rect);
        } else if (i == snakeLength - 1) {
            int tailDir = (snakeX[i - 1] < snakeX[i]) ? 3 : (snakeX[i - 1] > snakeX[i]) ? 1 : (snakeY[i - 1] < snakeY[i]) ? 0 : 2;
            SDL_RenderCopy(renderer, tailTextures[tailDir], NULL, &rect);
        } else {
            SDL_RenderCopy(renderer, bodyTextures[0], NULL, &rect);  // Simplified
        }
    }

    SDL_Rect fruitRect = {fruitX * GRID_SIZE, fruitY * GRID_SIZE, GRID_SIZE, GRID_SIZE};
    SDL_RenderCopy(renderer, fruitTexture, NULL, &fruitRect);

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    srand(time(NULL));

    if (!init()) {
        return 1;
    }

    snakeX[0] = WINDOW_WIDTH / (2 * GRID_SIZE);
    snakeY[0] = WINDOW_HEIGHT / (2 * GRID_SIZE);
    snakeLength = 1;
    direction = -1;
    score = 0;
    gameOver = false;

    generateFruit();

    Uint32 lastTime = SDL_GetTicks();

    while (!gameOver) {
        Uint32 currentTime = SDL_GetTicks();
        Uint32 deltaTime = currentTime - lastTime;

        if (deltaTime >= GAME_SPEED) {
            handleInput();
            if (direction != -1) {
                moveSnake();
            }
            render();
            lastTime = currentTime;
        }
    }

    close();
    return 0;
}
