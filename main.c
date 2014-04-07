#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL2_framerate.h>
#include "ddclientlib/ddclient.h"
#include "ddclientlib/ddhelpers.h"

#define verboseOut(...) if(verbose) printf( __VA_ARGS__ )
#define TRUE 1
#define FALSE 0

<<<<<<< HEAD
#define MAX_BALLS 1000
#define MAX_DOTS 50
#define PI 3.14159265
#define MAX_DECAY 1

=======
#ifndef DD_MAX_DOTS
#define DD_MAX_DOTS 150
#endif

#ifndef MAX_BALLS
#define MAX_BALLS DD_MAX_DOTS * 10
#endif
>>>>>>> c01fb1fa62f1b54bf908d1e12607828c45411c38

static int screen_w = 0;
static int screen_h = 0;
static int wind_speed=5;
static int dot_size=10;
static char verbose = 0;

typedef struct Parameters{
    char momentum;
    char wind;
    char flip_gravity;
} Parameters;

typedef struct Position {
    int x;
    int y;
    char matched;
} Position;

typedef struct Dot {
    int x;
    int y;
    int x_speed;
    int y_speed;
    int r,b,g;
    int decay;
    double angle;
    char keep;
    char matched;
} Dot;

typedef struct Ball {
    int x;
    int y;
    int y_speed;
    int x_speed;
    int r,b,g;
    char keep;
} Ball;

void pruneDots(Dot* dotList){

    int i;
    for(i=0; i<DD_MAX_DOTS; i++){
        dotList[i].keep = FALSE;
        if(dotList[i].matched == TRUE){ 
            dotList[i].keep = TRUE; 
            dotList[i].decay = MAX_DECAY; 
        } else if (dotList[i].decay > 0) {
            dotList[i].keep = TRUE;
            dotList[i].decay -= 1;
        }
        dotList[i].matched = FALSE;
    }

}

int spawnBall(Dot* dot, Ball* ballList){

    int i,j,x,y;

    for(i=0; i<MAX_BALLS; i++){
    
        if(!ballList[i].keep){
        
            ballList[i].x = dot->x;
            ballList[i].y = dot->y;
            ballList[i].keep = TRUE;
            ballList[i].x_speed = dot->x_speed;
            ballList[i].y_speed = dot->y_speed;
            
            ballList[i].r = rand() % 0xFF;
            ballList[i].b = rand() % 0xFF;
            ballList[i].g = rand() % 0xFF;
            break;
        }
    }
}


Dot* addDot(Dot* dotList, Position* positionPointer){

    int i,j;
    
    for(i=0; i<DD_MAX_DOTS; i++){
    
        if(!dotList[i].keep){
        
            dotList[i].x = (int) positionPointer->x;
            dotList[i].y = (int) positionPointer->y;
            dotList[i].matched=TRUE;
            dotList[i].keep=TRUE;
            dotList[i].x_speed=0;
            dotList[i].y_speed=0;
            dotList[i].decay = MAX_DECAY;
            
            dotList[i].r = rand() % 0xFF;
            dotList[i].b = rand() % 0xFF;
            dotList[i].g = rand() % 0xFF;
            return (Dot*) &dotList[i];
        }
    }
}


<<<<<<< HEAD
=======
Dot* matchPosition(Dot* dotList, struct Position* positionPointer){

    char new_dot=TRUE;

    int i,
        x,y,
        distance=9999,
        temp_dist,matchedIndex,
        x_dist, y_dist,
        x_speed, y_speed;

    x = positionPointer->x;  
    y = positionPointer->y;
    
    //printf("Matching, x=%d, y=%d\n", x, y);
    
    //Check if point matches any dot
    for(i=0; i<DD_MAX_DOTS; i++){
        
        if(dotList[i].keep && !dotList[i].matched){ //Check .matched so we only match maximum one point per dot
            
            x_dist = (x-dotList[i].x)^2;
            y_dist = (y-dotList[i].y)^2;
        
            temp_dist = sqrt( abs(x_dist+y_dist) );
            
            if(temp_dist < 15 && temp_dist<distance){ 
            
                distance=temp_dist; 
                matchedIndex=i; 
                new_dot=FALSE;
            }
        }
    }
    
    //Return pointer to the matched dot if we found one
    if(!new_dot){
    
        dotList[matchedIndex].matched=TRUE;
        return &dotList[matchedIndex];
    } 
    
    //Falling through here means make new dot!
    return ( addDot(&dotList[0], positionPointer));
    
}
>>>>>>> c01fb1fa62f1b54bf908d1e12607828c45411c38


