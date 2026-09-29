#include "drally.h"

extern void_cb ___2432c8h;

void dRally_Keyboard_make(SDL_Scancode);
void dRally_Keyboard_break(SDL_Scancode);
void dRally_Display_toggleFullscreen(void);

void IO_Loop(void){

    SDL_Event e;

    while(SDL_PollEvent(&e)){

        if(e.type == SDL_KEYDOWN){

            /* Alt+Enter: toggle fullscreen (don't feed to game input) */
            if((e.key.keysym.mod & KMOD_ALT) &&
               (e.key.keysym.scancode == SDL_SCANCODE_RETURN ||
                e.key.keysym.scancode == SDL_SCANCODE_KP_ENTER)){
                dRally_Display_toggleFullscreen();
                continue;
            }
            dRally_Keyboard_make(e.key.keysym.scancode);
        }
        else if(e.type == SDL_KEYUP){
           
            dRally_Keyboard_break(e.key.keysym.scancode);
        }
        else if(e.type == SDL_QUIT){
            printf("[dRally] TODO: exit not handled properly\n");
            ___2432c8h();
        }
    }
}
