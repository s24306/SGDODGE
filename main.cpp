#ifdef _WIN32
#define SDL_MAIN_HANDLED
#endif
#include "libs/SDL2/include/SDL.h"
#include "libs/SDL2_ttf/SDL_ttf.h"
#include "libs/SDL2_mixer/include/SDL_mixer.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <ctime>
#include <cmath>
#include <string>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int PLAYER_SIZE = 64;
const int ATTACK_SIZE = 128;
const int ATTACK_SPAWN_INTERVAL = 2000; // milliseconds
const int ANIMATION_SPEED = 100; // milliseconds per frame
const int ATTACK_CHANGE_INTERVAL = 15000; // milliseconds
const SDL_Color TEXT_COLOR = {255, 255, 255, 255}; // Black
const SDL_Color RESTART_BUTTON_COLOR = {169, 169, 169, 255}; // Grey
const int JOYSTICK_DEAD_ZONE = 8000; //Analog joystick dead zone

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* gFont = NULL;
SDL_Texture* gBackgroundTexture = NULL;
Mix_Music* gBackgroundMusic = NULL;
SDL_Joystick* gGameController = NULL;

int survivalTime = 0;

struct GameObject {
    int x, y;
    int size;
    int velX, velY;
    int frame;
    SDL_Texture* texture;
    SDL_RendererFlip flip; // Flip state for rendering
};

bool init();
bool loadMedia(GameObject& player, SDL_Texture*& walkTexture, int& walkFrames, std::vector<SDL_Texture*>& attackTextures);
void close(std::vector<SDL_Texture*>& attackTextures);
SDL_Texture* loadTexture(const char* path);
SDL_Texture* renderText(const std::string &message, SDL_Color color);
void handleEvents(bool& quit, GameObject& player, bool& gameOver, int& startTime, std::vector<GameObject>& attacks);
void update(GameObject& player, std::vector<GameObject>& attacks, int& lastSpawnTime, bool& gameOver, float attackSpeed, const std::vector<SDL_Texture*>& attackTextures, int elapsedTime);
void render(const GameObject& player, SDL_Texture* walkTexture, int walkFrames, const std::vector<GameObject>& attacks, int elapsedTime, bool gameOver, int survivalTime);
bool checkCollision(const GameObject& a, const GameObject& b);

