#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

// ─── Window / Renderer ───────────────────────────────────────────────────────
#define SCREEN_W 800
#define SCREEN_H 600

// ─── Background levels (Backg.niveau) ────────────────────────────────────────
typedef enum {
    NIVEAU_MENU      = 0,
    NIVEAU_OPTIONS   = 1,
    NIVEAU_SAUVEGARDE= 2,
    NIVEAU_JOUEUR    = 3,
    NIVEAU_SCORES    = 4,
    NIVEAU_ENIGME    = 5,
    NIVEAU_JEU       = 6,   /* gameplay actif                         */
    NIVEAU_PUZZLE    = 7,   /* puzzle déclenché à la mort du joueur   */
    NIVEAU_COSTUME_J1= 8,   /* sous-menu costume joueur 1             */
    NIVEAU_COSTUME_J2= 9,   /* sous-menu costume joueur 2             */
    NIVEAU_CAMPAGNE  = 10,  /* mode campagne (single-player)          */
    NIVEAU_CHAMPION  = 11,  /* écran de sélection du champion         */
    NIVEAU_QUIZ      = 12,  /* écran du quiz                          */
    NIVEAU_DEATH_CHOICE = 13 /* le joueur choisit puzzle ou quiz      */
} Niveau;


typedef enum {
    CHAMPION_THOR       = 0,
    CHAMPION_HULK       = 1,
    CHAMPION_IRONMAN    = 2,
    CHAMPION_CAPAMERICA = 3,
    CHAMPION_COUNT      = 4
} Champion;

// ─── Button structure ─────────────────────────────────────────────────────────
typedef struct {
    SDL_Texture *img_normal;   /* image repos   */
    SDL_Texture *img_actif;    /* image survol  */
    SDL_Rect     rect;         /* position + taille */
    int          actif;        /* 1 = souris dessus */
    char         label[64];    /* texte du bouton   */
} Bouton;

// ─── Background structure ─────────────────────────────────────────────────────
typedef struct {
    SDL_Texture *image;
    SDL_Rect     rect;
    Niveau       niveau;
    int          volume;       /* 0-128 */
    int          mode;         /* 0=normal, 1=fullscreen */
    Mix_Music   *musique;
} Background;

// ─── Global game state ────────────────────────────────────────────────────────
typedef struct {
    SDL_Window   *window;
    SDL_Renderer *renderer;
    TTF_Font     *font;
    TTF_Font     *font_large;
    int           continuer;   /* 0 = quitter */
    int           click;
    int           motion;
    int           mouse_x;
    int           mouse_y;
    char          input[32];   /* dernière touche/action */
    Background    backg;       /* background courant     */
    Mix_Chunk    *son_bref;    /* son survol bouton      */
    Niveau        puzzle_origin;     /* set before NIVEAU_PUZZLE/QUIZ — JEU or CAMPAGNE */
    int           death_count;       /* total deaths this session — drives puzzle/quiz alternation */
    Champion      champion_choisi;   /* champion sélectionné pour la campagne */
    int           camera_x;          /* used by campaign drawing; 0 in PvP */
} GameState;

#endif /* TYPES_H */
