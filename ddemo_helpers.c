#include "dotdemo.h"
#include <SDL2/SDL.h>

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
            
            dotList[i].r = rand() % 0xFF;
            dotList[i].b = rand() % 0xFF;
            dotList[i].g = rand() % 0xFF;
            return (Dot*) &dotList[i];
        }
    }
}