int main(int argc, char* args[]) {
    if (!init()) {
        printf("Failed to iniialize!\n");
        return -1;
    }

    bool quit = false;
    bool gameOver = false;
    int startTime = SDL_GetTicks();
    GameObject player = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, PLAYER_SIZE, 0, 0, NULL, 0, SDL_FLIP_NONE };
    std::vector<GameObject> attacks;
    std::vector<SDL_Texture*> attackTextures;
    SDL_Texture* walkTexture = NULL;
    int walkFrames = 0;
    int lastSpawnTime = 0;

    if (!loadMedia(player, walkTexture, walkFrames, attackTextures)) {
        printf("Failed to load media!\n");
        return -1;
    }

    int lastFrameTime = 0;
    float attackSpeed = 5;

    // Start playing background music
    if (Mix_PlayingMusic() == 0) {
        Mix_PlayMusic(gBackgroundMusic, -1);
    }

    while (!quit) {
        handleEvents(quit, player, gameOver, startTime, attacks);
        int elapsedTime = SDL_GetTicks() - startTime;

        if (!gameOver) {
            update(player, attacks, lastSpawnTime, gameOver, attackSpeed, attackTextures, elapsedTime);
            attackSpeed = attackSpeed * ((elapsedTime % ATTACK_CHANGE_INTERVAL) == 0 ? 1.2 : 1); // Increase attack speed by 20% every 30 seconds
        }
        render(player, walkTexture, walkFrames, attacks, elapsedTime, gameOver, survivalTime);

        int currentTime = SDL_GetTicks();
        if (currentTime - lastFrameTime > ANIMATION_SPEED) {
            player.frame = (player.frame + 1) % walkFrames;
            lastFrameTime = currentTime;
        }

        SDL_Delay(16);
    }

    close(attackTextures);
    return 0;
}

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        printf("SDL could not iniialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    if (TTF_Init() == -1) {
        printf("SDL_ttf could not iniialize! TTF_Error: %s\n", TTF_GetError());
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not iniialize! Mix_Error: %s\n", Mix_GetError());
        return false;
    }

    gWindow = SDL_CreateWindow("SGDODGE", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (gRenderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    gFont = TTF_OpenFont("fonts/OpenSans-Regular.ttf", 28);
    if (gFont == NULL) {
        printf("Failed to load font! TTF_Error: %s\n", TTF_GetError());
        return false;
    }

    if( SDL_NumJoysticks() < 1 ){
        printf( "Warning: No joysticks connected!\n" );
    } else {
            gGameController = SDL_JoystickOpen( 0 );
            if( gGameController == NULL ){
                printf( "Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError() );
            }
        }

    srand(static_cast<unsigned int>(time(NULL)));
    return true;
}

bool loadMedia(GameObject& player, SDL_Texture*& walkTexture, int& walkFrames, std::vector<SDL_Texture*>& attackTextures) {
    player.texture = loadTexture("img/idle.bmp");
    if (player.texture == NULL) {
        printf("Failed to load idle texture!\n");
        return false;
    }

    walkTexture = loadTexture("img/walking_sprite.bmp");
    if (walkTexture == NULL) {
        printf("Failed to load walk texture!\n");
        return false;
    }

    SDL_QueryTexture(walkTexture, NULL, NULL, &walkFrames, NULL);
    walkFrames /= PLAYER_SIZE;

    // Load attack textures
    for (int i = 1; i <= 3; ++i) {
        char path[32];
        snprintf(path, sizeof(path), "img/attack%d.bmp", i);
        SDL_Texture* attackTexture = loadTexture(path);
        if (attackTexture == NULL) {
            printf("Failed to load attack texture %s!\n", path);
            return false;
        }
        attackTextures.push_back(attackTexture);
    }

    // Load background texture
    gBackgroundTexture = loadTexture("img/grass.bmp");
    if (gBackgroundTexture == NULL) {
        printf("Failed to load background texture!\n");
        return false;
    }

    // Load background music
    gBackgroundMusic = Mix_LoadMUS("audio/backgroundMusic.mp3");
    if (gBackgroundMusic == NULL) {
        printf("Failed to load background music! Mix_Error: %s\n", Mix_GetError());
        return false;
    }

    return true;
}

SDL_Texture* loadTexture(const char* path) {
    SDL_Texture* newTexture = NULL;
    SDL_Surface* loadedSurface = SDL_LoadBMP(path);
    if (loadedSurface == NULL) {
        printf("Unable to load image %s! SDL_Error: %s\n", path, SDL_GetError());
    } else {
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (newTexture == NULL) {
            printf("Unable to create texture from %s! SDL_Error: %s\n", path, SDL_GetError());
        }
        SDL_FreeSurface(loadedSurface);
    }
    return newTexture;
}

SDL_Texture* renderText(const std::string &message, SDL_Color color) {
    SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, message.c_str(), color);
    if (textSurface == NULL) {
        printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        return NULL;
    } else {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
        if (textTexture == NULL) {
            printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
        }
        SDL_FreeSurface(textSurface);
        return textTexture;
    }
}

void close(std::vector<SDL_Texture*>& attackTextures) {
    for (SDL_Texture* texture : attackTextures) {
        SDL_DestroyTexture(texture);
    }

    SDL_DestroyTexture(gBackgroundTexture);
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    TTF_CloseFont(gFont);
    Mix_FreeMusic(gBackgroundMusic);
    SDL_JoystickClose( gGameController );
    gGameController = NULL;
    gBackgroundTexture = NULL;
    gRenderer = NULL;
    gWindow = NULL;
    gFont = NULL;
    gBackgroundMusic = NULL;

    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}

void handleEvents(bool& quit, GameObject& player, bool& gameOver, int& startTime, std::vector<GameObject>& attacks) {
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
        } else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
            bool isKeyDown = (e.type == SDL_KEYDOWN);

            switch (e.key.keysym.sym) {
                case SDLK_UP:
                    player.velY = isKeyDown ? -5 : 0;
                    break;
                case SDLK_DOWN:
                    player.velY = isKeyDown ? 5 : 0;
                    break;
                case SDLK_LEFT:
                    player.velX = isKeyDown ? -5 : 0;
                    player.flip = SDL_FLIP_NONE;
                    break;
                case SDLK_RIGHT:
                    player.velX = isKeyDown ? 5 : 0;
                    player.flip = SDL_FLIP_HORIZONTAL;
                    break;
            }
        } else if( e.type == SDL_JOYAXISMOTION ){
            //Motion on controller 0
            if( e.jaxis.which == 0 ){                        
                //X axis motion
                if( e.jaxis.axis == 0 ){
                    //Left of dead zone
                    if( e.jaxis.value < -JOYSTICK_DEAD_ZONE ){
                        player.velX = -5;
                        player.flip = SDL_FLIP_NONE;
                    }
                    //Right of dead zone
                    else if( e.jaxis.value > JOYSTICK_DEAD_ZONE ){
                        player.velX = 5;
                        player.flip = SDL_FLIP_HORIZONTAL;
                    }
                    else{
                        player.velX = 0;
                    }
                    } else if( e.jaxis.axis == 1 ){
                        //Below of dead zone
                        if( e.jaxis.value < -JOYSTICK_DEAD_ZONE ){
                            player.velY = -5;
                        }
                        //Above of dead zone
                        else if( e.jaxis.value > JOYSTICK_DEAD_ZONE ){
                            player.velY = 5;
                        }
                        else{
                            player.velY = 0;
                        }
                    }
                }
            }else if (gameOver && e.type == SDL_MOUSEBUTTONDOWN) {
            int x, y;
            SDL_GetMouseState(&x, &y);
            if (x > (SCREEN_WIDTH / 2 - 50) && x < (SCREEN_WIDTH / 2 + 50) && y > (SCREEN_HEIGHT / 2 + 30) && y < (SCREEN_HEIGHT / 2 + 70)) {
                gameOver = false;
                player.x = SCREEN_WIDTH / 2;
                player.y = SCREEN_HEIGHT / 2;
                startTime = SDL_GetTicks();
                attacks.clear();
                
                // Restart background music
                Mix_HaltMusic();
                Mix_PlayMusic(gBackgroundMusic, -1);
            }
        }
    }
}

