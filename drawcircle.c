#include "drawcircle.h"

/*
 * This is an implementation of the Midpoint Circle Algorithm 
 * found on Wikipedia at the following link:
 *
 *   http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
 *
 * The algorithm elegantly draws a circle quickly, using a
 * set_pixel function for clarity.
 */
void draw_circle(SDL_Renderer* renderer, int n_cx, int n_cy, int radius);
{
    // if the first pixel in the screen is represented by (0,0) (which is in sdl)
    // remember that the beginning of the circle is not in the middle of the pixel
    // but to the left-top from it:
 
    double error = (double)-radius;
    double x = (double)radius -0.5;
    double y = (double)0.5;
    double cx = n_cx - 0.5;
    double cy = n_cy - 0.5;
 
    while (x >= y)
    {
        SDL_RenderDrawPoint( renderer, (int)(cx + x), (int)(cy + y) );
        SDL_RenderDrawPoint( renderer, (int)(cx + y), (int)(cy + x) );
 
        if (x != 0)
        {
            SDL_RenderDrawPoint( renderer, (int)(cx - x), (int)(cy + y) );
            SDL_RenderDrawPoint( renderer, (int)(cx + y), (int)(cy - x) );
        }
 
        if (y != 0)
        {
            SDL_RenderDrawPoint( renderer, (int)(cx + x), (int)(cy - y) );
            SDL_RenderDrawPoint( renderer, (int)(cx - y), (int)(cy + x) );
        }
 
        if (x != 0 && y != 0)
        {
            SDL_RenderDrawPoint( renderer, (int)(cx - x), (int)(cy - y) );
            SDL_RenderDrawPoint( renderer, (int)(cx - y), (int)(cy - x) );
        }
 
        error += y;
        ++y;
        error += y;
 
        if (error >= 0)
        {
            --x;
            error -= x;
            error -= x;
        }
    }
}