void applyForces(Ball* ballList, Parameters physicsParams){

    int i,x;

    for( i=0; i<MAX_BALLS; ++i ) {
        if(ballList[i].keep){

            x = ballList[i].x;

            //Collision detection with edge of screen
            if(0>(x-(dot_size/2)) || (x+dot_size/2)>screen_w) ballList[i].x_speed = -ballList[i].x_speed; 

            //Apply linear force on all balls
            if(physicsParams.wind){ ballList[i].x = ballList[i].x + wind_speed;}    
                      
            //Apply individual momentum
            if(physicsParams.momentum){ 
                ballList[i].x += ballList[i].x_speed; 
                ballList[i].y += ballList[i].y_speed;
            }

            //Apply gravity
            if(physicsParams.flip_gravity) ballList[i].y_speed -= 2;
            else ballList[i].y_speed += 2;
            
            //Apply friction
            if(ballList[i].x_speed > 0) ballList[i].x_speed -= 1;
            if(ballList[i].x_speed < 0) ballList[i].x_speed += 1;      
                      
            //Check if ball has fallen from screen
            if(ballList[i].y > screen_h || ballList[i].y < 0){ ballList[i].keep=FALSE;}
        }
    }
}


void drawBall(Ball* ball, SDL_Renderer* renderer){

                 filledCircleRGBA( renderer, ball->x, ball->y, dot_size,
                                ball->r % 0xFF,          // R
                                ball->g % 0xFF,          // G
                                ball->b % 0xFF,          // B
                                0xFF                     // A
                        );
}


void drawDot(Dot* dot, SDL_Renderer* renderer){

                 filledCircleRGBA( renderer, dot->x, dot->y, dot_size,
                                dot->r % 0xFF,          // R
                                dot->g % 0xFF,          // G
                                dot->b % 0xFF,          // B
                                0xFF                    // A
                        );
}

void drawVector(Dot* dot, SDL_Renderer* r){

SDL_SetRenderDrawColor(r,255,255,255,255);

    SDL_RenderDrawLine(r, dot->x, dot->y, (dot->x+5*dot->x_speed), (dot->y+5*dot->y_speed) );

}

void updateVector(Dot* dot, Position* positionPointer){

    int x_speed, y_speed,
        dot_x=dot->x, dot_y=dot->y,
        pos_x=positionPointer->x, pos_y=positionPointer->y;
    double f1,f2,temp_angle;    
    
    //Calculate new speed
    x_speed = (pos_x - dot_x);
    y_speed = (pos_y - dot_y);    

    f1 = (x_speed);
    f2 = (y_speed);

    //Update dot parameters
    dot->x = pos_x;
    dot->y = pos_y;
    dot->x_speed = x_speed;
    dot->y_speed = y_speed; 
    temp_angle = atan2( f1,f2 )*180 / PI;
    if(temp_angle < 0) temp_angle += 360;
    dot->angle = temp_angle;
    printf("X_speed: %d\n", dot->x_speed);
    printf("Y_speed: %d\n", dot->y_speed);
    printf("Angle: %f\n", dot->angle);
    printf("************************\n");
}

