#include "thanos_bridge.h"
#include <math.h>


extern int npc_bbox_hit(float ax, float ay, int aw, int ah,
                        float bx, float by, int bw, int bh);


void thanos_init(Thanos *t, SDL_Renderer *ren)
{
    npc_init(&t->npc, 600.0f, (float)(SOL_Y - 20), ren);
    t->initialise              = 1;
    t->player_was_attacking    = 0;
    t->hit_done_this_attack    = 0;
}

void thanos_free(Thanos *t)
{
    if (!t->initialise) return;
    npc_free(&t->npc);
    t->initialise = 0;
}

void thanos_reset(Thanos *t, float x, float y)
{
    t->npc.x          = x;
    t->npc.y          = y;
    t->npc.vx         = 0;
    t->npc.vy         = 0;
    t->npc.hp         = NPC_HP_MAX;
    t->npc.dead       = 0;
    t->npc.death_hold = 0;
    t->npc.hit_cd     = 0;
    t->npc.stun       = 0;
    t->npc.flash      = 0;
    t->npc.atk_cd     = 0;
    t->npc.heavy_cd   = 0;
    t->npc.aggro_grow = 0.0f;
    t->npc.aggro_r    = NPC_AGGRO_MIN;
    t->npc.behaviour  = NPC_WANDER;
    t->npc.threat     = 0.0f;
    t->npc.slot       = NANIM_WALK;
    t->hit_done_this_attack = 0;
    t->player_was_attacking = 0;
}


void thanos_update(Thanos *t, Personnage *joueur)
{
    if (!t->initialise) return;


    float pcx = joueur->x + SPRITE_RENDER_W / 2.0f;
    float pcy = joueur->y + SPRITE_RENDER_H / 2.0f;


    npc_tick(&t->npc, pcx - 14.0f, pcy - 18.0f, NULL, 0, NULL);


    if (joueur->etat == ETAT_ATTACK && joueur->attaque_timer > 0) {
        if (!t->player_was_attacking) {
            int hit_x, hit_w, hit_y, hit_h;
            hit_y = (int)joueur->y;
            hit_h = HITBOX_H;
            if (joueur->direction == DIR_DROITE) {
                hit_x = (int)joueur->x + SPRITE_RENDER_W;
                hit_w = HITBOX_REACH;
            } else {
                hit_x = (int)joueur->x - HITBOX_REACH;
                hit_w = HITBOX_REACH;
            }

            if (npc_bbox_hit((float)hit_x, (float)hit_y, hit_w, hit_h,
                             t->npc.x, t->npc.y, NPC_W, NPC_H)) {
                npc_hurt(&t->npc);
                ajouterScore(joueur, 50);
            }
            t->player_was_attacking = 1;
        }
    } else {
        t->player_was_attacking = 0;
    }


    if (!t->npc.dead && joueur->attaque_cooldown == 0) {
        int does_hit = 0;
        int dmg_window = 0;

        if (t->npc.slot == NANIM_PUNCH) {
            int cur = t->npc.anim[NANIM_PUNCH].cur;
            if (cur == 2 || cur == 3) dmg_window = 1;
        } else if (t->npc.slot == NANIM_HEAVY_HIT) {
            dmg_window = 1;
        }

        if (dmg_window) {
            float dx = pcx - (t->npc.x + NPC_W / 2.0f);
            float dy = pcy - (t->npc.y + NPC_H / 2.0f);
            float dist = sqrtf(dx * dx + dy * dy);
            float reach = (t->npc.slot == NANIM_HEAVY_HIT) ? 110.0f : 80.0f;
            if (dist < reach) does_hit = 1;
        }

        if (does_hit && !t->hit_done_this_attack) {
            perdreVie(joueur);
            joueur->attaque_cooldown = (t->npc.slot == NANIM_HEAVY_HIT)
                ? THANOS_HEAVY_DAMAGE_CD
                : THANOS_TOUCH_DAMAGE_CD;
            t->hit_done_this_attack = 1;
        }
        if (t->npc.slot != NANIM_PUNCH && t->npc.slot != NANIM_HEAVY_HIT) {
            t->hit_done_this_attack = 0;
        }
    }
}

void thanos_draw(GameState *gs, Thanos *t)
{
    if (!t->initialise) return;
    npc_draw(gs->renderer, &t->npc);
}

int thanos_est_mort(Thanos *t)
{
    return t->initialise && t->npc.dead && t->npc.death_hold > 0;
}
