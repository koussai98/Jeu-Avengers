#ifndef MENU_PUZZLE_H
#define MENU_PUZZLE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include "types.h"

#define WINDOW_W     960
#define WINDOW_H     600
#define MAX_NAME_LEN  32
#define MAX_SCORES     3

#ifndef CLAMP_VAL
#define CLAMP_VAL(x, lo, hi) ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))
#endif

#define STATE_ENIGMA   8
#define STATE_PUZZLE  10
#define STATE_QUIT    11



typedef struct {
    char name[MAX_NAME_LEN];
    int  score;
} ScoreEntry;

typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    TTF_Font     *font_large;
    TTF_Font     *font_medium;
    TTF_Font     *font_small;
    Mix_Chunk    *sfx_hover;
    Mix_Chunk    *sfx_click;
    Mix_Music    *music_main;
    Mix_Music    *music_sub;
    Mix_Music    *music_victory;
    Mix_Music    *music_quiz;
    int           state;       // au lieu de GameState state;
    int           volume;
    int           fullscreen;
    int           brightness;
    int           player_mode;
    int           avatar_sel;
    int           input_sel;
    char          name_input[MAX_NAME_LEN];
    int           name_cursor;
    ScoreEntry    scores[MAX_SCORES];
    int           quiz_answer;
    int           quiz_selected;
    int           running;
    Uint32        frame_tick;
} Game;

typedef struct {
    SDL_Rect  rect;
    char      label[64];
    int       hovered;
    int       active;
    SDL_Color color_normal;
    SDL_Color color_hover;
    SDL_Color color_text;
} Button;

typedef struct {
    int       px;
    int       py;
    int       active;
    SDL_Rect  rect;
    SDL_Color color;
} PuzzlePiece;

typedef struct {
    SDL_Rect  rect;
    SDL_Color color;
    int       is_correct;
    int       dragging;
    int       drag_offset_x;
    int       drag_offset_y;
} OptionPiece;

#define color_gold()  ((SDL_Color){212, 160,  23, 255})
#define color_red()   ((SDL_Color){180,  20,  20, 255})
#define color_grey()  ((SDL_Color){ 90,  90,  90, 255})
#define color_white() ((SDL_Color){230, 230, 230, 255})

void menu_puzzle_init(Game *g);
void menu_puzzle_handle_event(Game *g, SDL_Event *e);
void menu_puzzle_draw(Game *g);

#endif
