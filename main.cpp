#include <iostream>
#include "libs/SDL2/include/SDL.h"
#include "libs/SDL2_image/include/SDL_image.h"

const int SCREEN_WIDTH = 1080;
const int SCREEN_HEIGHT = 720;

bool init();
bool loadMedia();
void close();

SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Surface* gImage = NULL;

bool init(){
    bool success = true;
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        success = false;
    } else {
        SDL_Window* gWindow = SDL_CreateWindow("SGRUN", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, NULL);
        if(gWindow == NULL){
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            success = false;
        } else {
            gScreenSurface = SDL_GetWindowSurface(gWindow);
        }
    }
    return success;
}

bool loadMedia(){
    bool success = true;
    gImage = IMG_Load("img/idle.png");
    if(gImage == NULL){
        printf( "Unable to load image %s! SDL Error: %s\n", "02_getting_an_image_on_the_screen/hello_world.bmp", SDL_GetError() );
        success = false;
    }
    return success;
}

void close(){
    SDL_FreeSurface(gImage);
    gImage = NULL;

    SDL_DestroyWindow( gWindow );
    gWindow = NULL;

    SDL_Quit();
}

int main( int argc, char* args[] ){

    if( !init() ){
        printf( "Failed to initialize!\n" );
    }else{
        if( !loadMedia() ){
            printf( "Failed to load media!\n" );
        }else{
            SDL_BlitSurface(gImage, NULL, gScreenSurface, NULL);
            SDL_UpdateWindowSurface( gWindow );
            SDL_Event e;
            bool quit = false;
            while( quit == false ){
                while( SDL_PollEvent( &e ) ){
                    if( e.type == SDL_QUIT ){
                        quit = true;
                    }
                }
            }
        }
    }
    
    close();

	return 0;
}