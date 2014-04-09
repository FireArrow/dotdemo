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
#include "dotdemo.h"

#define verboseOut(...) if(verbose) printf( __VA_ARGS__ )

#define MAX_DECAY 1
#define TRUE 1
#define FALSE 0

#define PI 3.14159265

#ifndef DD_MAX_DOTS
#define DD_MAX_DOTS 150
#endif

#ifndef MAX_BALLS
#define MAX_BALLS DD_MAX_DOTS * 10
#endif

static int screen_w = 0;
static int screen_h = 0;
static int dot_size=10;
static char verbose = 0;


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


void applyForces(Ball* ballList, Parameters physicsParams){

    int i,x;

    for( i=0; i<MAX_BALLS; ++i ) {
        if(ballList[i].keep){

            x = ballList[i].x;

            //Collision detection with edge of screen
            if(0>(x-(dot_size/2)) || (x+dot_size/2)>screen_w) ballList[i].x_speed = -ballList[i].x_speed; 

            //Apply linear force on all balls
            if(physicsParams.wind){ ballList[i].x += physicsParams.wind_speed;}    
                      
            //Apply individual momentum
            if(physicsParams.momentum){ 
                ballList[i].x += ballList[i].x_speed; 
                ballList[i].y += ballList[i].y_speed;
            }

            //Apply gravity
            if(physicsParams.flip_gravity) ballList[i].y_speed -= physicsParams.gravity_force;
            else ballList[i].y_speed += physicsParams.gravity_force;
            
            //Apply friction
            if(ballList[i].x_speed > 0) ballList[i].x_speed -= physicsParams.friction_force;
            if(ballList[i].x_speed < 0) ballList[i].x_speed += physicsParams.friction_force;      
                      
            //Check if ball has fallen from screen
            if(ballList[i].y > screen_h || ballList[i].y < 0){ ballList[i].keep=FALSE;}
        }
    }
}


void updateVector(Dot* dot, Position* positionPointer){

    int x_speed, y_speed,
        dot_x=dot->x, dot_y=dot->y,
        pos_x=positionPointer->x, pos_y=positionPointer->y;
    double f1,f2,temp_angle, vector_length;    
    
    //Calculate new speed
    x_speed = (pos_x - dot_x);
    y_speed = (pos_y - dot_y);    

    //Some floats for atan
    f1 = (x_speed);
    f2 = (y_speed);
    
    //Calculate vector angle
    temp_angle = atan2( f1,f2 )*180 / PI;
    if(temp_angle < 0) temp_angle += 360;

    //Calculate vector length
    vector_length = sqrt( pow(x_speed, 2) + pow(y_speed, 2) );

    //Update dot parameters
    dot->x = pos_x;
    dot->y = pos_y;
    dot->x_speed = x_speed;
    dot->y_speed = y_speed;
    dot->vector.angle = temp_angle;
    dot->vector.length = vector_length;
    
    verboseOut("X_speed: %d\n", dot->x_speed);
    verboseOut("Y_speed: %d\n", dot->y_speed);
    verboseOut("************************\n");
}

