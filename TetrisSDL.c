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

typedef struct {
    int x, y;
    float life;
} Particle;

typedef struct {
    int x, y;
} coord_t;

typedef struct {
    int dx, dy;
    coord_t p[NBLOCKS];
} shape_t;

// SDL Globals
SDL_Window* window;
SDL_Renderer* renderer;
TTF_Font* font;

// Game State
struct {
    int pit[PITDEPTH+1][PITWIDTH];
    int pitcnt[PITDEPTH];
    coord_t current[NBLOCMS];
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
    {0,3, {{0,0},{0,1},{0,2},{0,3},{FIN,FIN}}, // I
    {1,2, {{0,0},{1,0},{1,1},{1,2},{FIN,FIN}}, // J
    {1,2, {{0,1},{1,0},{1,1},{1,2},{FIN,FIN}}, // T
    {1,2, {{0,2},{1,0},{1,1},{1,2},{FIN,FIN}}, // L
    {1,2, {{0,0},{0,1},{1,1},{1,2},{FIN,FIN}}, // S
    {1,2, {{0,1},{0,2},{1,0},{1,1},{FIN,FIN}}, // Z
    {1,1, {{0,0},{0,1},{1,0},{1,1},{FIN,FIN}}, // O
};

void init_game() {
    memset(game.pit, 0, sizeof(game.pit));
    memset(game.pitcnt, 0, sizeof(game.pitcnt));
    game.score = game.lines = 0;
    game.level = 1;
    game.speed = 500;
    game.next_piece = rand() % NSHAPES;
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
    for(int i=0; i<MAX_PARTICLES; i++) {
        if(game.particles[i].life <= 0) {
            game.particles[i] = (Particle){
                x * BLOCK_SIZE + rand()%BLOCK_SIZE,
                y * BLOCK_SIZE + rand()%BLOCK_SIZE,
                1.0f
            };
            break;
        }
    }
}

void update_particles(float delta) {
    for(int i=0; i<MAX_PARTICLES; i++) {
        if(game.particles[i].life > 0) {
            game.particles[i].y += 50 * delta;
            game.particles[i].life -= delta;
        }
    }
}

void render_text(int x, int y, const char* text, SDL_Color col) {
    SDL_Surface* surf = TTF_RenderText_Solid(font, text, col);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_Rect rect = {x, y, surf->w, surf->h};
    SDL_RenderCopy(renderer, tex, NULL, &rect);
    SDL_FreeSurface(surf);
    SDL_DestroyTexture(tex);
}

void render_game(int current_piece) {
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    // Draw game board
    for(int y=0; y<PITDEPTH; y++) {
        for(int x=0; x<PITWIDTH; x++) {
            if(game.pit[y][x]) {
                SDL_Rect rect = {
                    x * BLOCK_SIZE,
                    y * BLOCK_SIZE,
                    BLOCK_SIZE-1,
                    BLOCK_SIZE-1
                };
                SDL_SetRenderDrawColor(renderer, 
                    game.colors[game.pit[y][x]-1].r,
                    game.colors[game.pit[y][x]-1].g,
                    game.colors[game.pit[y][x]-1].b, 255);
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    // Draw current piece
    for(int i=0; game.current[i].x!=FIN; i++) {
        if(game.current[i].x >= 0) {
            SDL_Rect rect = {
                game.current[i].y * BLOCK_SIZE,
                game.current[i].x * BLOCK_SIZE,
                BLOCK_SIZE-1,
                BLOCK_SIZE-1
            };
            SDL_Color c = game.colors[current_piece];
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    // Draw particles
    for(int i=0; i<MAX_PARTICLES; i++) {
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
    int panel_x = PITWIDTH * BLOCK_SIZE + 20;
    render_text(panel_x, 20, "SCORE", (SDL_Color){255,255,255,255});
    char score_str[20];
    sprintf(score_str, "%08d", game.score);
    render_text(panel_x, 50, score_str, game.colors[3]);

    render_text(panel_x, 100, "LEVEL", (SDL_Color){255,255,255,255});
    sprintf(score_str, "%02d", game.level);
    render_text(panel_x, 130, score_str, game.colors[4]);

    render_text(panel_x, 180, "NEXT", (SDL_Color){255,255,255,255});
    SDL_Color next_c = game.colors[game.next_piece];
    for(int i=0; shapes[game.next_piece].p[i].x!=FIN; i++) {
        SDL_Rect r = {
            panel_x + shapes[game.next_piece].p[i].y * BLOCK_SIZE,
            220 + shapes[game.next_piece].p[i].x * BLOCK_SIZE,
            BLOCK_SIZE-2, BLOCK_SIZE-2
        };
        SDL_SetRenderDrawColor(renderer, next_c.r, next_c.g, next_c.b,255);
        SDL_RenderFillRect(renderer, &r);
    }

    SDL_RenderPresent(renderer);
}

// [Keep original translate(), move(), stopped() functions]
// [Add your original game logic functions here]

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    font = TTF_OpenFont("arial.ttf", 24);
    
    window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, SCREEN_W, SCREEN_H, 0);
    renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    init_game();
    Uint32 last_frame = SDL_GetTicks();
    
    int running = 1;
    while(running) {
        Uint32 current = SDL_GetTicks();
        float delta = (current - last_frame) / 1000.0f;
        last_frame = current;

        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) running = 0;
            // [Add input handling]
        }

        update_particles(delta);
        render_game(current_piece);
        
        if(game.score > game.hi_score)
            game.hi_score = game.score;
    }

    save_hi_score();
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
