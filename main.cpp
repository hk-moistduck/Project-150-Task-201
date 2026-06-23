#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <time.h>
#include "./constants.h" //define all constants in a single place


int game_is_running = FALSE; // TRUE = game is on, FALSE = game closes

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* game_font = NULL;

int game_is_playing = TRUE; // TRUE = playing, FALSE = game over
int last_frame_time = 0;
int score = 0;

float move_timer = 0;
float move_delay = 0.15f; // move snake every 0.15 seconds

int can_turn_snake_head = TRUE;; // flag to prevent self collision bug when pressing two keys very fast

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
Position food;

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

    if(TTF_Init() == -1){
        fprintf(stderr, "Error creating SDL ttf: %s\n", TTF_GetError());
        return FALSE;
    }

    return TRUE;
}

void spawn_food(){
    // to check if food generated on snake body
    int is_on_body;

    do{
        // assume randomly generated spot is not on snake body
        is_on_body = FALSE;

        // roll the dice for food coordinates
        food.x = rand() % GRID_WIDTH;
        food.y = rand() % GRID_HEIGHT;

        // loop through entire snake body to look for overlap
        for(int i = 0; i < snake.length; i++){
            if(food.x == snake.body_position[i].x && food.y == snake.body_position[i].y){
                is_on_body = TRUE;
                break;
            }
        }
    } while(is_on_body); // try again if food is on snake body
}

void render_text(const char* text, int x, int y, SDL_Color color){
    if(game_font == NULL) return;

    SDL_Surface* surface = TTF_RenderText_Solid(game_font, text, color);
    if(surface == NULL) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect destination_rect = {x, y, surface -> w * 2, surface -> h * 2};

    SDL_RenderCopy(renderer, texture, NULL, &destination_rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void setup(){
    // seed random number generator for food
    srand(time(NULL));

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
    snake.direction_y = 0;

    spawn_food();

    game_font = TTF_OpenFont("assets/Minecraft.ttf", 16);
    if(game_font == NULL){
        fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
    }
}

void process_input(){
    SDL_Event event;

    while(SDL_PollEvent(&event)){
        switch(event.type){

            // exit game loop by clicking "X" on the window border
            case SDL_QUIT:
                game_is_running = FALSE;
                break;
            
            case SDL_KEYDOWN:
                // exit game loop by pressing Esc key
                if(event.key.keysym.sym == SDLK_ESCAPE){
                    game_is_running = FALSE;
                    break;
                }
            
                // input arrow keys for snake movement
                if(can_turn_snake_head == TRUE)
                    // set the other axis direction to 0 to prevent 180 degree turn
                    if(event.key.keysym.sym == SDLK_UP && snake.direction_y == 0){
                        snake.direction_x = 0;
                        snake.direction_y = -1;
                        can_turn_snake_head = FALSE;
                    }
                    else if(event.key.keysym.sym == SDLK_DOWN && snake.direction_y == 0){
                        snake.direction_x = 0;
                        snake.direction_y = 1;
                        can_turn_snake_head = FALSE;
                    }
                    else if(event.key.keysym.sym == SDLK_RIGHT && snake.direction_x == 0){
                        snake.direction_y = 0;
                        snake.direction_x = 1;
                        can_turn_snake_head = FALSE;
                    }
                    else if(event.key.keysym.sym == SDLK_LEFT && snake.direction_x == 0){
                        snake.direction_y = 0;
                        snake.direction_x = -1;
                        can_turn_snake_head = FALSE;
                    }
                    break;
        }
    }
}

void update(){
    float delta_time = (SDL_GetTicks64() - last_frame_time) / 1000.0f;
    last_frame_time = SDL_GetTicks64();

    // stop logic if game over
    if(game_is_playing == FALSE) return;
    
    // move snake at intervals for grid step effect
    move_timer += delta_time;
    if(move_timer >= move_delay){
        move_timer = 0;

        // shift every snake body tile position to the one in front of it
        for(int i = snake.length - 1; i > 0; i--){
            snake.body_position[i] = snake.body_position[i - 1];
        }
        can_turn_snake_head = TRUE;

        // apply snake head direction
        snake.body_position[0].x += snake.direction_x;
        snake.body_position[0].y += snake.direction_y;

        // boundary collisions
        if(snake.body_position[0].x < 0 || snake.body_position[0].x >= GRID_WIDTH ||
            snake.body_position[0].y < 0 || snake.body_position[0].y >= GRID_HEIGHT){
                game_is_playing = FALSE;
            }

        // self collisions
        for(int i = 1; i < snake.length; i++){
            if(snake.body_position[0].x == snake.body_position[i].x &&
                snake.body_position[0].y == snake.body_position[i].y){
                    game_is_playing = FALSE;
                }
        }

        // grow snake body by 1 when snake head eats food
        if(snake.body_position[0].x == food.x && snake.body_position[0].y == food.y){
            Position current_tail_position = snake.body_position[snake.length - 1];

            snake.length++;
            score += 10;

            // assign new snake tail at old tail position
            snake.body_position[snake.length - 1] = current_tail_position;

            spawn_food();
        }
    }
}

void render(){
    // set background color to black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if(game_is_playing == TRUE){
        // draw snake
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // green snake
        for(int i = 0; i < snake.length; i++){
            SDL_Rect snake_rect = {
                snake.body_position[i].x * TILE_SIZE,
                snake.body_position[i].y * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE
            };
            SDL_RenderFillRect(renderer, &snake_rect);
        }

        // draw food
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // red food
        SDL_Rect food_rect = {
            food.x * TILE_SIZE,
            food.y * TILE_SIZE, 
            TILE_SIZE, 
            TILE_SIZE
        };
        SDL_RenderFillRect(renderer, &food_rect);
    }

    SDL_Color white_color = { 255, 255, 255, 255};
    char score_string[32];
    sprintf(score_string, "Score: %d", score);

    render_text(score_string, 20, 20, white_color);

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