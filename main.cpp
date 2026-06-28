#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>
#include <time.h>
#include "./constants.h" // define all constants in a single place


int game_is_running = FALSE; // TRUE = game is on, FALSE = game closes

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
TTF_Font* game_font = NULL;

int game_is_playing = TRUE; // TRUE = playing, FALSE = game over

int score = 0;
int score_width = 0;
int score_height = 0;
SDL_Texture* score_texture = NULL;

Uint64 last_frame_time = 0;
float move_timer = 0;
float move_delay = 0.15f; // snake initial speed | move every 0.12s

int can_turn_snake_head = TRUE;; // flag to prevent self collision bug when pressing two keys very fast

SDL_Color white_color = {255, 255, 255, 255};
SDL_Color red_color = {255, 0, 0, 255};

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

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
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

// texture function uses extra parameters to change the scale of the textures
SDL_Texture* create_text_texture(
    const char* text, SDL_Color color, int scale,
    int* output_width, int* output_height){
        if(game_font == NULL) return NULL;

        // draw surface (CPU memory)
        SDL_Surface* surface = TTF_RenderText_Solid(game_font, text, color);
        if(surface == NULL) return NULL;

        // draw texture (GPU memory)
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

        // final text dimensions
        *output_width = surface -> w * scale;
        *output_height = surface -> h * scale;

        SDL_FreeSurface(surface);
        return texture;
}

// for optimization, score texture only when score changes
void refresh_score_texture(){
    if(score_texture != NULL){
        SDL_DestroyTexture(score_texture);
    }

    char score_string[32];
    sprintf(score_string, "Score: %d", score);

    score_texture = create_text_texture(score_string,
                                        white_color, 
                                        2,
                                        &score_width, 
                                        &score_height);
}

void move_snake(void){
    // shift every snake body tile position to the one in front of it
    for(int i = snake.length - 1; i > 0; i--){
        snake.body_position[i] = snake.body_position[i - 1];
    }
    can_turn_snake_head = TRUE;

    // apply snake head direction
    snake.body_position[0].x += snake.direction_x;
    snake.body_position[0].y += snake.direction_y;
}

void check_collision(void){
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
            if(food.x == snake.body_position[i].x && 
                food.y == snake.body_position[i].y){
                    is_on_body = TRUE;
                    break;
            }
        }
    } while(is_on_body); // try again if food is on snake body
}

void check_food_consumption(void){
    // grow snake body by 1 when snake head eats food
    if(snake.body_position[0].x == food.x && snake.body_position[0].y == food.y){
        Position current_tail_position = snake.body_position[snake.length - 1];

        snake.length++;
        score += SCORE_INCREMENT;
        // change score texture only when score changes
        refresh_score_texture();

        // assign new snake tail at old tail position
        snake.body_position[snake.length - 1] = current_tail_position;

        spawn_food();
    }
}

void draw_snake(void){
    for(int i = 0; i < snake.length; i++){
        if(i == 0){
            // draw head as dark green
            SDL_SetRenderDrawColor(renderer, 0, 180, 0, 255);
        }
        else{
            // draw rest of the body as green
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        }
        SDL_Rect snake_rect = {
            snake.body_position[i].x * TILE_SIZE,
            snake.body_position[i].y * TILE_SIZE,
            TILE_SIZE,
            TILE_SIZE
        };
        SDL_RenderFillRect(renderer, &snake_rect);
    }
}

void draw_food(void){
    // red food
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect food_rect = {
        food.x * TILE_SIZE,
        food.y * TILE_SIZE, 
        TILE_SIZE, 
        TILE_SIZE
    };
    SDL_RenderFillRect(renderer, &food_rect);
}

void draw_game_score(void){
    if(score_texture != NULL){
        SDL_Rect rect = {20, 20, score_width, score_height};
        SDL_RenderCopy(renderer, score_texture, NULL, &rect);
    }
}

void draw_game_start_prompt(void){
    int is_stationary = (snake.direction_x == 0 && snake.direction_y == 0);
    if(is_stationary == TRUE){
        int text_width = 0;
        int text_height = 0;
        SDL_Texture* start_prompt = create_text_texture(
                                                        "Press SPACE to Start",
                                                        white_color,
                                                        2,
                                                        &text_width,
                                                        &text_height);
        if(start_prompt != NULL){
            int text_x_position = (WINDOW_WIDTH - text_width) / 2;
            int text_y_positon = (WINDOW_HEIGHT / 2) + (2 * text_height);
            SDL_Rect rect = {text_x_position, text_y_positon, text_width, text_height};
            SDL_RenderCopy(renderer, start_prompt, NULL, &rect);
            SDL_DestroyTexture(start_prompt);
        }
    }
}

void draw_game_over(void){
    int text_width = 0;
    int text_height = 0;

    SDL_Texture* game_over_texture = create_text_texture(
                                                    "GAME OVER",
                                                    red_color,
                                                    3,
                                                    &text_width,
                                                    &text_height);
    if(game_over_texture != NULL){
        int text_x_position = (WINDOW_WIDTH - text_width) / 2;
        int text_y_position = (WINDOW_HEIGHT / 2) - (2 * text_height);

        SDL_Rect rect = {text_x_position, text_y_position, text_width, text_height};
        SDL_RenderCopy(renderer, game_over_texture, NULL, &rect);
        SDL_DestroyTexture(game_over_texture);
    }
}

