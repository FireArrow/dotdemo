#include <stdio.h>
#include <SDL2/SDL.h>
#include "ddclientlib/ddclient.h"
#include "drawcircle.h"

int run( SDL_Window* window, SDL_Renderer* renderer, int ddclientfd ) {

    char done = 0,
         dots_updated = 0;

    int i, j,
        x, y,
        no_dots,
        seqnr;

    float laser_point_buf[MAX_POINTS][2];

    char netByf[SEND_BUF_SIZE];

    SDL_Event event;

    while( !done ) {

        // Get input from dotdetector
        no_dots = getDots( ddclientfd, &laser_point_buf[0][0], &seqnr );

        // Get input from SDL
        while( SDL_PollEvent( &event ) ) {
            switch( event.type ) {
                case SDL_QUIT:
                    done = 1;
                    break;

                case SDL_KEYDOWN:
                    switch( event.key.keysym.sym ) {
                        case SDLK_ESCAPE:
                            done = 1;
                            break;
                    }
            }
        }

        // Clear the image
        SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
        SDL_RenderClear( renderer );

        // Draw laser points to screen
        SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0xB0, 0xFF );
        for( i=0; i<dots; ++i ) {
            x = (int)laser_point_buf[i][0];
            y = (int)laser_point_buf[i][1];
            SDL_Render_DrawPoint( 
                    renerer,
                    x,
                    y
                    );
            draw_circle( renderer, x, y, 10 );
        }

    }
    return 0;
}

int main( int argc, char** argv ) {
    int ret = 0; // Return value of the entire program
    int ddclientfd; // Socket for dotdetector

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    // Init the dotdetector client
    ddclientfd = initDDclient( "unimplemented", 10001 );

    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        fprintf( stderr, "ERROR: SDL init failed. Message: %s\n", SDL_GetError() );
        ret = -1;
        goto error;
    }

    if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "linear" ) ) {
        fprintf( stderr, "WARNING: SDL render scale hinting failed: Message%s\n", SDL_GetError() );
    }

    window = SDL_CreateWindow( "Dotdetector demo",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            640,
            480,
            SDL_WINDOW_FULLSCREEN_DESKTOP
            );
    if( window == NULL ) {
        printf( stderr, "ERROR: Failed to create SDL window. Message: %s\n", SDL_GetError() );
        ret = -2;
        goto error;
    }

    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
    if( renderer == NULL ) {
        fprintf( stderr, "ERROR: Renderer could not be created. Message: %s\n", SDL_GetError() );
        ret = -3;
        goto error;
    }

    SDL_RenderSetLogicalSize( renderer, 640, 480 );

    // Start the game loop
    ret = run( window, renderer, ddclientfd );

error:
    //Clean up SDL stuffs
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();

    return ret;
}
