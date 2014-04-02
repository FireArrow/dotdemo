#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "ddclientlib/ddclient.h"

#define verboseOut(...) if(verbose) printf( __VA_ARGS__ )

static int pattern_size = 180;
static int tri_offset = 30;
static int screen_w = 0;
static int screen_h = 0;

static char verbose = 0;

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

void setRect( SDL_Rect* rect, int x, int y, int w, int h ) {
    rect->x = x;
    rect->y = y;
    rect->w = w;
    rect->h = h;
}

void setPoly( Sint16* poly, size_t n, ... ) {
    int i;
    va_list values;
    va_start( values, n );
    for( i=0; i<n; ++i ) {
        poly[i] = (Sint16) va_arg( values, int );
    }
    va_end( values );
}

void drawCalibrationPattern( SDL_Renderer* renderer ) {
    static char first_run = 1;
    int i, j;

    static SDL_Rect rects[4];
    static Sint16 triangles_x[4][3];
    static Sint16 triangles_y[4][3];
    if( first_run ) {
        first_run = 0;
        setRect( &rects[0], 0                      , 0                      , pattern_size, pattern_size );
        setRect( &rects[1], screen_w - pattern_size, 0                      , pattern_size, pattern_size );
        setRect( &rects[2], screen_w - pattern_size, screen_h - pattern_size, pattern_size, pattern_size );
        setRect( &rects[3], 0                      , screen_h - pattern_size, pattern_size, pattern_size );


        setPoly( &triangles_x[0][0], 12,
                // Top left trinagle
                tri_offset                              ,
                pattern_size - tri_offset               ,
                tri_offset                              ,

                // Top right triangle
                screen_w + tri_offset - pattern_size    ,
                screen_w - tri_offset                   ,
                screen_w - tri_offset                   ,

                // Bottom right tringle
                screen_w + tri_offset - pattern_size    ,
                screen_w - tri_offset                   ,
                screen_w - tri_offset                   ,

                // Bottom left triangle
                tri_offset                              ,
                tri_offset                              ,
                pattern_size - tri_offset               
               );

        setPoly( &triangles_y[0][0], 12,
                // Top left trinagle
                tri_offset                              ,
                tri_offset                              ,
                pattern_size - tri_offset               ,

                // Top right triangle
                tri_offset                              ,
                tri_offset                              ,
                pattern_size - tri_offset               ,

                // Bottom right tringle
                screen_h - tri_offset                   ,
                screen_h + tri_offset - pattern_size    ,
                screen_h - tri_offset                   ,

                // Bottom left triangle
                screen_h + tri_offset - pattern_size    ,
                screen_h - tri_offset                   ,
                screen_h - tri_offset
               );
    }

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

char* usage_message[] = {
    "Usage:",
    "-p | --port <port> - Selects what port to listen for incommming dots",
    "-d | --display <displayNumber> - Select what display to start on",
    "-v | --verbose - Turns on verbose messages"
};

void usage( int ret, const char* err_msg ) {
    int i;
    char** line = &usage_message[0];
    printf( "%s\n", err_msg );

    for( i=0; line[i] != NULL; ++i ) {
        printf( "%s\n", line[i] );
    }

    exit( ret );
}

int main( int argc, char** argv ) {
    int ret = 0; // Return value of the entire program
    int i;
    int ddclientfd; // Socket for dotdetector
    int ddlistenport = 10001; // The port to listen for incomming dots
    int displaynumber = 0; // The display to view this on

    SDL_DisplayMode display_mode;

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    // Parse the input arguments
    for( i=1; i<argc; ++i ) {

        // Are we trying to set the port?
        if( strcmp( argv[i], "-p" ) == 0 || strcmp( argv[i], "--port" ) == 0 ) {
            i += 1;
            if( argv[i] != NULL ) {
                ddlistenport = atoi( argv[i] );
                if( ddlistenport <= 0 ) {
                    usage( -5, "Port must be a positive number" );
                }
            }
        }

        // What display should we use?
        else if( strcmp( argv[i], "-d" ) == 0 || strcmp( argv[i], "--display" ) == 0 ) {
            i += 1;
            if( argv[i] != NULL ) {
                displaynumber = atoi( argv[i] );
                if( displaynumber <= 0 ) {
                    usage( -6, "Display must be a positive number" );
                }
            }
            else {
                usage( -7, "Missing display number" );
            }
        }

        // Should we be verbose?
        else if( strcmp( argv[i], "-v" ) == 0 || strcmp( argv[i], "--verbose" ) == 0 ) {
            verbose = 1;
            verboseOut( "Verbose mode activated\n" );
        }
    }

    // Init the dotdetector client
    verboseOut( "Listening for dots on port %d", ddlistenport );
    ddclientfd = initDDclient( "unimplemented", ddlistenport );

    verboseOut( "Initiating SDL\n" );
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        fprintf( stderr, "ERROR: SDL init failed. Message: %s\n", SDL_GetError() );
        ret = -1;
        goto error;
    }

    verboseOut( "Setting scaling to linear\n" );
    if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "linear" ) ) {
        fprintf( stderr, "WARNING: SDL render scale hinting failed: Message: %s\n", SDL_GetError() );
    }

    //Make sure selected display is within range
    verboseOut( "Probing display %d for information\n", displaynumber );
    if( displaynumber >= SDL_GetNumVideoDisplays() ) {
        fprintf( stderr, "Warning: Selected display %d doesn't exist. ", displaynumber );
        displaynumber = SDL_GetNumVideoDisplays() - 1;
        fprintf( stderr, "Using display %d instead\n", displaynumber );
    }

    if( SDL_GetCurrentDisplayMode( displaynumber, &display_mode ) != 0 ) {
        fprintf( stderr, "Error: Couldn't probe display %d for information. Message: %s\n", displaynumber, SDL_GetError() );
        ret = -4;
        goto error;
    }

    screen_w = display_mode.w;
    screen_h = display_mode.h;
    verboseOut( "Using resolution %dx%d\n", screen_w, screen_h );

    verboseOut( "Creating window\n" );
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

    verboseOut( "Creating renderer\n" );
    renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
    if( renderer == NULL ) {
        fprintf( stderr, "ERROR: Renderer could not be created. Message: %s\n", SDL_GetError() );
        ret = -3;
        goto error;
    }

    SDL_RenderSetLogicalSize( renderer, screen_w, screen_h );

    // Start the game loop
    verboseOut( "Starting the main loop\n" );
    ret = run( window, renderer, ddclientfd );

error:

    verboseOut( "Cleaning up SDL\n" );
    //Clean up SDL stuffs
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();

    verboseOut( "Done. Bye bye!\n" );
    return ret;
}