void draw_final_score(void){
    int text_width = 0;
    int text_height = 0;
    char final_score_string[64];
    sprintf(final_score_string, "Final score: %d", score);

    SDL_Texture* final_score_texture = create_text_texture(
                                                    final_score_string,
                                                    white_color,
                                                    2,
                                                    &text_width,
                                                    &text_height);
    if(final_score_texture != NULL){
        int text_x_position = (WINDOW_WIDTH - text_width) / 2;
        int text_y_position = (WINDOW_HEIGHT / 2) - text_height;

        SDL_Rect rect = {text_x_position, text_y_position, text_width, text_height};
        SDL_RenderCopy(renderer, final_score_texture, NULL, &rect);
        SDL_DestroyTexture(final_score_texture);
    }
}

void draw_game_restart_prompt(void){
    int text_width = 0;
    int text_height = 0;

    SDL_Texture* restart_texture = create_text_texture(
                                                    "Press SPACE to Restart",
                                                    white_color,
                                                    2,
                                                    &text_width,
                                                    &text_height);
    if(restart_texture != NULL){
        int text_x_position = (WINDOW_WIDTH - text_width) / 2;
        int text_y_position = WINDOW_HEIGHT - (3 * text_height);

        SDL_Rect rect = {text_x_position, text_y_position, text_width, text_height};
        SDL_RenderCopy(renderer, restart_texture, NULL, &rect);
        SDL_DestroyTexture(restart_texture);
    }
}

void setup(){
    // seed random number generator for food
    srand(time(NULL));

    game_is_playing = TRUE;
    score = 0;
    move_timer = 0;

    // initialize snake in the middle of the grid
    snake.length = INITIAL_SNAKE_LENGTH;
    snake.body_position[0].x = GRID_WIDTH / 2;
    snake.body_position[0].y = GRID_HEIGHT / 2;
    snake.body_position[1].x = (GRID_WIDTH / 2) - 1;
    snake.body_position[1].y = GRID_HEIGHT / 2;
    snake.body_position[2].x = (GRID_WIDTH / 2) - 2;
    snake.body_position[2].y = GRID_HEIGHT / 2;

    // snake stays still until game starts
    snake.direction_x = 0;
    snake.direction_y = 0;

    spawn_food();

    if(game_font == NULL){
        game_font = TTF_OpenFont("assets/Minecraft.ttf", 16);
        if(game_font == NULL){
            fprintf(stderr, "Error loading font: %s\n", TTF_GetError());
        }
    }

    refresh_score_texture();
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

                int is_stationary = (snake.direction_x == 0 && snake.direction_y == 0);

                // input during game over screen
                if(game_is_playing == FALSE && event.key.keysym.sym == SDLK_SPACE){
                    setup();
                    break;
                }

                // input during game start screen
                if(game_is_playing == TRUE && is_stationary == TRUE){
                    if(event.key.keysym.sym == SDLK_SPACE){
                        snake.direction_x = 1;
                        snake.direction_y = 0;
                        can_turn_snake_head = FALSE;
                        break;
                    }
                }
            
                // input during snake movement
                if(can_turn_snake_head == TRUE && is_stationary == FALSE)
                    // set the other axis direction to 0 to prevent 180 degree turn
                    if((event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_w)
                        && snake.direction_y == 0){
                            snake.direction_x = 0;
                            snake.direction_y = -1;
                            can_turn_snake_head = FALSE;
                    }
                    else if((event.key.keysym.sym == SDLK_DOWN || event.key.keysym.sym == SDLK_s)
                        && snake.direction_y == 0){
                            snake.direction_x = 0;
                            snake.direction_y = 1;
                            can_turn_snake_head = FALSE;
                    }
                    else if((event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_d)
                        && snake.direction_x == 0){
                            snake.direction_y = 0;
                            snake.direction_x = 1;
                            can_turn_snake_head = FALSE;
                    }
                    else if((event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_a)
                        && snake.direction_x == 0){
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
    

    // snake is still at the start game screen
    int is_stationary = (snake.direction_x == 0 && snake.direction_y == 0);
    if(is_stationary == TRUE) return;

    // move snake at intervals for grid step effect
    move_timer += delta_time;
    if(move_timer >= move_delay){
        move_timer = 0;

        move_snake();

        check_collision();

        check_food_consumption();
    }
}

void render(){
    // set background color to black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if(game_is_playing == TRUE){
        
        draw_snake();

        draw_food();

        draw_game_score();

        draw_game_start_prompt();
    }
    // when snake collides
    else if(game_is_playing == FALSE){
        
        draw_game_over();
        
        draw_final_score();

        draw_game_restart_prompt();
    }

    SDL_RenderPresent(renderer);
}

void destroy_window(){
    if(score_texture != NULL) SDL_DestroyTexture(score_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(game_font);
    TTF_Quit();
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