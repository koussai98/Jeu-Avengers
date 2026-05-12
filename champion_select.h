#ifndef CHAMPION_SELECT_H
#define CHAMPION_SELECT_H

#include "types.h"
#include "utils.h"


extern const char *CHAMPION_NOMS[CHAMPION_COUNT];

typedef struct {
    SDL_Texture *portraits[CHAMPION_COUNT];
    SDL_Rect     rects[CHAMPION_COUNT];
    int          hover;
    Bouton       btn_retour;
    Bouton       btn_valider;
    int          choix_temp;
} ChampionSelect;

void champion_select_init   (GameState *gs, ChampionSelect *cs);
void champion_select_input  (GameState *gs, ChampionSelect *cs);
void champion_select_update (GameState *gs, ChampionSelect *cs);
void champion_select_draw   (GameState *gs, ChampionSelect *cs);
void champion_select_free   (ChampionSelect *cs);

const char *champion_nom        (Champion c);
const char *champion_portrait_path(Champion c);

#endif
