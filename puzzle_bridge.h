#ifndef PUZZLE_BRIDGE_H
#define PUZZLE_BRIDGE_H



#include "types.h"      
#include "menu_puzzle.h" 


extern int puzzle_solved;
extern int puzzle_failed;
extern int game_over;


typedef struct {
    Game       g;          
    TTF_Font  *font_small; 
    int        result;     
} PuzzleContext;


void puzzle_bridge_init(PuzzleContext *ctx, GameState *gs);


void puzzle_bridge_handle_event(PuzzleContext *ctx, SDL_Event *e);


void puzzle_bridge_draw(PuzzleContext *ctx);


void puzzle_bridge_free(PuzzleContext *ctx);

#endif 
