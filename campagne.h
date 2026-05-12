#ifndef CAMPAGNE_H
#define CAMPAGNE_H

#include "types.h"
#include "joueur.h"
#include "entites.h"
#include "thanos_bridge.h"

#define CAMPAGNE_WORLD_W      1600
#define MINIMAP_W              160
#define MINIMAP_H               60

typedef enum {
    ETAPE_LEVEL_1A   = 0,
    ETAPE_LEVEL_1B   = 1,
    ETAPE_LEVEL_2    = 2,
    ETAPE_BOSS       = 3,
    ETAPE_VICTOIRE   = 4,
    ETAPE_GAMEOVER   = 5
} EtapeCampagne;

typedef struct {
    EtapeCampagne  etape;
    EntiteSet      entites;
    Personnage     joueur;
    Thanos         thanos;

    int            world_w;
    int            camera_x;

    int            message_timer;
    char           message[80];

    int            spawn_timer;
    int            wave_index;
    int            loki_spawned;

    int            etape_courante_init;
    int            attente_finale;
    Uint32         victory_start_ms;


    SDL_Texture   *bg_textures[3];
    SDL_Texture   *portrait_champion;
} Campagne;


void campagne_init   (GameState *gs, Campagne *c);
void campagne_liberer(Campagne *c);
void campagne_reset  (GameState *gs, Campagne *c);
void campagne_revive (Campagne *c);


void campagne_input  (GameState *gs, Campagne *c);
void campagne_update (GameState *gs, Campagne *c);
void campagne_draw   (GameState *gs, Campagne *c);


void afficher_message_camp(Campagne *c, const char *txt, int duree_frames);
void setup_level_1a       (Campagne *c);
void setup_level_1b       (Campagne *c);
void setup_level_2        (Campagne *c);
void setup_boss_thanos    (GameState *gs, Campagne *c);
void update_spawn_1b      (Campagne *c);
void update_spawn_level2  (Campagne *c);
void draw_hud_campagne    (GameState *gs, Campagne *c);
void draw_minimap         (GameState *gs, Campagne *c);

#endif