Dot* matchPosition(Dot* dotList, struct Position* positionPointer){

    char new_dot=TRUE, turning;

    int i,
        pos_x,pos_y, 
        dot_x,dot_y, predicted_dot_x, predicted_dot_y,
        matchedIndex, matching_radius;
        
    double min_distance=9999,
        distance_to_point,
        x_dist, y_dist;
    double vector_angle,point_angle,vector_length,
           angle_matching_margin=90, angle_turning_margin=40, fast_threshold=30;

    pos_x = positionPointer->x;  
    pos_y = positionPointer->y;
    
    //verboseOut("Matching, x=%d, y=%d\n", x, y);
    
    //Check if point matches any dot
    for(i=0; i<DD_MAX_DOTS; i++){
        //For each dot
        if(dotList[i].keep && !dotList[i].matched){ //Check .matched so we only match maximum one point per dot
            //Set turning to true
            turning = TRUE;
            
            //Get position of current dot
            dot_x = dotList[i].x;
            dot_y = dotList[i].y;
            
            verboseOut("Checking dot X: %d, Y: %d\n", dot_x, dot_y);
            
            //Get length of dot vector
            vector_length = dotList[i].vector.length;

            //Get angle of dot vector
            vector_angle = dotList[i].vector.angle;  
            
            //Get angle between the position and current dot
            point_angle = atan2(pos_x-dot_x , pos_y-dot_y)*180 / PI;
            if(point_angle < 0) point_angle += 360;
            
            verboseOut("Point angle: %f\n", point_angle);
            
            verboseOut("Vector angle: %f\n", vector_angle);
            
            //Check if point is "turning"
            if( vector_angle-(angle_turning_margin/2) < point_angle < vector_angle+(angle_turning_margin/2) ) turning = FALSE;
            
            //If dot is "fast" and not "turning" we predict its next position and measure the distance from there
            if( (vector_length > fast_threshold) && !turning ){
            
                //Get distance from predicted dot to position
                x_dist = pow( (pos_x-dotList[i].x - (dotList[i].x_speed) ), 2);
                y_dist = pow( (pos_y-dotList[i].y - (dotList[i].y_speed) ), 2);
                distance_to_point = sqrt( abs(x_dist+y_dist) );
                verboseOut("Predicted Distance: %f\n", distance_to_point);
            } else {
                
                //Get distance from actual dot to position
                x_dist = pow( (pos_x-dotList[i].x ), 2 );
                y_dist = pow( (pos_y-dotList[i].y ), 2 );
                distance_to_point = sqrt( abs(x_dist+y_dist) );
                verboseOut("Normal distance: %f\n", distance_to_point);
            }
            
            verboseOut("Vector length: %f\n", vector_length);
            
            //Check how fast dot is moving. If it moves "slow" we check a circle with radius 100 around the dot.
            //If it moves "fast" we check a circle-arc with radius 300 and 90* angle in front of the dot.
            if(vector_length < fast_threshold) matching_radius = 100; 
            else matching_radius = 300;  
            
            
            //Check if the distance from point to dot (or predicted dot) is within the allowed radius
            if( (distance_to_point < matching_radius ) && (distance_to_point < min_distance) ){   //Find the dot closest to this "position"
            
                //If the vector length is over the "fast" threshold we have a "fast" dot. So we check the angle as well
                if(vector_length > fast_threshold){   
                    if( vector_angle-(angle_matching_margin/2)< point_angle <vector_angle+(angle_matching_margin/2)){
                    
                        //Save the index of this dot as the matched index and update the minimum distance found
                        min_distance = distance_to_point; 
                        matchedIndex = i; 
                        new_dot = FALSE;
                        verboseOut("Angle matching!\n");
                    }
                } else {
                    //Save the index of this dot as the matched index and update the minimum distance found
                    min_distance = distance_to_point; 
                    matchedIndex = i; 
                    new_dot = FALSE;
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
    physicsParams.wind=TRUE;
    physicsParams.wind_speed=5;
    physicsParams.friction_force=1;
    physicsParams.gravity_force=2;
    
    while( !done ) {    //Main loop

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
                verboseOut("Found dot x: %d, y: %d\n", (int)positionList[i].x, (int)positionList[i].y);
                
                //Return reference to matched dot, or reference to new dot if no match was found. This is where the magic happens
                matchedDot =  matchPosition(&dotList[0], &positionList[i]);   
                
                verboseOut("Matched with dot x: %d, y: %d\n", matchedDot->x, matchedDot->y);
                
                //If the position was matched to an existing dot we update the vector for that dot
                if(matchedDot->matched) updateVector(matchedDot, &positionList[i]); 
                
                //Draw the vector if enabled
                if(draw_vector) drawVector(matchedDot, renderer); 
                
                //Spawn a ball on the dot. The ball will be created with the same vector as the dot
                spawnBall(matchedDot, &ballList[0]);
                
                //We set the dot as matched even if it was created this frame. This is to keep it form beeing decayed later
                matchedDot->matched = TRUE;
                positionList[i].matched = TRUE;

            }
        }

        //Decay all dots without match. Dots created this frame will NOT be decayed.
        pruneDots(&dotList[0]);
        
        //Apply highly advanced physics model to balls
        applyForces(&ballList[0], physicsParams);
    
        //Draw dots

        for(i=0; i<DD_MAX_DOTS; i++){
            if(dotList[i].keep) drawDot( &dotList[i], renderer, dot_size);
        }
    
        //Draw balls
        if(makeItRain){
            for(i=0; i<MAX_BALLS; i++){
                if(ballList[i].keep) drawBall( &ballList[i], renderer, dot_size);
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

    } //End of main loop

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
    verboseOut( "%s\n", err_msg );

    for( i=0; line[i] != NULL; ++i ) {
        verboseOut( "%s\n", line[i] );
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
