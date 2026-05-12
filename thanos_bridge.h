#ifndef THANOS_BRIDGE_H
#define THANOS_BRIDGE_H

#include "types.h"
#include "joueur.h"
#include "npc.h"

#define THANOS_TOUCH_DAMAGE_CD   75
#define THANOS_HEAVY_DAMAGE_CD   90

typedef struct {
    Npc  npc;
    int  initialise;
    int  player_was_attacking;
    int  hit_done_this_attack;
} Thanos;

void thanos_init   (Thanos *t, SDL_Renderer *ren);
void thanos_free   (Thanos *t);
void thanos_reset  (Thanos *t, float x, float y);

void thanos_update (Thanos *t, Personnage *joueur);
void thanos_draw   (GameState *gs, Thanos *t);

int  thanos_est_mort(Thanos *t);

#endif
