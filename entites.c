#include "entites.h"
#include "utils.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>


void draw_filled_circle(SDL_Renderer *r, int cx, int cy, int rad,
                        Uint8 R, Uint8 G, Uint8 B, Uint8 A)
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, R, G, B, A);
    int rad2 = rad * rad;
    int dy;
    for (dy = -rad; dy <= rad; dy++) {
        int dx_max = (int)sqrtf((float)(rad2 - dy * dy));
        SDL_RenderDrawLine(r, cx - dx_max, cy + dy, cx + dx_max, cy + dy);
    }
}

int rect_overlap(const SDL_Rect *a, const SDL_Rect *b)
{
    return !(a->x + a->w <= b->x ||
             a->x         >= b->x + b->w ||
             a->y + a->h <= b->y ||
             a->y         >= b->y + b->h);
}

int circle_rect_overlap(float cx, float cy, int rad, const SDL_Rect *r)
{
    float closest_x = (float)((cx < r->x) ? r->x : (cx > r->x + r->w ? r->x + r->w : cx));
    float closest_y = (float)((cy < r->y) ? r->y : (cy > r->y + r->h ? r->y + r->h : cy));
    float dx = cx - closest_x;
    float dy = cy - closest_y;
    return (dx * dx + dy * dy) <= (float)(rad * rad);
}


void entites_init(EntiteSet *e)
{
    memset(e, 0, sizeof(*e));
}

void entites_clear(EntiteSet *e)
{

    SDL_Texture *otex[4];
    SDL_Texture *etex   = e->enemi_tex;
    SDL_Texture *e2tex  = e->enemi2_tex;
    SDL_Texture *lokitex = e->loki_tex;
    int i;
    for (i = 0; i < 4; i++) otex[i] = e->obstacle_tex[i];

    memset(e, 0, sizeof(*e));

    for (i = 0; i < 4; i++) e->obstacle_tex[i] = otex[i];
    e->enemi_tex  = etex;
    e->enemi2_tex = e2tex;
    e->loki_tex   = lokitex;
}

void entites_charger_textures(EntiteSet *e, SDL_Renderer *renderer)
{
    e->obstacle_tex[0] = charger_image(renderer, "assets/obstacle/obstacle1.png");
    e->obstacle_tex[1] = charger_image(renderer, "assets/obstacle/obstacle2.png");
    e->obstacle_tex[2] = charger_image(renderer, "assets/obstacle/obstacle3.png");
    e->obstacle_tex[3] = charger_image(renderer, "assets/obstacle/obstacle4.png");
    e->enemi_tex  = charger_image(renderer, "assets/enemy sprite sheet/enemi.png");
    e->enemi2_tex = charger_image(renderer, "assets/enemy sprite sheet/enemi2.png");
    e->loki_tex   = charger_image(renderer, "assets/enemy sprite sheet/loki.png");
}

void entites_liberer_textures(EntiteSet *e)
{
    int i;
    for (i = 0; i < 4; i++) {
        if (e->obstacle_tex[i]) {
            SDL_DestroyTexture(e->obstacle_tex[i]);
            e->obstacle_tex[i] = NULL;
        }
    }
    if (e->enemi_tex)  { SDL_DestroyTexture(e->enemi_tex);  e->enemi_tex  = NULL; }
    if (e->enemi2_tex) { SDL_DestroyTexture(e->enemi2_tex); e->enemi2_tex = NULL; }
    if (e->loki_tex)   { SDL_DestroyTexture(e->loki_tex);   e->loki_tex   = NULL; }
}


int entites_ajouter_coin(EntiteSet *e, float x, float y)
{
    int i;
    for (i = 0; i < MAX_COINS; i++) {
        if (!e->coins[i].actif) {
            e->coins[i].x     = x;
            e->coins[i].y     = y;
            e->coins[i].actif = 1;
            e->coins[i].spin  = i * 7;
            return 1;
        }
    }
    return 0;
}