Dot* matchPosition(Dot* dotList, struct Position* positionPointer){

    char new_dot=TRUE;

    int i,
        pos_x,pos_y, 
        dot_x,dot_y,
        matchedIndex, check_distance;
        
    double distance=9999,
        predicted_distance, normal_distance, compare_distance,
        x_dist, y_dist,
        angle_margin=45, vector_lenght;
    double temp_angle,point_angle;

    pos_x = positionPointer->x;  
    pos_y = positionPointer->y;
    
    //printf("Matching, x=%d, y=%d\n", x, y);
    
    //Check if point matches any dot
    for(i=0; i<MAX_DOTS; i++){
        //For each dot
        if(dotList[i].keep && !dotList[i].matched){ //Check .matched so we only match maximum one point per dot
        
            //Get position of current dot
            dot_x = dotList[i].x;
            dot_y = dotList[i].y;
            
            //Get angle between the position and current dot
            point_angle = atan2(pos_x-dot_x , pos_y-dot_y)*180 / PI;
            if(point_angle < 0) temp_angle += 360;
            
            printf("Checking dot X: %d, Y: %d\n", dotList[i].x, dotList[i].y);
            
            //Get distance from predicted dot to position
            x_dist = pow( (pos_x-dotList[i].x-(dotList[i].x_speed)*2), 2);
            y_dist = pow( (pos_y-dotList[i].y-(dotList[i].y_speed)*2), 2);
            predicted_distance = sqrt( abs(x_dist+y_dist) );
            printf("Predicted Distance: %f\n", predicted_distance);
            
            //Get distance from actual dot to position
            x_dist = pow( (pos_x-dotList[i].x ), 2 );
            y_dist = pow( (pos_y-dotList[i].y ), 2 );
            normal_distance = sqrt( abs(x_dist+y_dist) );
            printf("Normal distance: %f\n", normal_distance);

            //Get length of dot vector
            vector_lenght = sqrt( pow(dotList[i].x_speed, 2) + pow(dotList[i].y_speed, 2) );

            //Get angle of dot vector
            temp_angle = dotList[i].angle;  
            
            printf("Vector lenght: %f\n", vector_lenght);
            
            //Check how fast dot is moving. If it moves "slow" we check a circle with radius 100 around the dot.
            //If it moves "fast" we check a circle-arc with radius 300 and 90* angle in front of the dot.
            if(vector_lenght < 30){ check_distance = 100; compare_distance = normal_distance; }
            else{ check_distance = 300; compare_distance = predicted_distance; }
            
            printf("Angle: %f\n", temp_angle);
            
            if( (compare_distance < check_distance ) && compare_distance<distance){   //Find the dot closest to this "position"
                if(check_distance > 200){
                    if( temp_angle-angle_margin <point_angle < temp_angle+angle_margin){
                        distance=compare_distance; 
                        matchedIndex=i; 
                        new_dot=FALSE;
                        printf("Angle matching!\n");
                    }
                } else {
                
                    distance=compare_distance; 
                    matchedIndex=i; 
                    new_dot=FALSE;
                }
            }
        }
    }
    
    //Return pointer to the matched dot if we found one
    if(!new_dot){
    
        dotList[matchedIndex].matched=TRUE;
        return &dotList[matchedIndex];
    } 
    
    //Falling through here means make new dot!
    return ( addDot(&dotList[0], positionPointer));
    
}

/**
 * Main loop
 */
