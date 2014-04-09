#ifndef __DOTDEMO_H__
#define __DOTDEMO_H__
#endif

#include <SDL2/SDL.h>

typedef struct Vector {

    double length;
    double angle;
} Vector;

typedef struct Parameters{
    char wind;
    char momentum;
    char flip_gravity;
    int wind_speed;
    int gravity_force;
    int friction_force;
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
    Vector vector;
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

void drawBall(Ball* ball, SDL_Renderer* renderer, int dot_size);

void drawDot(Dot* dot, SDL_Renderer* renderer, int dot_size);

void drawVector(Dot* dot, SDL_Renderer* r);

int spawnBall(Dot* dot, Ball* ballList);

Dot* addDot(Dot* dotList, Position* positionPointer);

