#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL2_framerate.h>
#include "ddclientlib/ddclient.h"

#define verboseOut(...) if(verbose) printf( __VA_ARGS__ )

static int pattern_size = 180;
static int tri_offset = 30;
static int screen_w = 0;
static int screen_h = 0;
static int wind_speed=5;

static char verbose = 0;

typedef struct Dot {
    int x;
    int y;
    int y_speed;
    int x_speed;
    int r,b,g;
    char keep;
} Dot;

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
        filledPolygonRGBA( renderer, &triangles_x[i][0], &triangles_y[i][0], 3, 0x00, 0x00, 0x00, 0xFF );
    }

}

/**
 * Main loop
 */
int run( SDL_Window* window, SDL_Renderer* renderer, int ddclientfd ) {

    char done = 0,
         show_calibrate = 0,
         dots_updated = 0,
         makeItRain=0,
         draw_mode = 0,
         flip_gravity = 0,
         momentum=1,
         wind = 0,
         epilepsy = 0;

    int i, j,
        x, y,
        no_dots,
        seqnr,
        currentX=0,
        previousX=0,
        currentY=0,
        previousY=0,
        x_speed=0,
        dot_size=30;
    int numberOfDots=0;
        
    Dot dotList[1000];     

    float laser_point_buf[MAX_POINTS][2];

    char netByf[SEND_BUF_SIZE];

    SDL_Event event;

    FPSmanager fps;
    SDL_initFramerate( &fps );
    SDL_setFramerate( &fps, 30 );

    for(i=0; i<1000; i++) dotList[i].keep=0;

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
            previousX = currentX;
            previousY = currentY;
            currentY = (int)laser_point_buf[0][1];
            currentX = (int)laser_point_buf[0][0];
        }
        
        if(makeItRain && dots_updated && no_dots > 0) {
            for(j=0; j<no_dots; j++){
            for(i=0; i<1000; i++){
                if(!dotList[i].keep){

                    dotList[i].x = (int)laser_point_buf[0][0];
                    dotList[i].y = (int)laser_point_buf[0][1];
                    dotList[i].y_speed = (currentY-previousY)/2;
                    dotList[i].x_speed = (currentX-previousX)/2;
                    dotList[i].keep=1;
                    dotList[i].r = rand() % 0xFF;
                    dotList[i].b = rand() % 0xFF;
                    dotList[i].g = rand() % 0xFF;
                    ++numberOfDots;
                    
                    if( abs(dotList[i].x_speed) > 100) dotList[i].x_speed = 70;
                    if( abs(dotList[i].y_speed) > 100) dotList[i].y_speed = 70;  
                    //printf("Got dot: %d, %d\n", dotList[i].x, dotList[i].y);
                    printf("Moment: %d\n", dotList[i].x_speed);
                    break;
                }
            }
            }
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
                        case SDLK_r:
                            makeItRain = !makeItRain;
                            break;
                        case SDLK_w:
                            wind = !wind;
                            break;
                        case SDLK_f:
                            flip_gravity = !flip_gravity;
                            break;
                        case SDLK_m:
                            momentum = !momentum;
                            break;    
                        case SDLK_e:
                            epilepsy = !epilepsy;
                            break;    
                        case SDLK_1:
                            dot_size = 10;
                            break;    
                        case SDLK_2:
                            dot_size = 20;
                            break;    
                        case SDLK_3:
                            dot_size = 30;
                            break;    
                    }
            }
        }

        // Draw calibration pattern
        if( show_calibrate ) {
            SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0xFF, 0xFF );
            drawCalibrationPattern( renderer );
        }

        // Draw laser points to frame
        SDL_SetRenderDrawColor( renderer, 0xFF, 0xFF, 0xFF, 0xFF );
        
        //Update position of all "falling" dots, even if we dont draw them.
        for( i=0; i<1000; ++i ) {
            if(dotList[i].keep){
    
                x = dotList[i].x;
    
                if(0>(x-(dot_size/2)) || (x+dot_size/2)>screen_w) dotList[i].x_speed = -dotList[i].x_speed; //Collision detection with edge of screen
    
                if(wind){ dotList[i].x = dotList[i].x + wind_speed;}
                if(momentum){ dotList[i].x = dotList[i].x + dotList[i].x_speed; }
                
                x = dotList[i].x;
                
                y = dotList[i].y += dotList[i].y_speed;
                
                if(makeItRain){
                    //Draw falling dot if enabled
                    if(epilepsy){
                        filledCircleRGBA( 
                                renderer,
                                x,
                                y,
                                dot_size,
                                rand() % 0xFF,          // R
                                rand() % 0xFF,          // G
                                rand() % 0xFF,          // B
                                0xFF                    // A
                        );
                    } else {
                            filledCircleRGBA( 
                                renderer,
                                x,
                                y,
                                dot_size,
                                dotList[i].r % 0xFF,    // R
                                dotList[i].g % 0xFF,    // G
                                dotList[i].b % 0xFF,    // B
                                0xFF                    // A
                        );
                    }  
                }
    //          SDL_RenderDrawPoint( renderer, x, y );
                if(!flip_gravity) dotList[i].y_speed += 2;
                else dotList[i].y_speed -= 2;
                
                //Apply gravity
                if(dotList[i].x_speed > 0) dotList[i].x_speed -= 1;
                if(dotList[i].x_speed < 0) dotList[i].x_speed += 1;                
                
                if(dotList[i].y > screen_h || dotList[i].y < 0){ dotList[i].keep=0;}
            }
        }
        if(!makeItRain){
            for( i=0; i<no_dots; ++i ) {
                x = (int)laser_point_buf[i][0];
                y = (int)laser_point_buf[i][1];
                filledCircleRGBA( 
                        renderer,
                        x,
                        y,
                        dot_size,
                        rand() % 0xFF,          // R
                        rand() % 0xFF,          // G
                        rand() % 0xFF,          // B
                        0xFF                    // A
                        );
    //            SDL_RenderDrawPoint( renderer, x, y );
            }
        }
        
        


        // Draw frame to screen
        SDL_RenderPresent( renderer );
        SDL_framerateDelay( &fps );

    }

    return 0;
}

