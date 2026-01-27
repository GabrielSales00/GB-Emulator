#ifndef SCREEN_H
#define SCREEN_H


#include <SDL2/SDL.h>


typedef struct Screen {
    SDL_WINDOW * window;
    SDL_RENDERER * renderer;
    SDL_TEXTURE * texture;
} Screen;

Screen * Screen(const char * instance_name, int window_height, int window_width,
                int texture_height, int texture_width);


void close_screen(Screen * screen);

#endif