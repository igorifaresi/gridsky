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
};

typedef enum ComponentTypes ComponentType;

typedef void (*UpdateFunction)(int, int, char);

typedef struct Component {
    ComponentType type;
    size_t last_update_tick;
    SDL_Texture* sprite;
    // union {
    // }
    UpdateFunction update;
}Component;

Camera main_camera;
SDL_Texture* background[1024][1024];
Component grid[1024][1024];
int h_screen_resolution;
int v_screen_resolution;

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
    int block_offset = (h_block_qnt - camera->v_block_qnt) / 2;

    printf("block_size            -> %d\n", block_size);
    printf("h_block_qnt           -> %d\n", h_block_qnt);
    printf("init_render_block_pos -> %d\n", init_render_block_pos);
    printf("block_offset          -> %d\n", block_offset);

    assert(SDL_RenderClear(renderer) != -1);
    for (int y = 0; y < camera->v_block_qnt; y++) {
        int render_pos_x = init_render_block_pos;
        int render_pos_y = block_size * y;
        for (int x = 0; x < h_block_qnt + camera->v_block_qnt; x++) {
            int grid_pos_x = camera->x + x + block_offset;
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
                    printf("rendering comp\n");
                    renderCell(renderer, it->sprite, render_pos_y
                                                   , render_pos_x
                                                   , block_size);
                    if (it->type != BLOCK_COMP && update_entities
                    &&  it->last_update_tick != actual_tick) {
                        it->update(grid_pos_x, grid_pos_y, season);
                        it->last_update_tick = actual_tick;
                    }
                }
            }
            render_pos_x += block_size;
        }
    }
    SDL_RenderPresent(renderer);
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

int main()
{
    int return_status = 0;
    h_screen_resolution = 800;
    v_screen_resolution = 600;

    assert(SDL_Init(SDL_INIT_VIDEO) >= 0);
    SDL_Window* win = SDL_CreateWindow("game", 0, 0, 800
                                                   , 600
                                                   , 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    sprite_assets[0] = IMG_LoadTexture(renderer, "greatsword.png");
    sprite_assets[1] = IMG_LoadTexture(renderer, "fftatics_icons.png");

    main_camera.x = 0;
    main_camera.y = 0;
    main_camera.v_block_qnt = 12;

    fillBackground(sprite_assets[0]);
    eraseGrid();
    Component c;
    c.type = BLOCK_COMP;
    c.last_update_tick = 0;
    c.sprite = sprite_assets[1];
    c.update = NULL;
    grid[0][3] = c;
    for (;;) {
        SDL_Event e;
		if ( SDL_PollEvent(&e) ) {
			if (e.type == SDL_QUIT)
				break;
			else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE)
				break;
            else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_i)
                main_camera.y++;
            else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_k)
                main_camera.y--;
            else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_l)
                main_camera.x++;
            else if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_j)
                main_camera.x--;
		}
        updateGame(renderer, &main_camera, 0, 0, false);
    }
    closeGame(win, renderer, sprite_assets, 3);

    return return_status;
}