char* usage_message[] = {
    "Usage:",
    "-p | --port <port> - Selects what port to listen for incommming dots",
    "-d | --display <displayNumber> - Select what display to start on",
    "-r | --resulotion <WidthxHeight> - Set the resulotion manually. Overrides -d",
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

        // What resloution should we use? (overrides -d)
        else if( strcmp( argv[i], "-r" ) == 0 || strcmp( argv[i], "--resolution" ) == 0 ) {
            i += 1;
            if( argv[i] != NULL ) {
                char* c;
                for( c=&argv[i][0]; *c != 'x'; ++c );
                *c = '\0';
                c += 1;
                screen_w = atoi( argv[i] );
                screen_h = atoi( c );
                
            }
            else {
                usage( -8, "Missing resolution (e.g. 1024x768)" );
            }
        }

        // Should we be verbose?
        else if( strcmp( argv[i], "-v" ) == 0 || strcmp( argv[i], "--verbose" ) == 0 ) {
            verbose = 1;
            verboseOut( "Verbose mode activated\n" );
        }
    }

    // Init the dotdetector client
    verboseOut( "Listening for dots on port %d\n", ddlistenport );
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

    if( screen_w == 0 && screen_h == 0 ) {
        screen_w = display_mode.w;
        screen_h = display_mode.h;
    }
    verboseOut( "Using resolution %dx%d\n", screen_w, screen_h );

    verboseOut( "Creating window\n" );
    window = SDL_CreateWindow( "Dotdetector demo",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            screen_w,
            screen_h,
            0
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
