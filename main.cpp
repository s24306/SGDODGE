#include <iostream>
#include "libs/SDL2/include/SDL.h"
#include "Classes/LTexture.h"

const int SCREEN_WIDTH = 1080;
const int SCREEN_HEIGHT = 720;

bool init();
bool loadMedia();
void close();

SDL_Window* gWindow = NULL;
SDL_Surface* gScreenSurface = NULL;
SDL_Surface* gImage = NULL;
SDL_Surface* gCurrentSurface = NULL;

SDL_Rect gSpriteClips[ 4 ];
LTexture gSpriteSheetTexture;



bool init(){
    bool success = true;
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        success = false;
    } else {
        gWindow = SDL_CreateWindow("SGRUN", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if(gWindow == NULL){
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            success = false;
        } else{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
			}
        }
    }
    return success;
}

SDL_Surface* loadSurface( std::string path )
{
    SDL_Surface* loadedSurface = SDL_LoadBMP( path.c_str() );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
    }

    return loadedSurface;
}

bool LTexture::loadFromFile( std::string path )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = loadSurface( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), SDL_GetError() );
	}
	else
	{
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load sprite sheet texture
	if( !gSpriteSheetTexture.loadFromFile( "img/walking_sprite.bmp" ) )
	{
		printf( "Failed to load sprite sheet texture!\n" );
		success = false;
	}
	else
	{
		//Set first sprite
		gSpriteClips[ 0 ].x =   0;
		gSpriteClips[ 0 ].y =   0;
		gSpriteClips[ 0 ].w = gSpriteSheetTexture.getWidth()/4;
		gSpriteClips[ 0 ].h = gSpriteSheetTexture.getHeight();

		//Set second sprite
		gSpriteClips[ 1 ].x = gSpriteSheetTexture.getWidth()/4;
		gSpriteClips[ 1 ].y =   0;
		gSpriteClips[ 1 ].w = gSpriteSheetTexture.getWidth()/4;
		gSpriteClips[ 1 ].h = gSpriteSheetTexture.getHeight();
		
		//Set third sprite
		gSpriteClips[ 2 ].x = (gSpriteSheetTexture.getWidth()/4)*2;
		gSpriteClips[ 2 ].y = 0;
		gSpriteClips[ 2 ].w = gSpriteSheetTexture.getWidth()/4;
		gSpriteClips[ 2 ].h = gSpriteSheetTexture.getHeight();

		//Set fourth sprite
		gSpriteClips[ 3 ].x = (gSpriteSheetTexture.getWidth()/4)*3;
		gSpriteClips[ 3 ].y = 0;
		gSpriteClips[ 3 ].w = gSpriteSheetTexture.getWidth()/4;
		gSpriteClips[ 3 ].h = gSpriteSheetTexture.getHeight();
	}

	return success;
}

void close(){

    //Free loaded images
	gSpriteSheetTexture.free();

	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
    SDL_FreeSurface(gImage);
    gImage = NULL;
	gWindow = NULL;
	gRenderer = NULL;

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
                    } else if(e.type == SDL_QUIT){

                    }
                }
                //Clear screen
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
				SDL_RenderClear( gRenderer );

				//Render top left sprite
				gSpriteSheetTexture.render( 0, 0, &gSpriteClips[ 0 ] );

				//Render top right sprite
				gSpriteSheetTexture.render( SCREEN_WIDTH - gSpriteClips[ 1 ].w, 0, &gSpriteClips[ 1 ] );

				//Render bottom left sprite
				gSpriteSheetTexture.render( 0, SCREEN_HEIGHT - gSpriteClips[ 2 ].h, &gSpriteClips[ 2 ] );

				//Render bottom right sprite
				gSpriteSheetTexture.render( SCREEN_WIDTH - gSpriteClips[ 3 ].w, SCREEN_HEIGHT - gSpriteClips[ 3 ].h, &gSpriteClips[ 3 ] );

				//Update screen
				SDL_RenderPresent( gRenderer );
            }
        }
    }
    
    close();

	return 0;
}