void update(GameObject& player, std::vector<GameObject>& attacks, int& lastSpawnTime, bool& gameOver, float attackSpeed, const std::vector<SDL_Texture*>& attackTextures, int elapsedTime) {
    player.x += player.velX;
    player.y += player.velY;

    //Player leaving window
    if (player.x < 0) player.x = 0;
    if (player.y < 0) player.y = 0;
    if (player.x > SCREEN_WIDTH - PLAYER_SIZE) player.x = SCREEN_WIDTH - PLAYER_SIZE;
    if (player.y > SCREEN_HEIGHT - PLAYER_SIZE) player.y = SCREEN_HEIGHT - PLAYER_SIZE;

    //choose side
    int currentTime = SDL_GetTicks();
    if (currentTime - lastSpawnTime > ATTACK_SPAWN_INTERVAL) {
        GameObject attack;
        int side = rand() % 4;
        switch (side) {
            case 0: // Top
                attack = { rand() % SCREEN_WIDTH, -ATTACK_SIZE, ATTACK_SIZE, 0, 0, NULL, 0, SDL_FLIP_NONE };
                break;
            case 1: // Bottom
                attack = { rand() % SCREEN_WIDTH, SCREEN_HEIGHT, ATTACK_SIZE, 0, 0, NULL, 0, SDL_FLIP_NONE };
                break;
            case 2: // Left
                attack = { -ATTACK_SIZE, rand() % SCREEN_HEIGHT, ATTACK_SIZE, 0, 0, NULL, 0, SDL_FLIP_NONE };
                break;
            case 3: // Right
                attack = { SCREEN_WIDTH, rand() % SCREEN_HEIGHT, ATTACK_SIZE, 0, 0, NULL, 0, SDL_FLIP_NONE };
                break;
        }

        //Pathfinding between Player and Attack
        double angle = atan2(player.y - attack.y, player.x - attack.x);
        attack.velX = static_cast<int>(attackSpeed * cos(angle));
        attack.velY = static_cast<int>(attackSpeed * sin(angle));

        // Assign the attack texture based on elapsed time
        attack.texture = attackTextures[(elapsedTime / ATTACK_CHANGE_INTERVAL) % attackTextures.size()];

        attacks.push_back(attack);
        lastSpawnTime = currentTime;
    }

    for (auto i = attacks.begin(); i != attacks.end();) {
        i->x += i->velX;
        i->y += i->velY;

        if (checkCollision(player, *i)) {
            gameOver = true;
            survivalTime = elapsedTime;
            break;
        }

        //Check if out of bounds
        if (i->x < -ATTACK_SIZE || i->y < -ATTACK_SIZE || i->x > SCREEN_WIDTH || i->y > SCREEN_HEIGHT) {
            i = attacks.erase(i);
        } else {
            ++i;
        }
    }
}

