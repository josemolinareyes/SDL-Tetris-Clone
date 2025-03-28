/*
 * Original code (C) 1995, Vadim Antonov
 * Port to RetroBSD (C) 2015, Serge Vakulenko
 * Port to SDL (C) 2025, José Molina Reyes (josemolinareyes@riseup.net)
 * Development assistance: DeepSeek-R1 AI (https://www.deepseek.com)
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The authors disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */

#define _CRT_SECURE_NO_WARNINGS
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// Game Constants
#define PITWIDTH     12
#define PITDEPTH     21
#define NSHAPES      7
#define NBLOCKS      5
#define FIN          999
#define BLOCK_SIZE   30
#define SCREEN_W     (PITWIDTH * BLOCK_SIZE + 200)
#define SCREEN_H     (PITDEPTH * BLOCK_SIZE)
#define MAX_PARTICLES 100

// Structures
typedef struct {
    int x, y;
} coord_t;

typedef struct {
    int dx, dy;
    coord_t p[NBLOCKS];
} shape_t;

typedef struct {
    int x, y;
    float life;
} Particle;

// SDL Globals
SDL_Window* window;
SDL_Renderer* renderer;
TTF_Font* font;

// Game State
struct {
    int pit[PITDEPTH + 1][PITWIDTH];
    int pitcnt[PITDEPTH];
    coord_t current[NBLOCKS];
    int score;
    int hi_score;
    int level;
    int lines;
    int next_piece;
    int speed;
    Uint32 last_update;
    Particle particles[MAX_PARTICLES];
    SDL_Color colors[NSHAPES];
} game;

const shape_t shapes[NSHAPES] = {
    {0,3, {{0,0},{0,1},{0,2},{0,3},{FIN,FIN}}}, // I
    {1,2, {{0,0},{1,0},{1,1},{1,2},{FIN,FIN}}}, // J
    {1,2, {{0,1},{1,0},{1,1},{1,2},{FIN,FIN}}}, // T
    {1,2, {{0,2},{1,0},{1,1},{1,2},{FIN,FIN}}}, // L
    {1,2, {{0,0},{0,1},{1,1},{1,2},{FIN,FIN}}}, // S
    {1,2, {{0,1},{0,2},{1,0},{1,1},{FIN,FIN}}}, // Z
    {1,1, {{0,0},{0,1},{1,0},{1,1},{FIN,FIN}}}, // O
};

void init_game() {
    memset(game.pit, 0, sizeof(game.pit));
    memset(game.pitcnt, 0, sizeof(game.pitcnt));
    game.score = game.lines = 0;
    game.level = 1;
    game.speed = 500;
    game.next_piece = rand() % NSHAPES;
    
    // Piece colors
    game.colors[0] = (SDL_Color){0,255,255,255};   // Cyan
    game.colors[1] = (SDL_Color){0,0,255,255};     // Blue
    game.colors[2] = (SDL_Color){128,0,128,255};   // Purple
    game.colors[3] = (SDL_Color){255,165,0,255};   // Orange
    game.colors[4] = (SDL_Color){0,255,0,255};     // Green
    game.colors[5] = (SDL_Color){255,0,0,255};     // Red
    game.colors[6] = (SDL_Color){255,255,0,255};   // Yellow

    // Load high score
    FILE* f = fopen("highscore.dat", "rb");
    if(f) {
        fread(&game.hi_score, sizeof(int), 1, f);
        fclose(f);
    }
}

void save_hi_score() {
    FILE* f = fopen("highscore.dat", "wb");
    if(f) {
        fwrite(&game.hi_score, sizeof(int), 1, f);
        fclose(f);
    }
}

void add_particle(int x, int y) {
    for(int i = 0; i < MAX_PARTICLES; i++) {
        if(game.particles[i].life <= 0) {
            game.particles[i] = (Particle){
                x * BLOCK_SIZE + rand() % BLOCK_SIZE,
                y * BLOCK_SIZE + rand() % BLOCK_SIZE,
                1.0f
            };
            break;
        }
    }
}

void update_particles(float delta) {
    for(int i = 0; i < MAX_PARTICLES; i++) {
        if(game.particles[i].life > 0) {
            game.particles[i].y += 50 * delta;
            game.particles[i].life -= delta;
        }
    }
}

