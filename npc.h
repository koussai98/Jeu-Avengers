#ifndef NPC_H_INCLUDED
#define NPC_H_INCLUDED

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

/* ═══════════════════════════════
 * TUNABLE CONSTANTS
 * ═══════════════════════════════ */
#define NPC_W              110
#define NPC_H              150
#define NPC_SPEED_WANDER    1.0f
#define NPC_SPEED_HUNT      2.0f
#define NPC_SPEED_MAX       4.5f
#define NPC_AGGRO_MIN       90.0f
#define NPC_AGGRO_MAX      260.0f
#define NPC_AGGRO_GROW      0.15f
#define NPC_HP_MAX          5
#define NPC_HIT_CD          40
#define NPC_STUN_LEN        55
#define NPC_DEATH_HOLD      60
#define NPC_WORLD_W         800
#define NPC_WORLD_H         600

#define NPC_MAX_COINS       8
#define NPC_COIN_W          48
#define NPC_COIN_H          48
#define NPC_COIN_RADIUS     36

/* ═══════════════════════════════
 * INTERNAL ANIMATION (self-contained, no anim.h)
 * ═══════════════════════════════ */
#define NPC_MAX_FRAMES 10
typedef struct {
    SDL_Texture *frames[NPC_MAX_FRAMES];
    int count, cur, timer, spd, loop, done;
} NpcAnim;

/* ═══════════════════════════════
 * ENUMS
 * ═══════════════════════════════ */
typedef enum { NPC_WANDER, NPC_HUNT }   NpcBehaviour;

typedef enum {
    NANIM_WALK,
    NANIM_WALK_RAGE,
    NANIM_PUNCH,
    NANIM_HEAVY_CHARGE,
    NANIM_HEAVY_HIT,
    NANIM_HIT,
    NANIM_DEATH,
    NANIM_TAUNT,
    NANIM_COUNT
} NpcAnimSlot;

/* ═══════════════════════════════
 * COIN SYSTEM
 * ═══════════════════════════════ */
typedef struct { float x, y; int alive; } NpcCoin;

typedef struct {
    NpcCoin  pool[NPC_MAX_COINS];
    int      active;    /* coins currently on screen */
    int      gathered;  /* lifetime collected        */
    NpcAnim  spin;      /* shared spin animation     */
} NpcCoins;

/* ═══════════════════════════════
 * NPC STRUCT
 * ═══════════════════════════════ */
typedef struct {
    float        x, y;          /* top-left position  */
    float        dest_x, dest_y;/* wander target      */
    float        vx, vy;
    int          grounded;

    NpcBehaviour behaviour;
    int          facing;        /* +1 right, -1 left  */
    float        threat;        /* 0..1 rage level    */
    float        aggro_grow;    /* 0..1 radius growth */
    float        aggro_r;       /* current aggro ring */
    float        move_spd;      /* current speed      */

    int          hp;
    int          hit_cd;        /* invincibility frames    */
    int          stun;          /* knockback stun frames   */
    int          flash;         /* blink frames on hit     */
    int          atk_cd;        /* punch cooldown          */
    int          heavy_cd;      /* heavy attack cooldown   */
    int          dead;
    int          death_hold;    /* countdown after death anim */

    NpcAnim      anim[NANIM_COUNT];
    NpcAnimSlot  slot;          /* current animation slot */
} Npc;

/* ═══════════════════════════════
 * PUBLIC API
 * ═══════════════════════════════ */
void npc_init   (Npc *n, float x, float y, SDL_Renderer *ren);
void npc_free   (Npc *n);
void npc_respawn(Npc *n, float px, float py);
void npc_hurt   (Npc *n);
void npc_tick   (Npc *n, float px, float py, SDL_Rect *plats, int nplat, NpcCoins *coins);
void npc_draw   (SDL_Renderer *ren, const Npc *n);

void coins_init  (NpcCoins *c, SDL_Renderer *ren);
void coins_free  (NpcCoins *c);
void coins_burst (NpcCoins *c, float cx, float cy, int n);
int  coins_tick  (NpcCoins *c, float px, float py, int pw, int ph);
void coins_draw  (SDL_Renderer *ren, NpcCoins *c);

int  npc_bbox_hit(float ax, float ay, int aw, int ah,
                  float bx, float by, int bw, int bh);

#endif