int entites_ajouter_obstacle(EntiteSet *e, int x, int y, int w, int h, int tex_idx)
{
    int i;
    for (i = 0; i < MAX_OBSTACLES; i++) {
        if (!e->obstacles[i].actif) {
            e->obstacles[i].rect.x      = x;
            e->obstacles[i].rect.y      = y;
            e->obstacles[i].rect.w      = w;
            e->obstacles[i].rect.h      = h;
            e->obstacles[i].actif       = 1;
            e->obstacles[i].texture_idx = tex_idx % 4;
            return 1;
        }
    }
    return 0;
}

int entites_ajouter_trap(EntiteSet *e, int x, int y, int w, int h)
{
    int i;
    for (i = 0; i < MAX_TRAPS; i++) {
        if (!e->traps[i].actif) {
            e->traps[i].rect.x = x;
            e->traps[i].rect.y = y;
            e->traps[i].rect.w = w;
            e->traps[i].rect.h = h;
            e->traps[i].actif  = 1;
            return 1;
        }
    }
    return 0;
}

int entites_ajouter_enemy(EntiteSet *e, EnemyType t, float x, float y)
{
    int i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (!e->enemies[i].actif) {
            e->enemies[i].x       = x;
            e->enemies[i].y       = y;
            e->enemies[i].vx      = 0;
            e->enemies[i].vy      = 0;
            e->enemies[i].type    = t;
            e->enemies[i].actif   = 1;
            e->enemies[i].mort    = 0;
            e->enemies[i].flash   = 0;
            e->enemies[i].facing  = 1;
            e->enemies[i].anim_frame  = 0;
            e->enemies[i].anim_timer  = 0;
            e->enemies[i].attack_timer = 0;

            if (t == ENEMY_MELEE) {
                e->enemies[i].rayon  = ENEMY_RADIUS_DEFAULT;
                e->enemies[i].hp     = ENEMY_HP_MELEE;
                e->enemies[i].hp_max = ENEMY_HP_MELEE;
            } else if (t == ENEMY_RANGED) {
                e->enemies[i].rayon  = ENEMY_RADIUS_DEFAULT;
                e->enemies[i].hp     = ENEMY_HP_RANGED;
                e->enemies[i].hp_max = ENEMY_HP_RANGED;
            } else {
                e->enemies[i].rayon  = LOKI_RADIUS;
                e->enemies[i].hp     = ENEMY_HP_LOKI;
                e->enemies[i].hp_max = ENEMY_HP_LOKI;
            }
            e->enemies[i].fire_cd = 60 + (rand() % 60);
            return 1;
        }
    }
    return 0;
}

int entites_compter_enemies_vivants(EntiteSet *e)
{
    int i, n = 0;
    for (i = 0; i < MAX_ENEMIES; i++)
        if (e->enemies[i].actif && !e->enemies[i].mort) n++;
    return n;
}

int entites_loki_present(EntiteSet *e)
{
    int i;
    for (i = 0; i < MAX_ENEMIES; i++)
        if (e->enemies[i].actif && e->enemies[i].type == ENEMY_LOKI) return 1;
    return 0;
}

int entites_loki_mort(EntiteSet *e)
{
    int i;
    for (i = 0; i < MAX_ENEMIES; i++)
        if (e->enemies[i].actif && e->enemies[i].type == ENEMY_LOKI && e->enemies[i].mort)
            return 1;
    return 0;
}


int projectile_ajouter(EntiteSet *e, float x, float y, float vx, int from_enemy)
{
    int i;
    for (i = 0; i < MAX_PROJECTILES; i++) {
        if (!e->projectiles[i].actif) {
            e->projectiles[i].x          = x;
            e->projectiles[i].y          = y;
            e->projectiles[i].vx         = vx;
            e->projectiles[i].actif      = 1;
            e->projectiles[i].from_enemy = from_enemy;
            return 1;
        }
    }
    return 0;
}