void render_text(int x, int y, const char* text, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void translate(const shape_t* t, const coord_t* c, int a, coord_t* res) {
    int xw, yw;
    coord_t org;

    if(a & 1) { xw = t->dy; yw = t->dx; } 
    else { xw = t->dx; yw = t->dy; }

    org = *c;
    org.x -= (xw + 1) / 2;
    org.y -= yw / 2;

    if(org.y < 0) org.y = 0;
    if(org.y + yw >= PITWIDTH && c->y <= PITWIDTH)
        org.y = PITWIDTH - 1 - yw;

    for(int i = 0; t->p[i].x != FIN; i++) {
        switch(a) {
            case 0: res[i] = t->p[i]; break;
            case 3: res[i].x = xw - t->p[i].y; 
                    res[i].y = t->p[i].x; break;
            case 2: res[i].x = xw - t->p[i].x; 
                    res[i].y = yw - t->p[i].y; break;
            case 1: res[i].x = t->p[i].y; 
                    res[i].y = yw - t->p[i].x; break;
        }
        res[i].x += org.x;
        res[i].y += org.y;
    }
    res[i].x = res[i].y = FIN;

    // Bubble sort coordinates
    int swapped;
    do {
        swapped = 0;
        for(int j = 0; res[j+1].x != FIN; j++) {
            if((res[j].x > res[j+1].x) || 
               (res[j].x == res[j+1].x && res[j].y > res[j+1].y)) {
                coord_t tmp = res[j];
                res[j] = res[j+1];
                res[j+1] = tmp;
                swapped = 1;
            }
        }
    } while(swapped);
}

void move(coord_t* old, coord_t* new) {
    while(1) {
        if(old->x == FIN) {
            if(new->x == FIN) break;
            old++;
        } else if(new->x == FIN) {
            new++;
        } else if(old->x < new->x || 
                 (old->x == new->x && old->y < new->y)) {
            old++;
        } else {
            new++;
        }
    }
}

void stopped(coord_t* c) {
    int nfull = 0;
    for(; c->x != FIN; c++) {
        if(c->x <= 0) { 
            memset(game.pit, 0, sizeof(game.pit));
            return;
        }
        game.pit[c->x][c->y] = 1;
        game.pitcnt[c->x]++;
        if(game.pitcnt[c->x] == PITWIDTH) nfull++;
    }

    switch(nfull) {
        case 1: game.score += 40 * game.level; break;
        case 2: game.score += 100 * game.level; break;
        case 3: game.score += 300 * game.level; break;
        case 4: game.score += 1200 * game.level; break;
    }

    if(nfull > 0) {
        for(int y = 0; y < PITDEPTH; y++) {
            if(game.pitcnt[y] == PITWIDTH) {
                for(int x = 0; x < PITWIDTH; x++) {
                    add_particle(x, y);
                }
            }
        }
    }
}

int handle_input() {
    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        if(e.type == SDL_QUIT) return 1;
        
        if(e.type == SDL_KEYDOWN) {
            switch(e.key.keysym.sym) {
                case SDLK_LEFT:   return JOYSTICK_LEFT;
                case SDLK_RIGHT:  return JOYSTICK_RIGHT;
                case SDLK_UP:     return JOYSTICK_UP;
                case SDLK_DOWN:   return JOYSTICK_DOWN;
                case SDLK_SPACE:  return JOYSTICK_SELECT;
                case SDLK_ESCAPE: return 1;
            }
        }
    }
    return JOYSTICK_IDLE;
}

void render_game(int current_piece) {
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    // Draw game board
    for(int y = 0; y < PITDEPTH; y++) {
        for(int x = 0; x < PITWIDTH; x++) {
            if(game.pit[y][x]) {
                SDL_Rect rect = {
                    x * BLOCK_SIZE,
                    y * BLOCK_SIZE,
                    BLOCK_SIZE - 1,
                    BLOCK_SIZE - 1
                };
                SDL_Color c = game.colors[game.pit[y][x] - 1];
                SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    // Draw particles
    for(int i = 0; i < MAX_PARTICLES; i++) {
        if(game.particles[i].life > 0) {
            SDL_SetRenderDrawColor(renderer, 
                255, 255, 255, (Uint8)(game.particles[i].life * 255));
            SDL_Rect p = {
                game.particles[i].x,
                game.particles[i].y,
                3, 3
            };
            SDL_RenderFillRect(renderer, &p);
        }
    }

    // Draw UI
    SDL_Color white = {255, 255, 255, 255};
    char buffer[50];
    int panel_x = PITWIDTH * BLOCK_SIZE + 20;
    
    render_text(panel_x, 20, "SCORE", white);
    sprintf(buffer, "%08d", game.score);
    render_text(panel_x, 50, buffer, game.colors[3]);

    render_text(panel_x, 100, "LEVEL", white);
    sprintf(buffer, "%02d", game.level);
    render_text(panel_x, 130, buffer, game.colors[4]);

    render_text(panel_x, 180, "NEXT", white);
    SDL_Color next_color = game.colors[game.next_piece];
    for(int i = 0; shapes[game.next_piece].p[i].x != FIN; i++) {
        SDL_Rect rect = {
            panel_x + shapes[game.next_piece].p[i].y * BLOCK_SIZE,
            220 + shapes[game.next_piece].p[i].x * BLOCK_SIZE,
            BLOCK_SIZE - 2,
            BLOCK_SIZE - 2
        };
        SDL_SetRenderDrawColor(renderer, next_color.r, next_color.g, next_color.b, 255);
        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    font = TTF_OpenFont("arial.ttf", 24);
    
    window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, SCREEN_W, SCREEN_H, 0);
    renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    srand(time(NULL));
    init_game();

    int ptype = game.next_piece;
    game.next_piece = rand() % NSHAPES;
    int angle = 0;
    coord_t c = { -2, PITWIDTH / 2 - 1 };
    coord_t new[NBLOCKS], old[NBLOCKS], chk[NBLOCKS];

    Uint32 last_frame = SDL_GetTicks();
    int running = 1;
    
    while(running) {
        Uint32 current = SDL_GetTicks();
        float delta = (current - last_frame) / 1000.0f;
        last_frame = current;

        int key = handle_input();
        if(key == 1) break;

        // Game logic
        if(current - game.last_update > game.speed) {
            coord_t cnew = c;
            cnew.x++;
            translate(&shapes[ptype], &cnew, angle, chk);
            
            int collision = 0;
            for(coord_t* cp = chk; cp->x != FIN; cp++) {
                if(cp->x >= 0 && game.pit[cp->x][cp->y]) {
                    collision = 1;
                    break;
                }
            }
            
            if(collision) {
                stopped(new);
                ptype = game.next_piece;
                game.next_piece = rand() % NSHAPES;
                c = (coord_t){ -2, PITWIDTH / 2 - 1 };
                angle = 0;
                translate(&shapes[ptype], &c, angle, new);
            } else {
                memcpy(old, new, sizeof(old));
                memcpy(new, chk, sizeof(new));
                c = cnew;
            }
            
            game.last_update = current;
        }

        update_particles(delta);
        render_game(ptype);
        SDL_Delay(10);
    }

    save_hi_score();
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
