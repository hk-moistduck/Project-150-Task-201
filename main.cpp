#include <stdio.h>
#include <SDL2/SDL.h>
#include "./constants.h" //define all constants in a single place


int game_is_running = FALSE;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int initialize_window(void){
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
        fprintf(stderr, "error initializing SDL\n");
        return FALSE;
    }

    window = SDL_CreateWindow(
            "SNAKE_GAME",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            0
        );

    renderer = SDL_CreateRenderer(window, -1, 0);
    if(!renderer){
        fprintf(stderr, "Error creating SDL Renderer\n");
        return FALSE;
    }

    return TRUE;
}

void setup(){

}

void process_input(){
    SDL_Event event;

    while(SDL_PollEvent(&event)){
        switch(event.type){

            // exit game loop by clicking "X" on the window border
            case SDL_QUIT:
                game_is_running = FALSE;
                break;
            
            // exit game loop by pressing Esc key
            case SDL_KEYDOWN:
                if(event.key.keysym.sym == SDLK_ESCAPE){
                    game_is_running = FALSE;
                    break;
                }
        }
    }
}

void update(){

}

void render(){
    // set background color to black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);
}

void destroy_window(){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char* args[]){
    game_is_running = initialize_window();

    setup();

    // game loop
    while(game_is_running){
        process_input();
        update();
        render();
    }

    destroy_window();

    return 0;
}