void entites_update(EntiteSet *e, Personnage *joueur)
{
    float pcx = joueur->x + SPRITE_RENDER_W / 2.0f;
    float pcy = joueur->y + SPRITE_RENDER_H / 2.0f;
    int i;


    for (i = 0; i < MAX_COINS; i++) {
        if (!e->coins[i].actif) continue;
        e->coins[i].spin++;
        float dx = e->coins[i].x - pcx;
        float dy = e->coins[i].y - pcy;
        float r2 = dx * dx + dy * dy;
        float ramassage = COIN_RADIUS + (SPRITE_RENDER_W / 3.0f);
        if (r2 < ramassage * ramassage) {
            e->coins[i].actif = 0;
            ajouterScore(joueur, COIN_VALEUR);
        }
    }


    for (i = 0; i < MAX_TRAPS; i++) {
        if (!e->traps[i].actif) continue;
        SDL_Rect joueur_box = { (int)joueur->x + 30, (int)joueur->y + 30,
                                SPRITE_RENDER_W - 60, SPRITE_RENDER_H - 30 };
        if (rect_overlap(&joueur_box, &e->traps[i].rect)) {
            if (joueur->attaque_cooldown == 0) {
                perdreVie(joueur);
                joueur->attaque_cooldown = TRAP_DAMAGE_CD;
            }
        }
    }


    for (i = 0; i < MAX_ENEMIES; i++) {
        Enemy *en = &e->enemies[i];
        if (!en->actif || en->mort) continue;
        if (en->flash > 0) en->flash--;
        if (en->attack_timer > 0) en->attack_timer--;


        en->anim_timer++;
        if (en->anim_timer >= ANIM_TICKS_PER_FRAME) {
            en->anim_timer = 0;
            en->anim_frame = (en->anim_frame + 1) % 4;
        }

        float dx = pcx - en->x;
        float dy = pcy - en->y;
        float dist = sqrtf(dx * dx + dy * dy);
        en->facing = (dx >= 0) ? 1 : -1;

        float ground_y = (float)(SOL_Y + SPRITE_RENDER_H / 2);

        if (en->type == ENEMY_MELEE) {

            if (dist > 1.0f) {
                en->vx = (dx / dist) * ENEMY_SPEED_MELEE;
            }
            en->x += en->vx;
            if (dist < 80.0f) en->attack_timer = 16;

            if (en->y < ground_y) {
                en->y += 2.0f;
                if (en->y > ground_y) en->y = ground_y;
            } else if (en->y > ground_y) {
                en->y -= 1.0f;
            }
        } else if (en->type == ENEMY_RANGED) {

            float ideal = 280.0f;
            if (dist > ideal + 30) {
                en->vx = (dx / dist) * ENEMY_SPEED_RANGED;
            } else if (dist < ideal - 30) {
                en->vx = -(dx / dist) * ENEMY_SPEED_RANGED;
            } else {
                en->vx = 0;
            }
            en->x += en->vx;

            float py_target = (float)(SOL_Y + 40);
            if (en->y < py_target) en->y += 2.0f;


            if (en->fire_cd > 0) en->fire_cd--;
            if (en->fire_cd == 0) {
                float vx = (dx > 0) ? 5.0f : -5.0f;
                projectile_ajouter(e, en->x, en->y, vx, 1);
                en->fire_cd = 90 + (rand() % 60);
                en->attack_timer = 16;
            }
        } else {

            if (dist > 200.0f) {
                en->vx = (dx / (dist+0.001f)) * ENEMY_SPEED_LOKI;
            } else if (dist < 80.0f) {
                en->vx = 0.0f;
            } else {
                en->vx = (dx / dist) * (ENEMY_SPEED_LOKI * 0.5f);
            }
            en->x += en->vx;

            if (en->y < ground_y - 10) {
                en->y += 2.0f;
            } else if (en->y > ground_y - 10) {
                en->y -= 1.0f;
            }


            if (en->fire_cd > 0) en->fire_cd--;
            if (en->fire_cd == 0 && dist > 60.0f) {
                float vx = (dx > 0) ? 6.0f : -6.0f;
                projectile_ajouter(e, en->x, en->y - 10, vx, 1);
                en->fire_cd = 70 + (rand() % 50);
                en->attack_timer = 20;
            }
            if (dist < 90.0f) en->attack_timer = 16;
        }


        if (en->x < (float)en->rayon) en->x = (float)en->rayon;



        SDL_Rect joueur_box = { (int)joueur->x + 30, (int)joueur->y + 30,
                                SPRITE_RENDER_W - 60, SPRITE_RENDER_H - 30 };
        if (circle_rect_overlap(en->x, en->y, en->rayon, &joueur_box)) {
            if (joueur->attaque_cooldown == 0) {
                perdreVie(joueur);
                joueur->attaque_cooldown = ENEMY_TOUCH_DAMAGE_CD;
            }
        }
    }


    for (i = 0; i < MAX_PROJECTILES; i++) {
        Projectile *p = &e->projectiles[i];
        if (!p->actif) continue;
        p->x += p->vx;
        if (p->x < -20 || p->x > 4096) {
            p->actif = 0;
            continue;
        }
        if (p->from_enemy) {
            SDL_Rect joueur_box = { (int)joueur->x + 30, (int)joueur->y + 30,
                                    SPRITE_RENDER_W - 60, SPRITE_RENDER_H - 30 };
            SDL_Rect proj_box = { (int)p->x - 6, (int)p->y - 6, 12, 12 };
            if (rect_overlap(&joueur_box, &proj_box)) {
                p->actif = 0;
                if (joueur->attaque_cooldown == 0) {
                    perdreVie(joueur);
                    joueur->attaque_cooldown = ENEMY_TOUCH_DAMAGE_CD;
                }
            }
        }
    }
}


