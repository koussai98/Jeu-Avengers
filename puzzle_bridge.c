
#include "puzzle_bridge.h"
#include <string.h>
#include <stdio.h>


extern int puzzle_solved;
extern int puzzle_failed;
extern int game_over;

/* 
 *    ctx->g  gs
 *    gs->renderer   → g.renderer
 *    gs->font_large → g.font_large  
 *    gs->font       → g.font_medium 
 *    gs->font       → g.font_small  
 *    gs->son_bref   → g.sfx_hover + g.sfx_click
 *    gs->backg.niveau → g.state 
 *    volume fixe 80 → g.volume 
 */
void puzzle_bridge_init(PuzzleContext *ctx, GameState *gs)
{
    Game *g = &ctx->g;

    memset(g, 0, sizeof(*g));

    
    g->renderer     = gs->renderer;
    g->font_large   = gs->font_large;       
    g->font_medium  = gs->font;             
    g->font_small   = gs->font;             

    
    g->sfx_hover    = gs->son_bref;
    g->sfx_click    = gs->son_bref;

    
    g->music_main    = NULL;
    g->music_sub     = NULL;
    g->music_victory = NULL;
    g->music_quiz    = NULL;

    
    g->volume       = 80;

    
    g->state        = STATE_PUZZLE;

    
    ctx->result     = 0;


    {
        const char *img_paths[5];
        SDL_Surface *surf;
        int i;

        
        extern SDL_Texture *puzzle_textures[5];

        img_paths[0] = "assets/images/thor.png";
        img_paths[1] = "assets/images/hulk.png";
        img_paths[2] = "assets/images/ironman.png";
        img_paths[3] = "assets/images/capamerica.png";
        img_paths[4] = "assets/images/thanos.png";

        for (i = 0; i < 5; i++) {
            if (puzzle_textures[i]) continue;   
            surf = IMG_Load(img_paths[i]);
            if (surf) {
                puzzle_textures[i] = SDL_CreateTextureFromSurface(gs->renderer, surf);
                SDL_FreeSurface(surf);
            }
            
        }
    }

    
    menu_puzzle_init(g);
}


void puzzle_bridge_handle_event(PuzzleContext *ctx, SDL_Event *e)
{
    Game *g = &ctx->g;

    menu_puzzle_handle_event(g, e);

    
    if (puzzle_solved && !game_over) {
        ctx->result = 1;    
    } else if (game_over || g->state == STATE_ENIGMA) {
        ctx->result = -1;   
    } else {
        ctx->result = 0;    
    }
}


void puzzle_bridge_draw(PuzzleContext *ctx)
{

    SDL_Renderer *r = ctx->g.renderer;
    SDL_Rect viewport;
    int offset_x;

    offset_x       = (WINDOW_W - SCREEN_W) / 2;   
    viewport.x     = -offset_x;
    viewport.y     = 0;
    viewport.w     = WINDOW_W;
    viewport.h     = SCREEN_H;

    SDL_RenderSetViewport(r, &viewport);
    menu_puzzle_draw(&ctx->g);
    SDL_RenderSetViewport(r, NULL);   
}


void puzzle_bridge_free(PuzzleContext *ctx)
{
    extern SDL_Texture *puzzle_textures[5];
    int i;

    for (i = 0; i < 5; i++) {
        if (puzzle_textures[i]) {
            SDL_DestroyTexture(puzzle_textures[i]);
            puzzle_textures[i] = NULL;
        }
    }

    
    ctx->g.sfx_hover = NULL;
    ctx->g.sfx_click = NULL;
    ctx->result      = 0;
}
