#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "ddclientlib/ddclient.h"

#define pattern_size 180
#define tri_offset 30
#define screen_w 1366
#define screen_h 768

void transformBase( float* v, const int from_dim, const int to_dim ) {
    *v = ( *v * (float)to_dim ) / (float)from_dim;
}

void transformDots( float* laser_point_buf, int no_dots ) {
    int i;
    for( i=0; i<no_dots*2; i += 2 ) {
        transformBase( &laser_point_buf[i], 640, screen_w ); // x
        transformBase( &laser_point_buf[i+1], 480, screen_h ); // y
    }
}

void drawCalibrationPattern( SDL_Renderer* renderer ) {
    int i, j;

    SDL_Rect rects[4] = {
        { 0                      , 0                      , pattern_size, pattern_size },
        { screen_w - pattern_size, 0                      , pattern_size, pattern_size },
        { screen_w - pattern_size, screen_h - pattern_size, pattern_size, pattern_size },
        { 0                      , screen_h - pattern_size, pattern_size, pattern_size }
    };
    static Sint16 triangles_x[4][3] = 
    {
        { // Top left trinagle
            tri_offset                              ,
            pattern_size - tri_offset               ,
            tri_offset
        },
        { // Top right triangle
            screen_w + tri_offset - pattern_size    ,
            screen_w - tri_offset                   ,
            screen_w - tri_offset                   ,
        },
        { // Bottom right tringle
            screen_w + tri_offset - pattern_size    ,
            screen_w - tri_offset                   ,
            screen_w - tri_offset                   ,
        },
        { // Bottom left triangle
            tri_offset                              ,
            tri_offset                              ,
            pattern_size - tri_offset               ,
        }
    };

    static Sint16 triangles_y[4][3] = {
        { // Top left trinagle
            tri_offset                              ,
            tri_offset                              ,
            pattern_size - tri_offset
        },
        { // Top right triangle
            tri_offset                              ,
            tri_offset                              ,
            pattern_size - tri_offset
        },
        { // Bottom right tringle
            screen_h - tri_offset                   ,
            screen_h + tri_offset - pattern_size    ,
            screen_h - tri_offset
        },
        { // Bottom left triangle
            screen_h + tri_offset - pattern_size    ,
            screen_h - tri_offset                   ,
            screen_h - tri_offset
        }
    };
    SDL_RenderFillRects( renderer, rects, 4 );
    for( i=0; i<4; ++i ) {
        filledPolygonRGBA( renderer, &triangles_x[i][0], &triangles_y[i][0], 3, 0x00, 0x00, 0x00, 0xAF );
    }
    
}

/**
 * Main loop
 */
int run( SDL_Window* window, SDL_Renderer* renderer, int ddclientfd ) {

    char done = 0,
         show_calibrate = 0,
         dots_updated = 0,
         draw_mode = 0;

    int i, j,
        x, y,
        no_dots,
        seqnr;

    float laser_point_buf[MAX_POINTS][2];

    char netByf[SEND_BUF_SIZE];

    SDL_Event event;

    while( !done ) {

        // Clear the image
        if( !draw_mode ) {
            SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
            SDL_RenderClear( renderer );
        }

        // Get input from dotdetector
        no_dots = getDots( ddclientfd, &laser_point_buf[0][0], &dots_updated, &seqnr );
        if( dots_updated ) {
            transformDots( &laser_point_buf[0][0], no_dots );
        }

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

                        case SDLK_c:
                            show_calibrate = ~show_calibrate;
                            break;

                        case SDLK_d:
                            draw_mode = ~draw_mode;
                            break;
                    }
            }
        }

        // Draw calibration pattern
        if( show_calibrate ) {
            SDL_SetRenderDrawColor( renderer, 0xA0, 0xA0, 0xA0, 0xFF );
            drawCalibrationPattern( renderer );
        }

        // Draw laser points to frame
        SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0xB0, 0xFF );
        for( i=0; i<no_dots; ++i ) {
            x = (int)laser_point_buf[i][0];
            y = (int)laser_point_buf[i][1];
            filledCircleRGBA( 
                    renderer,
                    x,
                    y,
                    10,
                    rand() % 0xFF,          // R
                    rand() % 0xFF,          // G
                    rand() % 0xFF,          // B
                    10 + (rand() % 0x66)    // A
                    );
        }

        // Draw frame to screen
        SDL_RenderPresent( renderer );

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
            screen_w,
            screen_h,
            SDL_WINDOW_FULLSCREEN_DESKTOP
            );
    if( window == NULL ) {
        fprintf( stderr, "ERROR: Failed to create SDL window. Message: %s\n", SDL_GetError() );
        ret = -2;
        goto error;
    }

    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
    if( renderer == NULL ) {
        fprintf( stderr, "ERROR: Renderer could not be created. Message: %s\n", SDL_GetError() );
        ret = -3;
        goto error;
    }

    SDL_RenderSetLogicalSize( renderer, screen_w, screen_h );

    // Start the game loop
    ret = run( window, renderer, ddclientfd );

error:
    //Clean up SDL stuffs
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();

    return ret;
}