void render(const GameObject& player, SDL_Texture* walkTexture, int walkFrames, const std::vector<GameObject>& attacks, int elapsedTime, bool gameOver, int survivalTime) {
    // Render background
    SDL_RenderCopy(gRenderer, gBackgroundTexture, NULL, NULL);

    SDL_Rect srcRect, destRect;
    destRect = { player.x, player.y, PLAYER_SIZE, PLAYER_SIZE };

    if (player.velX != 0 || player.velY != 0) {
        srcRect = { player.frame * PLAYER_SIZE, 0, PLAYER_SIZE, PLAYER_SIZE };
        SDL_RenderCopyEx(gRenderer, walkTexture, &srcRect, &destRect, 0, NULL, player.flip);
    } else {
        SDL_RenderCopyEx(gRenderer, player.texture, NULL, &destRect, 0, NULL, player.flip);
    }

    for (const auto& attack : attacks) {
        SDL_Rect attackRect = { attack.x, attack.y, ATTACK_SIZE, ATTACK_SIZE };
        SDL_RenderCopy(gRenderer, attack.texture, NULL, &attackRect);
    }

    // Render timer
    int seconds = elapsedTime / 1000;
    int minutes = seconds / 60;
    seconds = seconds % 60;
    std::string timerText = std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
    SDL_Texture* timerTexture = renderText(timerText, TEXT_COLOR);
    int textWidth, textHeight;
    SDL_QueryTexture(timerTexture, NULL, NULL, &textWidth, &textHeight);
    SDL_Rect textRect = { 10, 10, textWidth, textHeight };
    SDL_RenderCopy(gRenderer, timerTexture, NULL, &textRect);
    SDL_DestroyTexture(timerTexture);

    if (gameOver) {
        // Render a black screen
        SDL_Rect fillRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
        SDL_RenderFillRect(gRenderer, &fillRect);

        // Render Game Over text
        std::string gameOverText = "Game Over!";
        SDL_Texture* gameOverTexture = renderText(gameOverText, TEXT_COLOR);
        SDL_QueryTexture(gameOverTexture, NULL, NULL, &textWidth, &textHeight);
        textRect = { (SCREEN_WIDTH - textWidth) / 2, (SCREEN_HEIGHT - textHeight) / 2, textWidth, textHeight };
        SDL_RenderCopy(gRenderer, gameOverTexture, NULL, &textRect);
        SDL_DestroyTexture(gameOverTexture);

        // Render Survival Time
        int survivalSeconds = survivalTime / 1000;
        int survivalMinutes = survivalSeconds / 60;
        survivalSeconds = survivalSeconds % 60;
        std::string survivalText = "Survived: " + std::to_string(survivalMinutes) + ":" + (survivalSeconds < 10 ? "0" : "") + std::to_string(survivalSeconds);
        SDL_Texture* survivalTexture = renderText(survivalText, TEXT_COLOR);
        SDL_QueryTexture(survivalTexture, NULL, NULL, &textWidth, &textHeight);
        textRect = { (SCREEN_WIDTH - textWidth) / 2, (SCREEN_HEIGHT - textHeight) / 2 + 30, textWidth, textHeight };
        SDL_RenderCopy(gRenderer, survivalTexture, NULL, &textRect);
        SDL_DestroyTexture(survivalTexture);

        // Render Restart button wih grey background
        std::string restartText = "Restart";
        SDL_Texture* restartTexture = renderText(restartText, TEXT_COLOR);
        SDL_QueryTexture(restartTexture, NULL, NULL, &textWidth, &textHeight);
        SDL_Rect restartRect = { (SCREEN_WIDTH - textWidth) / 2, (SCREEN_HEIGHT - textHeight) / 2 + 60, textWidth, textHeight };
        SDL_Rect restartBgRect = { restartRect.x - 10, restartRect.y + 7, restartRect.w + 20, restartRect.h - 8 };
        SDL_SetRenderDrawColor(gRenderer, RESTART_BUTTON_COLOR.r, RESTART_BUTTON_COLOR.g, RESTART_BUTTON_COLOR.b, RESTART_BUTTON_COLOR.a);
        SDL_RenderFillRect(gRenderer, &restartBgRect);
        SDL_RenderCopy(gRenderer, restartTexture, NULL, &restartRect);
        SDL_DestroyTexture(restartTexture);
    }

    SDL_RenderPresent(gRenderer);
}

bool checkCollision(const GameObject& a, const GameObject& b) {
    int leftA = a.x;
    int rightA = a.x + a.size;
    int topA = a.y;
    int bottomA = a.y + a.size;

    int leftB = b.x;
    int rightB = b.x + b.size;
    int topB = b.y;
    int bottomB = b.y + b.size;

    if (bottomA <= topB) {
        return false;
    }

    if (topA >= bottomB) {
        return false;
    }

    if (rightA <= leftB) {
        return false;
    }

    if (leftA >= rightB) {
        return false;
    }

    return true;
}
