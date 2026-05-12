/*
 * npc.c — self-contained NPC + coin module
 * Assets used (all .png, all in assets/ folder):
 *   Walk:          right1..right8
 *   Walk rage:     right1rage..right8rage
 *   Punch:         rightpunchattack1..rightpunchattack4
 *   Heavy charge:  heavyattackcharge1..heavyattackcharge4
 *   Heavy hit:     heavyattackkda1
 *   Hit:           righthit1..righthit3
 *   Death:         right1death..right3death
 *   Taunt:         taunt1..taunt3
 *   Coins:         coin1..coin4
 */
#include "npc.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

/* ════════════════════════════════════════
 * INTERNAL ANIMATION HELPERS
 * ════════════════════════════════════════ */

/* Load frames with an explicit list of full paths */
static void _load_paths(NpcAnim *a, SDL_Renderer *ren,
                        const char **paths, int count,
                        int spd, int loop) {
    a->count = 0; a->cur = 0; a->timer = spd;
    a->spd   = spd; a->loop = loop; a->done = 0;
    for (int i = 0; i < count && i < NPC_MAX_FRAMES; i++) {
        SDL_Surface *s = IMG_Load(paths[i]);
        if (!s) { fprintf(stderr, "NPC: missing %s\n", paths[i]); continue; }
        a->frames[a->count++] = SDL_CreateTextureFromSurface(ren, s);
        SDL_FreeSurface(s);
    }
}

/* Load numbered sequence: assets/<prefix><N>.png  N=1..count */
static void _load_seq(NpcAnim *a, SDL_Renderer *ren,
                      const char *prefix, int count,
                      int spd, int loop) {
    a->count = 0; a->cur = 0; a->timer = spd;
    a->spd   = spd; a->loop = loop; a->done = 0;
    for (int i = 0; i < count && i < NPC_MAX_FRAMES; i++) {
        char path[256];
        snprintf(path, sizeof(path), "assets/thanos/%s%d.png", prefix, i + 1);
        SDL_Surface *s = IMG_Load(path);
        if (!s) { fprintf(stderr, "NPC: missing %s\n", path); continue; }
        a->frames[a->count++] = SDL_CreateTextureFromSurface(ren, s);
        SDL_FreeSurface(s);
    }
}

static void _anim_reset(NpcAnim *a) {
    a->cur   = 0;
    a->timer = a->spd;
    a->done  = 0;
}

static void _anim_step(NpcAnim *a) {
    if (a->count == 0 || a->done) return;
    if (--a->timer <= 0) {
        a->timer = a->spd;
        if (a->loop) a->cur = (a->cur + 1) % a->count;
        else { if (a->cur < a->count - 1) a->cur++; else a->done = 1; }
    }
}