void entites_collision_obstacles(EntiteSet *e, Personnage *joueur, float old_x, float old_y)
{
    int i;


    int feet_now  = (int)(joueur->y + SPRITE_RENDER_H);
    int feet_prev = (int)(old_y    + SPRITE_RENDER_H);
    int center_x  = (int)(joueur->x + SPRITE_RENDER_W / 2.0f);


    int landed = 0;
    for (i = 0; i < MAX_OBSTACLES; i++) {
        if (!e->obstacles[i].actif) continue;
        SDL_Rect o = e->obstacles[i].rect;
        SDL_Rect oi = { o.x - 4, o.y, o.w + 8, o.h };

        if (center_x >= oi.x && center_x <= oi.x + oi.w) {

            if (joueur->vy >= 0.0f && feet_prev <= o.y + 2 && feet_now >= o.y - 1) {
                joueur->y      = (float)(o.y - SPRITE_RENDER_H);
                joueur->vy     = 0.0f;
                joueur->au_sol = 1;
                landed = 1;
                break;
            }
        }
    }


    SDL_Rect body = { (int)joueur->x + 30, (int)joueur->y + 20,
                      SPRITE_RENDER_W - 60, SPRITE_RENDER_H - 30 };

    for (i = 0; i < MAX_OBSTACLES; i++) {
        if (!e->obstacles[i].actif) continue;
        SDL_Rect o = e->obstacles[i].rect;

        SDL_Rect ow = { o.x - 4, o.y + 4, o.w + 8, o.h - 6 };

        if (rect_overlap(&body, &ow)) {

            int feet_test = (int)joueur->y + SPRITE_RENDER_H;
            if (landed && feet_test <= o.y + 1) continue;


            int head_now  = (int)joueur->y + 20;
            int head_prev = (int)old_y     + 20;
            if (joueur->vy < 0.0f &&
                head_prev >= o.y + o.h - 2 &&
                head_now  <= o.y + o.h + 4) {
                joueur->y  = (float)(o.y + o.h - 20);
                joueur->vy = 1.0f;
                continue;
            }


            joueur->x = old_x;
            body.x    = (int)joueur->x + 30;
        }
    }
}


