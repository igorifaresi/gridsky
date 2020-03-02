#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

//-----------------------------------------------------------------------------

typedef struct Camera {
    int x;
    int y;
    int v_block_qnt;
}Camera;

enum ComponentTypes {
    BLANK_COMP,
    BLOCK_COMP,
    PLAYER_COMP,
    FOE_COMP,
};

typedef enum ComponentTypes ComponentType;

typedef void (*UpdateFunction)(int, int, char);

typedef struct FoeData {
    unsigned char life;
}FoeData;

typedef struct Component {
    ComponentType type;
    size_t last_update_tick;
    SDL_Texture* sprite;
    union {
        FoeData foe_data;
    };
    UpdateFunction update;
}Component;

enum InputTypes {
    NOT_INPUT,
    UP_INPUT,
    DOWN_INPUT,
    LEFT_INPUT,
    RIGHT_INPUT,
    SKIP_INPUT,
};

typedef enum InputTypes InputType;

Camera main_camera;
SDL_Texture* background[1024][1024];
Component grid[1024][1024];
int h_screen_resolution;
int v_screen_resolution;

unsigned player_grid_pos_x;
unsigned player_grid_pos_y;
struct {
    unsigned char life;
    unsigned char power;
    unsigned char element;
}player_data;


SDL_Texture* sprite_assets[3];


//-----------------------------------------------------------------------------

void closeGame(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture** sprites, 
               int sprite_qnt)
{
    for (int i = 0; i < sprite_qnt; i++) {
        SDL_DestroyTexture(sprites[i]);
    }
    SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}

void fillBackground(SDL_Texture* sprite)
{
    for (int y = 0; y < 1024; y++) {
        for (int x = 0; x < 1024; x++) {
            background[y][x] = sprite;
        }
    }
}

void eraseGrid() 
{
    for (int y = 0; y < 1024; y++) {
        for (int x = 0; x < 1024; x++) {
            grid[y][x].type = BLANK_COMP;
        }
    }
}

void renderCell(SDL_Renderer* renderer, SDL_Texture* sprite, int pos_y, 
                int pos_x, int size)
{
    SDL_Rect rect;
    rect.x = pos_x;
    rect.y = pos_y;
    rect.h = size;
    rect.w = size;
    SDL_RenderCopy(renderer, sprite, NULL, &rect);
}

void updateGame(SDL_Renderer* renderer, Camera* camera, char season, size_t actual_tick, bool update_entities) 
{
    assert(h_screen_resolution > v_screen_resolution);
    int block_size = v_screen_resolution / camera->v_block_qnt;
    int tmp  = (h_screen_resolution / 2) - (v_screen_resolution / 2);
    int h_block_qnt = 2 * (int)ceil((double)tmp / (double)block_size);
    int init_render_block_pos =    (h_screen_resolution / 2) 
                                 - ((camera->v_block_qnt * block_size) / 2)
                                 - ((h_block_qnt * block_size) / 2);
    int block_offset = h_block_qnt / 2; //wrong calculus

    assert(SDL_RenderClear(renderer) != -1);
    for (int y = 0; y < camera->v_block_qnt; y++) {
        int render_pos_x = init_render_block_pos;
        int render_pos_y = block_size * y;
        for (int x = 0; x < h_block_qnt + camera->v_block_qnt; x++) {
            int grid_pos_x = camera->x + x - block_offset;
            int grid_pos_y = camera->y - y;
            if ((grid_pos_x >= 0 && grid_pos_x < 1024)
             && (grid_pos_y >= 0 && grid_pos_y < 1024)) {
                SDL_Texture* back = background[grid_pos_y][grid_pos_x];
                if (back != NULL) {
                    renderCell(renderer, background[grid_pos_y][grid_pos_x]
                                       , render_pos_y
                                       , render_pos_x
                                       , block_size);
                }
                Component* it = &grid[grid_pos_y][grid_pos_x];
                if (it->type != BLANK_COMP) {
                    renderCell(renderer, it->sprite, render_pos_y
                                                   , render_pos_x
                                                   , block_size);
                    if (it->type != BLOCK_COMP && it->type != PLAYER_COMP
                    && update_entities && it->last_update_tick != actual_tick) {
                        it->last_update_tick = actual_tick;
                        it->update(grid_pos_x, grid_pos_y, season);
                        updateGame(renderer, camera, season, actual_tick, false);
                        updateGame(renderer, camera, season, actual_tick, false);
                    }
                }
            }
            render_pos_x += block_size;
        }
    }
    SDL_RenderPresent(renderer);
}