static void _anim_blit(SDL_Renderer *ren, NpcAnim *a,
                       int cx, int cy, int flip) {
    if (a->count == 0) return;
    SDL_Texture *t = a->frames[a->cur];
    if (!t) return;
    SDL_Rect dst = { cx - NPC_W/2, cy - NPC_H/2, NPC_W, NPC_H };
    SDL_RenderCopyEx(ren, t, NULL, &dst, 0, NULL,
                     flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

static void _anim_free(NpcAnim *a) {
    for (int i = 0; i < a->count; i++)
        if (a->frames[i]) SDL_DestroyTexture(a->frames[i]);
    a->count = 0;
}

/* ════════════════════════════════════════
 * MATH HELPERS
 * ════════════════════════════════════════ */
static float _dist(float ax, float ay, float bx, float by) {
    float dx = bx-ax, dy = by-ay;
    return sqrtf(dx*dx + dy*dy);
}
static float _clamp(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}
static void _pick_dest(Npc *n) {
    n->dest_x = (float)(60  + rand() % (NPC_WORLD_W - 120));
    n->dest_y = (float)(60  + rand() % (NPC_WORLD_H - 120));
    n->facing = (n->dest_x > n->x) ? 1 : -1;
}
static void _switch_slot(Npc *n, NpcAnimSlot s) {
    if (n->slot == s) return;
    n->slot = s;
    _anim_reset(&n->anim[s]);
}

/* ════════════════════════════════════════
 * PLATFORM COLLISION
 * ════════════════════════════════════════ */
static int _plat_resolve(float *x, float *y, float *vy,
                         int w, int h,
                         SDL_Rect *plats, int n) {
    int ground = 0;
    for (int i = 0; i < n; i++) {
        SDL_Rect *p = &plats[i];
        if (*x+w <= p->x || *x >= p->x+p->w) continue;
        if (*y+h <= p->y || *y >= p->y+p->h) continue;
        if (*y+h - *vy <= p->y+1) {
            *y = (float)(p->y - h);
            *vy = 0;
            ground = 1;
        }
    }
    return ground;
}

/* ════════════════════════════════════════
 * PUBLIC: BOUNDING BOX HIT TEST
 * ════════════════════════════════════════ */
int npc_bbox_hit(float ax, float ay, int aw, int ah,
                 float bx, float by, int bw, int bh) {
    return !(ax+aw <= bx || ax >= bx+bw ||
             ay+ah <= by || ay >= by+bh);
}

/* ════════════════════════════════════════
 * PUBLIC: NPC INIT / FREE / RESPAWN
 * ════════════════════════════════════════ */
void npc_init(Npc *n, float x, float y, SDL_Renderer *ren) {
    n->x = x; n->y = y;
    n->vx = 0; n->vy = 0;
    n->grounded   = 0;
    n->behaviour  = NPC_WANDER;
    n->facing     = 1;
    n->threat     = 0.0f;
    n->aggro_grow = 0.0f;
    n->aggro_r    = NPC_AGGRO_MIN;
    n->move_spd   = NPC_SPEED_HUNT;
    n->hp         = NPC_HP_MAX;
    n->hit_cd     = 0;
    n->stun       = 0;
    n->flash      = 0;
    n->atk_cd     = 0;
    n->heavy_cd   = 0;
    n->dead       = 0;
    n->death_hold = 0;
    n->slot       = NANIM_WALK;
    _pick_dest(n);

    /* Walk right */
    _load_seq(&n->anim[NANIM_WALK],        ren, "right",             8, 5, 1);
    /* Walk rage — separate rage sprites */
    _load_seq(&n->anim[NANIM_WALK_RAGE],   ren, "right",             8, 3, 1);
    {
        /* Override walk rage with actual rage frames */
        _anim_free(&n->anim[NANIM_WALK_RAGE]);
        _load_seq(&n->anim[NANIM_WALK_RAGE], ren, "right", 8, 3, 1);
        /* Try loading rage-specific frames if they exist */
        NpcAnim tmp; tmp.count = 0; tmp.cur = 0;
        tmp.timer = 3; tmp.spd = 3; tmp.loop = 1; tmp.done = 0;
        for (int i = 0; i < 8 && i < NPC_MAX_FRAMES; i++) {
            char p[256];
            snprintf(p, sizeof(p), "assets/thanos/right%drage.png", i+1);
            SDL_Surface *s = IMG_Load(p);
            if (!s) break;
            tmp.frames[tmp.count++] = SDL_CreateTextureFromSurface(ren, s);
            SDL_FreeSurface(s);
        }
        if (tmp.count > 0) {
            _anim_free(&n->anim[NANIM_WALK_RAGE]);
            n->anim[NANIM_WALK_RAGE] = tmp;
        }
    }

    /* Punch: rightpunchattack1..4 */
    _load_seq(&n->anim[NANIM_PUNCH],        ren, "rightpunchattack",  4, 4, 0);
    /* Heavy charge: heavyattackcharge1..4 */
    _load_seq(&n->anim[NANIM_HEAVY_CHARGE], ren, "heavyattackcharge", 4, 5, 0);
    /* Heavy hit: heavyattackda1 (single frame) */
    {
        const char *p[] = { "assets/thanos/heavyattackda1.png" };
        _load_paths(&n->anim[NANIM_HEAVY_HIT], ren, p, 1, 12, 0);
    }
    /* Hit: righthit1..3 */
    _load_seq(&n->anim[NANIM_HIT],           ren, "righthit",         3, 4, 0);
    /* Death: right1death, right2death, right3death */
    {
        const char *p[] = {
            "assets/thanos/right1death.png",
            "assets/thanos/right2death.png",
            "assets/thanos/right3death.png"
        };
        _load_paths(&n->anim[NANIM_DEATH], ren, p, 3, 18, 0);
    }
    /* Taunt: taunt1..3 */
    _load_seq(&n->anim[NANIM_TAUNT],         ren, "taunt",            3, 8, 0);
}

void npc_free(Npc *n) {
    for (int i = 0; i < NANIM_COUNT; i++) _anim_free(&n->anim[i]);
}

void npc_respawn(Npc *n, float px, float py) {
    (void)py;
    float ex; int tries = 0;
    do { ex = (float)(80 + rand() % (NPC_WORLD_W - 160)); tries++; }
    while (fabsf(ex - px) < 200.0f && tries < 20);

    n->x = ex; n->y = 480.0f;
    n->vx = 0; n->vy = 0;
    n->grounded  = 0;
    n->behaviour = NPC_WANDER;
    n->threat    = 0.0f;
    n->aggro_r   = NPC_AGGRO_MIN + (NPC_AGGRO_MAX - NPC_AGGRO_MIN) * n->aggro_grow;
    n->move_spd  = NPC_SPEED_HUNT + (NPC_SPEED_MAX - NPC_SPEED_HUNT) * n->aggro_grow;
    n->hp        = NPC_HP_MAX;
    n->hit_cd    = 0; n->stun = 0; n->flash = 0;
    n->atk_cd    = 0; n->heavy_cd = 0;
    n->dead      = 0; n->death_hold = 0;
    _switch_slot(n, NANIM_WALK);
    _pick_dest(n);
}

void npc_hurt(Npc *n) {
    if (n->hit_cd > 0 || n->dead) return;
    n->hp--;
    n->hit_cd = NPC_HIT_CD;
    n->flash  = 6;
    n->stun   = NPC_STUN_LEN;
    n->vy     = -7.0f;
    _switch_slot(n, NANIM_HIT);
    if (n->hp <= 0) {
        n->hp = 0; n->dead = 1;
        n->death_hold = -1;   /* -1 = still playing anim */
        _switch_slot(n, NANIM_DEATH);
    }
}

/* ════════════════════════════════════════
 * PUBLIC: NPC TICK (AI + PHYSICS + ANIM)
 * ════════════════════════════════════════ */
void npc_tick(Npc *n, float px, float py,
              SDL_Rect *plats, int nplat, NpcCoins *coins) {

    /* ── death sequence ── */
    if (n->dead) {
        if (n->death_hold == -1) {
            _anim_step(&n->anim[NANIM_DEATH]);
            if (n->anim[NANIM_DEATH].done) {
                if (coins)
                    coins_burst(coins,
                                n->x + NPC_W/2.0f,
                                n->y + NPC_H/2.0f,
                                2 + rand() % 3);
                n->death_hold = NPC_DEATH_HOLD;
            }
        } else if (n->death_hold > 0) {
            n->death_hold--;
        }
        /* death_hold == 0 → caller should respawn */
        return;
    }

    float cx  = n->x + NPC_W/2.0f,  cy  = n->y + NPC_H/2.0f;
    float pcx = px   + 14.0f,        pcy = py   + 18.0f;
    float d   = _dist(cx, cy, pcx, pcy);

    /* ── cooldown timers ── */
    if (n->hit_cd  > 0) n->hit_cd--;
    if (n->flash   > 0) n->flash--;
    if (n->atk_cd  > 0) n->atk_cd--;
    if (n->heavy_cd> 0) n->heavy_cd--;
    if (n->stun    > 0) { n->stun--; n->vx *= 0.82f; n->vy *= 0.82f; }

    /* ── facing ── */
    n->facing = (pcx < cx) ? -1 : 1;

    /* ── behaviour state machine ── */
    if (d <= n->aggro_r && n->stun == 0) {
        if (n->behaviour == NPC_WANDER)
            n->aggro_grow = _clamp(n->aggro_grow + NPC_AGGRO_GROW, 0.0f, 1.0f);
        n->behaviour = NPC_HUNT;
        n->threat    = 1.0f;
    } else {
        n->behaviour = NPC_WANDER;
        n->threat    = 0.0f;
    }
    n->aggro_r  = NPC_AGGRO_MIN + (NPC_AGGRO_MAX - NPC_AGGRO_MIN) * n->aggro_grow;
    n->move_spd = NPC_SPEED_HUNT + (NPC_SPEED_MAX - NPC_SPEED_HUNT) * n->aggro_grow;

    /* ── movement & animation selection ── */
    if (n->stun > 0) {
        _switch_slot(n, NANIM_HIT);
    } else if (n->behaviour == NPC_HUNT) {
        if (d < 70.0f) {
            /* close combat */
            n->vx = 0; n->vy = 0;
            if (n->aggro_grow > 0.6f && n->heavy_cd == 0 && n->atk_cd == 0) {
                _switch_slot(n, NANIM_HEAVY_CHARGE);
                n->heavy_cd = 30; n->atk_cd = 8;
            } else if (n->atk_cd == 0) {
                _switch_slot(n, NANIM_PUNCH);
                n->atk_cd = 16;
            }
        } else {
            /* chase */
            float dx = pcx-cx, dy = pcy-cy;
            float len = _dist(0, 0, dx, dy);
            if (len > 0.1f) {
                n->vx = (dx/len) * n->move_spd;
                n->vy = (dy/len) * n->move_spd;
            }
            _switch_slot(n, n->threat > 0.5f ? NANIM_WALK_RAGE : NANIM_WALK);
        }
    } else {
        /* wander toward random destination */
        float dx = n->dest_x - cx, dy = n->dest_y - cy;
        float dist = _dist(0, 0, dx, dy);
        if (dist < 10.0f) {
            _pick_dest(n);
            _switch_slot(n, rand()%100 < 15 ? NANIM_TAUNT : NANIM_WALK);
        } else {
            n->vx = (dx/dist) * NPC_SPEED_WANDER;
            n->vy = (dy/dist) * NPC_SPEED_WANDER;
            if (!(n->slot == NANIM_TAUNT && !n->anim[NANIM_TAUNT].done))
                _switch_slot(n, NANIM_WALK);
        }
    }

    /* heavy charge → heavy hit transition */
    if (n->slot == NANIM_HEAVY_CHARGE && n->anim[NANIM_HEAVY_CHARGE].done)
        _switch_slot(n, NANIM_HEAVY_HIT);
    /* hit anim → walk transition */
    if (n->slot == NANIM_HIT && n->anim[NANIM_HIT].done && n->stun == 0)
        _switch_slot(n, NANIM_WALK);

    /* ── physics ── */
    n->vy += 0.55f;
    n->x  += n->vx;
    n->y  += n->vy;
    n->grounded = 0;

    if (plats && nplat > 0)
        n->grounded = _plat_resolve(&n->x, &n->y, &n->vy,
                                    NPC_W, NPC_H, plats, nplat);

    if (n->x < 0)                   { n->x = 0;                            n->vx =  fabsf(n->vx); }
    if (n->x > NPC_WORLD_W - NPC_W) { n->x = (float)(NPC_WORLD_W - NPC_W); n->vx = -fabsf(n->vx); }
    if (n->y < 0)                   { n->y = 0;                            n->vy =  fabsf(n->vy); }
    if (n->y > NPC_WORLD_H - NPC_H) { n->y = (float)(NPC_WORLD_H - NPC_H); n->vy = 0; n->grounded = 1; }

    /* ── advance current animation ── */
    _anim_step(&n->anim[n->slot]);
}

/* ════════════════════════════════════════
 * PUBLIC: NPC DRAW
 * ════════════════════════════════════════ */
static void _rect(SDL_Renderer *r, int x, int y, int w, int h,
                  Uint8 R, Uint8 G, Uint8 B) {
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    SDL_RenderFillRect(r, &(SDL_Rect){x, y, w, h});
}
static void _ring(SDL_Renderer *r, int cx, int cy, int rad,
                  Uint8 R, Uint8 G, Uint8 B, Uint8 A) {
    SDL_SetRenderDrawColor(r, R, G, B, A);
    int x = rad, y = 0, err = 1 - rad;
    while (x >= y) {
        SDL_RenderDrawPoint(r, cx+x, cy+y); SDL_RenderDrawPoint(r, cx-x, cy+y);
        SDL_RenderDrawPoint(r, cx+x, cy-y); SDL_RenderDrawPoint(r, cx-x, cy-y);
        SDL_RenderDrawPoint(r, cx+y, cy+x); SDL_RenderDrawPoint(r, cx-y, cy+x);
        SDL_RenderDrawPoint(r, cx+y, cy-x); SDL_RenderDrawPoint(r, cx-y, cy-x);
        y++;
        if (err < 0) err += 2*y+1;
        else { x--; err += 2*(y-x+1); }
    }
}

void npc_draw(SDL_Renderer *ren, const Npc *n) {
    int x  = (int)n->x,     y  = (int)n->y;
    int cx = x + NPC_W/2,   cy = y + NPC_H/2;
    float g = n->aggro_grow, th = n->threat;

    /* dead: freeze on last death frame */
    if (n->dead) {
        NpcAnim *da = (NpcAnim*)&n->anim[NANIM_DEATH];
        if (da->count > 0) {
            int sv = da->cur;
            da->cur = da->count - 1;
            _anim_blit(ren, da, cx, cy, n->facing < 0);
            da->cur = sv;
        }
        return;
    }

    /* aggro ring */
    Uint8 ra = th > 0.5f ? (Uint8)(80+g*80) : (Uint8)(25+g*40);
    _ring(ren, cx, cy, (int)n->aggro_r,
          (Uint8)(80+g*175), (Uint8)(180-g*160), (Uint8)(80-g*60), ra);

    /* sprite — blink on hit */
    if (!(n->flash > 0 && n->flash % 2 == 0)) {
        if (n->anim[n->slot].count > 0) {
            _anim_blit(ren, (NpcAnim*)&n->anim[n->slot], cx, cy, n->facing < 0);
        } else {
            /* fallback block if sprites missing */
            if (th > 0.5f) _rect(ren, x, y, NPC_W, NPC_H, 210, 30, 30);
            else _rect(ren, x, y, NPC_W, NPC_H,
                       (Uint8)(110+g*30), (Uint8)(110-g*30), (Uint8)(125-g*40));
            SDL_SetRenderDrawColor(ren, (Uint8)(55+g*65), 0, 0, 255);
            SDL_RenderDrawRect(ren, &(SDL_Rect){x, y, NPC_W, NPC_H});
        }
    }

    /* HP bar */
    int bw = NPC_W+10, bx = x-5, by = y-18, bh = 7;
    int filled = n->hp > 0 ? (bw * n->hp) / NPC_HP_MAX : 0;
    SDL_SetRenderDrawColor(ren, 30, 30, 30, 220);
    SDL_RenderFillRect(ren, &(SDL_Rect){bx, by, bw, bh});
    SDL_SetRenderDrawColor(ren,
        (Uint8)(255 - n->hp*40), (Uint8)(n->hp*50), 20, 255);
    SDL_RenderFillRect(ren, &(SDL_Rect){bx, by, filled, bh});
    SDL_SetRenderDrawColor(ren, 180, 180, 180, 200);
    SDL_RenderDrawRect(ren, &(SDL_Rect){bx, by, bw, bh});

    /* rage bar */
    if (g > 0.0f) {
        SDL_SetRenderDrawColor(ren, 40, 40, 40, 200);
        SDL_RenderFillRect(ren, &(SDL_Rect){x, y-10, NPC_W, 4});
        SDL_SetRenderDrawColor(ren,
            (Uint8)(80+g*175), (Uint8)(180-g*170), 20, 255);
        SDL_RenderFillRect(ren, &(SDL_Rect){x, y-10, (int)(NPC_W*g), 4});
    }
}

/* ════════════════════════════════════════
 * PUBLIC: COIN SYSTEM
 * ════════════════════════════════════════ */
void coins_init(NpcCoins *c, SDL_Renderer *ren) {
    for (int i = 0; i < NPC_MAX_COINS; i++) c->pool[i].alive = 0;
    c->active  = 0;
    c->gathered = 0;
    _load_seq(&c->spin, ren, "coin", 4, 5, 1);
}

void coins_free(NpcCoins *c) { _anim_free(&c->spin); }

void coins_burst(NpcCoins *c, float cx, float cy, int n) {
    for (int i = 0; i < n; i++) {
        float angle = ((float)rand() / RAND_MAX) * 6.2832f;
        float dist  = 20.0f + ((float)rand() / RAND_MAX) * 50.0f;
        float nx = _clamp(cx + cosf(angle)*dist, 4.0f, (float)(NPC_WORLD_W - NPC_COIN_W - 4));
        float ny = _clamp(cy + sinf(angle)*dist, 4.0f, (float)(NPC_WORLD_H - NPC_COIN_H - 4));
        for (int j = 0; j < NPC_MAX_COINS; j++) {
            if (!c->pool[j].alive) {
                c->pool[j].x = nx;
                c->pool[j].y = ny;
                c->pool[j].alive = 1;
                c->active++;
                break;
            }
        }
    }
}

int coins_tick(NpcCoins *c, float px, float py, int pw, int ph) {
    _anim_step(&c->spin);
    int got = 0;
    for (int i = 0; i < NPC_MAX_COINS; i++) {
        if (!c->pool[i].alive) continue;
        if (npc_bbox_hit(px, py, pw, ph,
                         c->pool[i].x, c->pool[i].y,
                         NPC_COIN_W, NPC_COIN_H)) {
            c->pool[i].alive = 0;
            c->active--;
            c->gathered++;
            got++;
        }
    }
    return got;
}

void coins_draw(SDL_Renderer *ren, NpcCoins *c) {
    _anim_step(&c->spin);   /* advance even when tick not called */
    for (int i = 0; i < NPC_MAX_COINS; i++) {
        if (!c->pool[i].alive) continue;
        int cx = (int)c->pool[i].x + NPC_COIN_W/2;
        int cy = (int)c->pool[i].y + NPC_COIN_H/2;
        if (c->spin.count > 0) {
            SDL_Texture *t = c->spin.frames[c->spin.cur];
            if (t) {
                SDL_Rect dst = { cx-NPC_COIN_W/2, cy-NPC_COIN_H/2,
                                 NPC_COIN_W, NPC_COIN_H };
                SDL_RenderCopy(ren, t, NULL, &dst);
                continue;
            }
        }
        /* fallback gold square */
        SDL_SetRenderDrawColor(ren, 255, 215, 0, 255);
        SDL_RenderFillRect(ren, &(SDL_Rect){cx-NPC_COIN_W/2, cy-NPC_COIN_H/2,
                                            NPC_COIN_W, NPC_COIN_H});
    }
}