void entites_player_attack(EntiteSet *e, Personnage *joueur)
{
    if (joueur->etat != ETAT_ATTACK) return;
    if (joueur->attaque_timer <= 0)  return;

    int hit_x, hit_y, hit_w, hit_h;
    hit_y = (int)joueur->y;
    hit_h = HITBOX_H;

    if (joueur->direction == DIR_DROITE) {
        hit_x = (int)joueur->x + SPRITE_RENDER_W;
        hit_w = HITBOX_REACH;
    } else {
        hit_x = (int)joueur->x - HITBOX_REACH;
        hit_w = HITBOX_REACH;
    }
    SDL_Rect hitbox = { hit_x, hit_y, hit_w, hit_h };

    int i;
    for (i = 0; i < MAX_ENEMIES; i++) {
        Enemy *en = &e->enemies[i];
        if (!en->actif || en->mort) continue;
        if (en->flash > 0) continue;
        if (circle_rect_overlap(en->x, en->y, en->rayon, &hitbox)) {
            en->hp--;
            en->flash = (en->type == ENEMY_LOKI) ? LOKI_HIT_FLASH : ENEMY_HIT_FLASH;
            if (en->hp <= 0) {
                en->mort = 1;
                ajouterScore(joueur, (en->type == ENEMY_LOKI) ? 200 : 25);
            }
        }
    }
}


