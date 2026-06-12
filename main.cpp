#include <stdio.h>
#include <SDL2/SDL.h>
#include "./constants.h" //define all constants in a single place


int game_is_running = FALSE;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

typedef struct{
    int x;
    int y;
} Position;

typedef struct{
    Position body_position[MAX_SNAKE_LENGTH];
    int length;
    int direction_x;
    int direction_y;
} Snake;

Snake snake;

int game_is_playing = TRUE; // TRUE = playing, FALSE = game over

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
    // initialize snake in the middle of the grid
    snake.length = 3;
    snake.body_position[0].x = GRID_WIDTH / 2;
    snake.body_position[0].y = GRID_HEIGHT / 2;
    snake.body_position[1].x = (GRID_WIDTH / 2) - 1;
    snake.body_position[1].y = GRID_HEIGHT / 2;
    snake.body_position[2].x = (GRID_WIDTH / 2) - 2;
    snake.body_position[2].y = GRID_HEIGHT / 2;

    // start moving right
    snake.direction_x = 1;
    snake.direction_x = 0;
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

    if(game_is_playing = TRUE){
        // draw snake
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // green snake
        for(int i = 0; i < snake.length; i++){
            SDL_Rect snake_rect {
                snake.body_position[i].x * TILE_SIZE,
                snake.body_position[i].y * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE
            };
            SDL_RenderFillRect(renderer, &snake_rect);
        }
    }

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