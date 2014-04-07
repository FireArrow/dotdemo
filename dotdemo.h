#ifndef __DOTDEMO_H__
#define __DOTDEMO_H__


typedef struct Dot Dot;
typedef struct Position Position;

struct Dot {
    int x;
    int y;
    int x_speed;
    int y_speed;
    int r,b,g;
    int decay;
    double angle;
    char keep;
    char matched;
    Position* matchedPosition;
};

struct Position {
    int x;
    int y;
    int matchedDistance;
    char matched;
    Dot* matchedDot;
};


#endif