void entites_draw(GameState *gs, EntiteSet *e)
{
    SDL_Renderer *r = gs->renderer;
    int cam = gs->camera_x;
    int i;


    for (i = 0; i < MAX_OBSTACLES; i++) {
        if (!e->obstacles[i].actif) continue;
        SDL_Rect dst = e->obstacles[i].rect;
        dst.x -= cam;
        SDL_Texture *tex = e->obstacle_tex[e->obstacles[i].texture_idx];

        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 80);
        SDL_RenderFillRect(r, &(SDL_Rect){ dst.x + 4, dst.y + 4, dst.w, dst.h });

        if (tex) {
            SDL_RenderCopy(r, tex, NULL, &dst);
        } else {
            SDL_SetRenderDrawColor(r, 90, 60, 35, 255);
            SDL_RenderFillRect(r, &dst);
        }

        SDL_SetRenderDrawColor(r, 30, 20, 10, 255);
        SDL_RenderDrawRect(r, &dst);
        SDL_RenderDrawRect(r, &(SDL_Rect){ dst.x + 1, dst.y + 1, dst.w - 2, dst.h - 2 });
    }


    for (i = 0; i < MAX_TRAPS; i++) {
        if (!e->traps[i].actif) continue;
        SDL_Rect dst = e->traps[i].rect;
        dst.x -= cam;
        SDL_SetRenderDrawColor(r, 200, 40, 40, 255);
        SDL_RenderFillRect(r, &dst);

        SDL_SetRenderDrawColor(r, 255, 220, 80, 255);
        int sx = dst.x;
        int sy = dst.y;
        int sw = dst.w;
        int spike_w = 8;
        int n_spikes = sw / spike_w;
        int k;
        for (k = 0; k < n_spikes; k++) {
            int px = sx + k * spike_w;
            SDL_RenderDrawLine(r, px, sy, px + spike_w / 2, sy - 6);
            SDL_RenderDrawLine(r, px + spike_w / 2, sy - 6, px + spike_w, sy);
        }
    }


    for (i = 0; i < MAX_COINS; i++) {
        if (!e->coins[i].actif) continue;
        int phase = (e->coins[i].spin / 8) % 4;
        int squash = (phase == 1 || phase == 3) ? COIN_RADIUS / 2 : COIN_RADIUS;
        int cx = (int)e->coins[i].x - cam;
        int cy = (int)e->coins[i].y;
        draw_filled_circle(r, cx, cy, COIN_RADIUS, 0, 0, 0, 80);
        draw_filled_circle(r, cx, cy, squash, 255, 215, 30, 255);
        draw_filled_circle(r, cx - 3, cy - 3, squash / 3, 255, 240, 130, 255);
    }


    for (i = 0; i < MAX_ENEMIES; i++) {
        Enemy *en = &e->enemies[i];
        if (!en->actif || en->mort) continue;

        int cx = (int)en->x - cam;
        int cy = (int)en->y;
        int rad = en->rayon;

        SDL_Rect dst = { cx - rad, cy - (int)(rad * 1.4f),
                         rad * 2, (int)(rad * 2.6f) };

        SDL_Texture *tex = NULL;
        SDL_Rect src;

        if (en->type == ENEMY_MELEE) {
            tex = e->enemi_tex;
            int row = (en->attack_timer > 0) ? (ENEMI_ROWS > 1 ? 1 : 0) : 0;
            int col = en->anim_frame % ENEMI_COLS;
            src.x = col * ENEMI_FRAME_W;
            src.y = row * ENEMI_FRAME_H;
            src.w = ENEMI_FRAME_W;
            src.h = ENEMI_FRAME_H;
        } else if (en->type == ENEMY_RANGED) {
            tex = e->enemi2_tex;
            int row = (en->attack_timer > 0) ? (ENEMI2_ROWS > 1 ? 1 : 0) : 0;
            int col = en->anim_frame % ENEMI2_COLS;
            src.x = col * ENEMI2_FRAME_W;
            src.y = row * ENEMI2_FRAME_H;
            src.w = ENEMI2_FRAME_W;
            src.h = ENEMI2_FRAME_H;
        } else {
            tex = e->loki_tex;
            int row = (en->attack_timer > 0) ? (LOKI_ROWS > 1 ? 1 : 0) : 0;
            int col = en->anim_frame % LOKI_COLS;
            src.x = col * LOKI_FRAME_W;
            src.y = row * LOKI_FRAME_H;
            src.w = LOKI_FRAME_W;
            src.h = LOKI_FRAME_H;
        }

        SDL_RendererFlip flip = (en->facing < 0) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

        if (tex) {
            if (en->flash > 0 && en->flash % 2 == 0) {
                SDL_SetTextureAlphaMod(tex, 200);
            } else if (en->flash > 0) {
                SDL_SetTextureColorMod(tex, 255, 120, 120);
            }
            SDL_RenderCopyEx(r, tex, &src, &dst, 0, NULL, flip);
            SDL_SetTextureColorMod(tex, 255, 255, 255);
            SDL_SetTextureAlphaMod(tex, 255);
        } else {

            Uint8 R, G, B;
            if (en->flash > 0) { R = 255; G = 255; B = 255; }
            else if (en->type == ENEMY_MELEE)  { R = 180; G = 50;  B = 50;  }
            else if (en->type == ENEMY_RANGED) { R = 60;  G = 80;  B = 200; }
            else                                { R = 150; G = 50;  B = 200; }
            draw_filled_circle(r, cx, cy, rad, R, G, B, 255);
        }


        if (en->hp_max > 1) {
            int bw = (en->type == ENEMY_LOKI) ? 80 : 40;
            int bh = (en->type == ENEMY_LOKI) ? 6 : 4;
            int bx = cx - bw / 2;
            int by = cy - rad - 12;
            SDL_SetRenderDrawColor(r, 40, 40, 40, 220);
            SDL_RenderFillRect(r, &(SDL_Rect){ bx, by, bw, bh });
            SDL_SetRenderDrawColor(r, 220, 50, 80, 255);
            int fw = (bw * en->hp) / en->hp_max;
            SDL_RenderFillRect(r, &(SDL_Rect){ bx, by, fw, bh });

            if (en->type == ENEMY_LOKI) {
                SDL_Color violet = {220, 100, 220, 255};
                afficher_texte(r, gs->font, "LOKI", violet, bx, by - 18);
            }
        }
    }


    for (i = 0; i < MAX_PROJECTILES; i++) {
        Projectile *p = &e->projectiles[i];
        if (!p->actif) continue;
        int cx = (int)p->x - cam;
        int cy = (int)p->y;
        draw_filled_circle(r, cx, cy, 6, 255, 200, 100, 255);
        draw_filled_circle(r, cx, cy, 3, 255, 255, 220, 255);
    }
}