//-----------------------------------------------------------------------------

bool updatePlayer(unsigned* pos_x, unsigned* pos_y, char season, InputType input)
{
    bool acted = false;
    unsigned next_x = *pos_x;
    unsigned next_y = *pos_y;
    switch (input)
    {
    case UP_INPUT:
        next_y++;
        break;
    case DOWN_INPUT:
        next_y--;
        break;
    case LEFT_INPUT:
        next_x--;
        break;
    case RIGHT_INPUT:
        next_x++;
        break;
    default:
        break;
    }
    if ((next_x != *pos_x || next_y != *pos_y)
    &&  (next_x >= 0 && next_x < 1024)
    &&  (next_y >= 0 && next_y < 1024)) {
        bool can_move = (grid[next_y][next_x].type == BLANK_COMP);
        if (!can_move) {
            switch (grid[next_y][next_x].type) {
            case FOE_COMP:
                grid[next_y][next_x].foe_data.life -= 2;
                if (grid[next_y][next_x].foe_data.life <= 0) {
                    grid[next_y][next_x].type = BLANK_COMP;
                    can_move = true;
                }
                printf("[log] give 2 damage on %d, %d foe\n", next_x, next_y);
                break;
            default:
                break;
            }
        }
        if (can_move) {
            printf("[log] moving to %d, %d\n", next_x, next_y);
            grid[next_y][next_x] = grid[*pos_y][*pos_x];
            grid[*pos_y][*pos_x].type = BLANK_COMP;
            *pos_y = next_y;
            *pos_x = next_x;
            acted = true;
        } else {
            printf("[log] cell %d, %d is not blank\n", next_x, next_y);
        }
    } else {
        printf("[log] cell %d, %d is not valid\n", next_x, next_y);
    }
    return acted;
}

//-----------------------------------------------------------------------------

void dummyFoe(int bar, int baz, char foobar)
{
    printf("[log] dummy foe updated\n");
}

void initFoe(Component* component, UpdateFunction function, SDL_Texture* sprite,
             unsigned char life)
{
    component->type = FOE_COMP;
    component->last_update_tick = -1;
    component->update = function;
    component->sprite = sprite;
    component->foe_data.life = life;
}

//-----------------------------------------------------------------------------

#include "q-learning.h"

int main()
{
    int return_status = 0;
    h_screen_resolution = 800;
    v_screen_resolution = 600;

    assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
    SDL_Window* win = SDL_CreateWindow("game", 0, 0, h_screen_resolution
                                                   , v_screen_resolution
                                                   , 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    sprite_assets[0] = IMG_LoadTexture(renderer, "goomba.png");
    sprite_assets[1] = IMG_LoadTexture(renderer, "mario.png");
    sprite_assets[2] = IMG_LoadTexture(renderer, "blue.png");

    fillBackground(sprite_assets[2]);
    eraseGrid();

    initQLearningDemo();

    int tick_count = 0;
    updateGame(renderer, &main_camera, 0, 0, false);
    for (;;) {
        InputType input = NOT_INPUT;
        SDL_Event e;
        if ( SDL_PollEvent(&e) ) {
            if (e.type == SDL_QUIT)
                break;
            else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
                break;
                
            else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_i) {
                main_camera.y++;
                updateGame(renderer, &main_camera, 0, 0, false);
            } else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_k) {
                main_camera.y--;
                updateGame(renderer, &main_camera, 0, 0, false);
            } else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_l) {
                main_camera.x++;
                updateGame(renderer, &main_camera, 0, 0, false);
            } else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_j) {
                main_camera.x--;
                updateGame(renderer, &main_camera, 0, 0, false);
            }

            if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_w)
                input = UP_INPUT;
            else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_s)
                input = DOWN_INPUT;
            else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_a)
                input = LEFT_INPUT;
            else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_d)
                input = RIGHT_INPUT;
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_p)
                input = SKIP_INPUT;
        }
        if (input != NOT_INPUT) {
            if (input == SKIP_INPUT
            ||  updatePlayer(&player_grid_pos_x, &player_grid_pos_y, 0, input)) {
                updateGame(renderer, &main_camera, 0, tick_count, false);
                updateGame(renderer, &main_camera, 0, tick_count, true);
                tick_count++;
            }
        }
    }
    closeGame(win, renderer, sprite_assets, 3);

    return return_status;
}
