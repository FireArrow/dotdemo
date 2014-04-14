#include "dotdemo.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#define TRUE 1
#define FALSE 0
#define MAX_DECAY 1

#ifndef DD_MAX_DOTS
#define DD_MAX_DOTS 150
#endif

#ifndef MAX_BALLS
#define MAX_BALLS DD_MAX_DOTS * 10
#endif

void drawBall(Ball* ball, SDL_Renderer* renderer, int dot_size){

                 filledCircleRGBA( renderer, ball->x, ball->y, dot_size,
                                ball->r % 0xFF,          // R
                                ball->g % 0xFF,          // G
                                ball->b % 0xFF,          // B
                                0xFF                     // A
                        );
}


void drawDot(Dot* dot, SDL_Renderer* renderer, int dot_size){

                 filledCircleRGBA( renderer, dot->x, dot->y, dot_size,
                                dot->r % 0xFF,          // R
                                dot->g % 0xFF,          // G
                                dot->b % 0xFF,          // B
                                0xFF                    // A
                        );
}

void drawMatchingArea(Dot* dot, SDL_Renderer* renderer){

    if(dot->vector.length <= 15) aacircleRGBA(renderer, dot->x, dot->y, 100, 255, 0, 0, 255); 
    
    if(dot->vector.length > 15) pieRGBA(renderer, dot->x, dot->y, 30+(dot->vector.length)*1.5, (-dot->vector.angle+90)-90, (-dot->vector.angle+90)+90, 255, 0, 0, 255);

    pieRGBA(renderer, dot->x, dot->y,300, (-dot->vector.angle+90)-45, (-dot->vector.angle+90)+45, 255, 255, 0, 100);

}

void drawVector(Dot* dot, SDL_Renderer* r){

SDL_SetRenderDrawColor(r,255,255,255,255);

    SDL_RenderDrawLine(r, dot->x, dot->y, (dot->x+dot->x_speed), (dot->y+dot->y_speed) );

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
            dotList[i].vector.length=0;
            dotList[i].vector.angle=0;
            dotList[i].matched_distance=9999;
            dotList[i].matched_w_distance=9999;
            
            dotList[i].r = rand() % 0xFF;
            dotList[i].b = rand() % 0xFF;
            dotList[i].g = rand() % 0xFF;
            return (Dot*) &dotList[i];
        }
    }
}

void getKeyboardInput(SDL_Event event, PhysicsParameters* physicsParams, InputParameters* inputParams){

    while( SDL_PollEvent( &event ) ) {
    switch( event.type ) {
        case SDL_QUIT:
            inputParams->done = 1;
            break;

        case SDL_KEYDOWN:
            switch( event.key.keysym.sym ) {
                case SDLK_ESCAPE:
                    inputParams->done = 1;
                    break;

                case SDLK_c:
                    inputParams->show_calibrate = ~inputParams->show_calibrate;
                    break;

                case SDLK_d:
                    inputParams->draw_mode = ~inputParams->draw_mode;
                    break;

                case SDLK_r:
                    inputParams->make_it_rain = !inputParams->make_it_rain;
                    break;

                case SDLK_w:
                    physicsParams->wind = !physicsParams->wind;
                    break;

                case SDLK_f:
                    physicsParams->flip_gravity = !physicsParams->flip_gravity;
                    break;

                case SDLK_m:
                    physicsParams->momentum = !physicsParams->momentum;
                    break;

                case SDLK_v:
                    inputParams->draw_vector = !inputParams->draw_vector;
                    break;

                case SDLK_a:
                    inputParams->draw_matching_area = !inputParams->draw_matching_area;
                    break;

                case SDLK_l:
                    inputParams->leftovers = !inputParams->leftovers;
                    break;

                case SDLK_1:
                    inputParams->dot_size = 10;
                    break;

                case SDLK_2:
                    inputParams->dot_size = 20;
                    break;

                case SDLK_3:
                    inputParams->dot_size = 30;
                    break;

            } // End of key down
        }
    }
}