int run( SDL_Window* window, SDL_Renderer* renderer, int ddclientfd ) {

    char done = FALSE,
         show_calibrate = FALSE,
         dots_updated = FALSE,
         makeItRain = FALSE,
         draw_mode = FALSE,
         draw_vector = FALSE,
         epilepsy = FALSE;

    int x, y,
        i, j,
        seqnr;
        
    int numberOfDots=0;
    int numberOfBalls=0;
    int numberOfPositions=0;
    float* positionPointer;
    
    Dot* matchedDot;
    Position positionList[DD_MAX_DOTS];
    Dot dotList[DD_MAX_DOTS];
    Ball ballList[MAX_BALLS];     
    Parameters physicsParams;

    float laser_point_buf[DD_MAX_DOTS][2];

    SDL_Event event;

    FPSmanager fps;
    SDL_initFramerate( &fps );
    SDL_setFramerate( &fps, 30 );

    for(i=0; i<MAX_BALLS; i++) ballList[i].keep=FALSE;
    for(i=0; i<DD_MAX_DOTS; i++) dotList[i].keep=FALSE;
    physicsParams.momentum=TRUE;
    physicsParams.flip_gravity=FALSE;
    physicsParams.wind=FALSE;

    while( !done ) {

        // Clear the image
        if( !draw_mode ) {
            SDL_SetRenderDrawColor( renderer, 0x00, 0x00, 0x00, 0xFF );
            SDL_RenderClear( renderer );
        }

        // Get input from dotdetector
        numberOfPositions = getDots( ddclientfd, &laser_point_buf[0][0], &dots_updated, &seqnr );
        
        //Process dots
        if( dots_updated ) {
        
            //Set up pointer and transform coordinates recieved from dotdetector
            positionPointer = &laser_point_buf[0][0];
            transformDots( positionPointer, numberOfPositions );
            
            //Fill positionList
            j=0;
            for(i=0; i<numberOfPositions*2; i += 2){
            
                positionList[j].x = positionPointer[i];
                positionList[j].y = positionPointer[i+1];
                ++j;
            }
            
            //Match positions to dots, and spawn balls on all matched dots (including new dots)
            for(i=0; i<numberOfPositions; i++) {
                //Informative printout
                printf("Found position x: %d, y: %d\n", (int)positionList[i].x, (int)positionList[i].y);
                
                //Return reference to matched dot, or reference to new dot if no match was found. This is where the magic happens
                matchedDot =  matchPosition(&dotList[0], &positionList[i]);   
                
                printf("Matched with dot x: %d, y: %d\n", matchedDot->x, matchedDot->y);
                
                //If the position was matched to an existing dot we update the vector for that dot
                if(matchedDot->matched) updateVector(matchedDot, &positionList[i]); 
                
                //Draw the vector if enabled
                if(draw_vector) drawVector(matchedDot, renderer); 
                
                //Spawn a ball on the dot. The ball will be created with the same vector as the dot
                spawnBall(matchedDot, &ballList[0]);
                
                //We set the dot as matched even if it was created this frame. This is to keep it form beeing pruned later
                matchedDot->matched = TRUE;
                positionList[i].matched = TRUE;

            }
        }

        //Remove all dots without match. Dots created this frame will NOT be pruned.
        pruneDots(&dotList[0]);
        
        //Apply highly advanced physics model to balls
        applyForces(&ballList[0], physicsParams);
    
        //Draw dots
        for(i=0; i<DD_MAX_DOTS; i++){
            if(dotList[i].keep) drawDot( &dotList[i], renderer);
        }
    
        //Draw balls
        if(makeItRain){
            for(i=0; i<MAX_BALLS; i++){
                if(ballList[i].keep) drawBall( &ballList[i], renderer);
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
                            physicsParams.wind = !physicsParams.wind;
                            break;
                        case SDLK_f:
                            physicsParams.flip_gravity = !physicsParams.flip_gravity;
                            break;
                        case SDLK_m:
                            physicsParams.momentum = !physicsParams.momentum;
                            break;    
                        case SDLK_e:
                            epilepsy = !epilepsy;
                            break;    
                        case SDLK_v:
                            draw_vector = !draw_vector;
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
    if( screen_w < 100 || screen_h < 100 ) {
        fprintf( stderr, "Warning: You are using a silly low resolution!\n" );
    }
    verboseOut( "Using resolution %dx%d\n", screen_w, screen_h );

    initDDhelpers( screen_w, screen_h, fmin( screen_w, screen_h ) / 4, 10 );

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
