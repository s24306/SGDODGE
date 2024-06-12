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
        gWindow = SDL_CreateWindow("SGRUN", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
    gImage = SDL_LoadBMP("img/idle.bmp");
    if(gImage == NULL){
        printf( "Unable to load image %s! SDL Error: %s\n", "img/idle.png", SDL_GetError() );
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
            SDL_Event e;
            bool quit = false;
            while( quit == false ){
                while( SDL_PollEvent( &e ) ){
                    if( e.type == SDL_QUIT ){
                        quit = true;
                    }
                }
                SDL_BlitSurface(gImage, NULL, gScreenSurface, NULL);
                SDL_UpdateWindowSurface( gWindow );
            }
        }
    }
    
    close();

	return 0;
}