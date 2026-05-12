#ifndef ENTITES_H
#define ENTITES_H

#include "types.h"
#include "joueur.h"

#define MAX_COINS       30
#define MAX_OBSTACLES   16
#define MAX_TRAPS       12
#define MAX_ENEMIES     20
#define MAX_PROJECTILES 24

#define COIN_RADIUS      14
#define COIN_VALEUR      10

#define ENEMY_RADIUS_DEFAULT  26
#define ENEMY_HP_MELEE         2
#define ENEMY_HP_RANGED        1
#define ENEMY_HP_LOKI          8
#define ENEMY_SPEED_MELEE      1.6f
#define ENEMY_SPEED_RANGED     0.8f
#define ENEMY_SPEED_LOKI       1.2f
#define ENEMY_TOUCH_DAMAGE_CD 60
#define ENEMY_HIT_FLASH        6
#define LOKI_RADIUS           45
#define LOKI_HIT_FLASH        12

#define TRAP_DAMAGE_CD       60


#define ENEMI_SHEET_W       1024
#define ENEMI_SHEET_H        768
#define ENEMI_COLS             4
#define ENEMI_ROWS             3
#define ENEMI_FRAME_W       (ENEMI_SHEET_W / ENEMI_COLS)
#define ENEMI_FRAME_H       (ENEMI_SHEET_H / ENEMI_ROWS)


#define ENEMI2_SHEET_W       637
#define ENEMI2_SHEET_H      1024
#define ENEMI2_COLS            4
#define ENEMI2_ROWS            4
#define ENEMI2_FRAME_W      (ENEMI2_SHEET_W / ENEMI2_COLS)
#define ENEMI2_FRAME_H      (ENEMI2_SHEET_H / ENEMI2_ROWS)


#define LOKI_SHEET_W        1024
#define LOKI_SHEET_H        1024
#define LOKI_COLS              4
#define LOKI_ROWS              4
#define LOKI_FRAME_W        (LOKI_SHEET_W / LOKI_COLS)
#define LOKI_FRAME_H        (LOKI_SHEET_H / LOKI_ROWS)

#define ANIM_TICKS_PER_FRAME   8

typedef struct {
    float x, y;
    int   actif;
    int   spin;
} Coin;

typedef struct {
    SDL_Rect rect;
    int      actif;
    int      texture_idx;
} Obstacle;

typedef struct {
    SDL_Rect rect;
    int      actif;
} Trap;

typedef enum {
    ENEMY_MELEE  = 0,
    ENEMY_RANGED = 1,
    ENEMY_LOKI   = 2
} EnemyType;

typedef struct {
    float     x, y;
    float     vx, vy;
    int       rayon;
    int       hp;
    int       hp_max;
    int       actif;
    int       mort;
    int       flash;
    EnemyType type;
    int       fire_cd;
    int       facing;


    int       anim_frame;
    int       anim_timer;
    int       attack_timer;
} Enemy;

typedef struct {
    float x, y;
    float vx;
    int   actif;
    int   from_enemy;
} Projectile;

typedef struct {
    Coin       coins[MAX_COINS];
    Obstacle   obstacles[MAX_OBSTACLES];
    Trap       traps[MAX_TRAPS];
    Enemy      enemies[MAX_ENEMIES];
    Projectile projectiles[MAX_PROJECTILES];


    SDL_Texture *obstacle_tex[4];
    SDL_Texture *enemi_tex;
    SDL_Texture *enemi2_tex;
    SDL_Texture *loki_tex;
} EntiteSet;


void entites_init           (EntiteSet *e);
void entites_charger_textures(EntiteSet *e, SDL_Renderer *renderer);
void entites_liberer_textures(EntiteSet *e);
void entites_clear           (EntiteSet *e);


int  entites_ajouter_coin    (EntiteSet *e, float x, float y);
int  entites_ajouter_obstacle(EntiteSet *e, int x, int y, int w, int h, int tex_idx);
int  entites_ajouter_trap    (EntiteSet *e, int x, int y, int w, int h);
int  entites_ajouter_enemy   (EntiteSet *e, EnemyType t, float x, float y);


int  entites_compter_enemies_vivants(EntiteSet *e);
int  entites_loki_present           (EntiteSet *e);
int  entites_loki_mort              (EntiteSet *e);


int  projectile_ajouter(EntiteSet *e, float x, float y, float vx, int from_enemy);


void entites_update              (EntiteSet *e, Personnage *joueur);
void entites_collision_obstacles (EntiteSet *e, Personnage *joueur, float old_x, float old_y);
void entites_player_attack       (EntiteSet *e, Personnage *joueur);


void entites_draw(GameState *gs, EntiteSet *e);


void draw_filled_circle(SDL_Renderer *r, int cx, int cy, int rad,
                        Uint8 R, Uint8 G, Uint8 B, Uint8 A);
int  rect_overlap(const SDL_Rect *a, const SDL_Rect *b);
int  circle_rect_overlap(float cx, float cy, int rad, const SDL_Rect *r);

#endif
