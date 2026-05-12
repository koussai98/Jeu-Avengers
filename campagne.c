#include "campagne.h"
#include "champion_select.h"
#include <string.h>
#include <stdio.h>


void afficher_message_camp(Campagne *c, const char *txt, int duree_frames)
{
    strncpy(c->message, txt, sizeof(c->message) - 1);
    c->message[sizeof(c->message) - 1] = '\0';
    c->message_timer = duree_frames;
}


void setup_level_1a(Campagne *c)
{
    entites_clear(&c->entites);
    c->world_w      = CAMPAGNE_WORLD_W;
    c->camera_x     = 0;
    c->loki_spawned = 0;

    int sol_top = SOL_Y + SPRITE_RENDER_H;


    entites_ajouter_obstacle(&c->entites,  300, sol_top - 75,  60, 75, 0);
    entites_ajouter_obstacle(&c->entites,  580, sol_top - 90,  70, 90, 1);
    entites_ajouter_obstacle(&c->entites,  900, sol_top - 60,  55, 60, 2);
    entites_ajouter_obstacle(&c->entites, 1180, sol_top - 80,  60, 80, 3);
    entites_ajouter_obstacle(&c->entites, 1420, sol_top - 55,  50, 55, 0);


    entites_ajouter_coin(&c->entites,  220.0f, (float)(sol_top - 40));
    entites_ajouter_coin(&c->entites,  330.0f, (float)(sol_top - 100));
    entites_ajouter_coin(&c->entites,  500.0f, (float)(sol_top - 70));
    entites_ajouter_coin(&c->entites,  610.0f, (float)(sol_top - 130));
    entites_ajouter_coin(&c->entites,  860.0f, (float)(sol_top - 80));
    entites_ajouter_coin(&c->entites,  990.0f, (float)(sol_top - 50));
    entites_ajouter_coin(&c->entites, 1120.0f, (float)(sol_top - 70));
    entites_ajouter_coin(&c->entites, 1300.0f, (float)(sol_top - 100));
    entites_ajouter_coin(&c->entites, 1450.0f, (float)(sol_top - 60));
    entites_ajouter_coin(&c->entites, 1530.0f, (float)(sol_top - 40));

    afficher_message_camp(c, "NIVEAU 1-A : ENTRAINEMENT - Atteins la fin du niveau", 200);
}

void setup_level_1b(Campagne *c)
{
    entites_clear(&c->entites);
    c->world_w      = CAMPAGNE_WORLD_W;
    c->camera_x     = 0;
    c->loki_spawned = 0;

    int sol_top = SOL_Y + SPRITE_RENDER_H;

    entites_ajouter_obstacle(&c->entites,  450, sol_top - 70,  60, 70, 1);
    entites_ajouter_obstacle(&c->entites,  850, sol_top - 60,  55, 60, 2);
    entites_ajouter_obstacle(&c->entites, 1200, sol_top - 80,  65, 80, 3);


    entites_ajouter_coin(&c->entites,  280.0f, (float)(sol_top - 50));
    entites_ajouter_coin(&c->entites,  500.0f, (float)(sol_top - 100));
    entites_ajouter_coin(&c->entites,  780.0f, (float)(sol_top - 90));
    entites_ajouter_coin(&c->entites, 1050.0f, (float)(sol_top - 70));
    entites_ajouter_coin(&c->entites, 1300.0f, (float)(sol_top - 80));

    c->spawn_timer = 60;
    c->wave_index  = 0;

    afficher_message_camp(c, "NIVEAU 1-B : ENNEMIS - puis LOKI a la fin !", 180);
}

void setup_level_2(Campagne *c)
{
    entites_clear(&c->entites);
    c->world_w      = CAMPAGNE_WORLD_W;
    c->camera_x     = 0;
    c->loki_spawned = 0;

    int sol_top = SOL_Y + SPRITE_RENDER_H;


    entites_ajouter_obstacle(&c->entites,  320, sol_top - 60,  50, 60, 0);
    entites_ajouter_obstacle(&c->entites,  680, sol_top - 90,  60, 90, 2);
    entites_ajouter_obstacle(&c->entites, 1050, sol_top - 60,  55, 60, 3);
    entites_ajouter_obstacle(&c->entites, 1370, sol_top - 80,  60, 80, 1);


    entites_ajouter_coin(&c->entites,  200.0f, (float)(sol_top - 50));
    entites_ajouter_coin(&c->entites,  730.0f, (float)(sol_top - 130));
    entites_ajouter_coin(&c->entites, 1100.0f, (float)(sol_top - 80));
    entites_ajouter_coin(&c->entites, 1500.0f, (float)(sol_top - 60));


    entites_ajouter_enemy(&c->entites, ENEMY_MELEE,  600.0f,  (float)(SOL_Y + SPRITE_RENDER_H / 2));
    entites_ajouter_enemy(&c->entites, ENEMY_MELEE,  1100.0f, (float)(SOL_Y + SPRITE_RENDER_H / 2));
    entites_ajouter_enemy(&c->entites, ENEMY_RANGED, 850.0f,  (float)(SOL_Y + 30));

    c->spawn_timer = 240;
    c->wave_index  = 0;

    afficher_message_camp(c, "NIVEAU 2 - Survis pour affronter THANOS", 180);
}

void setup_boss_thanos(GameState *gs, Campagne *c)
{
    entites_clear(&c->entites);
    c->world_w  = SCREEN_W;
    c->camera_x = 0;

    if (c->thanos.initialise) {
        thanos_reset(&c->thanos, 600.0f, (float)(SOL_Y - 20));
    } else {
        thanos_init(&c->thanos, gs->renderer);
    }

    afficher_message_camp(c, "THANOS APPARAIT !", 120);
}


void update_spawn_1b(Campagne *c)
{
    int vivants = entites_compter_enemies_vivants(&c->entites);


    if (c->wave_index >= 5 && !c->loki_spawned) {
        if (vivants > 0) return;

        entites_ajouter_enemy(&c->entites, ENEMY_LOKI,
                              (float)(c->world_w - 200),
                              (float)(SOL_Y + SPRITE_RENDER_H / 2 - 10));
        c->loki_spawned = 1;
        afficher_message_camp(c, "LOKI - Le semi-boss arrive !", 150);
        return;
    }


    if (c->wave_index >= 5) return;

    if (c->spawn_timer > 0) {
        c->spawn_timer--;
        return;
    }

    if (vivants > 0) {
        c->spawn_timer = 30;
        return;
    }


    int from_left = (c->wave_index % 2 == 0) ? 0 : 1;
    float spawn_x = from_left ? 80.0f : (float)(c->world_w - 80);

    EnemyType t = (c->wave_index >= 3) ? ENEMY_RANGED : ENEMY_MELEE;
    float spawn_y = (t == ENEMY_RANGED) ? (float)(SOL_Y + 30)
                                        : (float)(SOL_Y + SPRITE_RENDER_H / 2);

    entites_ajouter_enemy(&c->entites, t, spawn_x, spawn_y);
    c->wave_index++;
    c->spawn_timer = 180;
}

void update_spawn_level2(Campagne *c)
{
    int vivants = entites_compter_enemies_vivants(&c->entites);
    if (c->wave_index >= 4) return;

    if (c->spawn_timer > 0) {
        c->spawn_timer--;
        return;
    }
    if (vivants > 2) {
        c->spawn_timer = 30;
        return;
    }

    float spawn_x = (c->wave_index % 2 == 0) ? 80.0f : (float)(c->world_w - 80);
    EnemyType t = (c->wave_index % 2 == 0) ? ENEMY_RANGED : ENEMY_MELEE;
    float spawn_y = (t == ENEMY_RANGED) ? (float)(SOL_Y + 30)
                                        : (float)(SOL_Y + SPRITE_RENDER_H / 2);
    entites_ajouter_enemy(&c->entites, t, spawn_x, spawn_y);

    c->wave_index++;
    c->spawn_timer = 200;
}


void campagne_init(GameState *gs, Campagne *c)
{
    memset(c, 0, sizeof(*c));
    initialiserJoueur1(gs, &c->joueur);
    c->joueur.vies        = 5;
    c->joueur.world_w_max = CAMPAGNE_WORLD_W;
    c->etape                 = ETAPE_LEVEL_1A;
    c->etape_courante_init   = -1;
    c->world_w               = CAMPAGNE_WORLD_W;
    c->camera_x              = 0;
    c->attente_finale        = 0;
    c->loki_spawned          = 0;


    entites_charger_textures(&c->entites, gs->renderer);


    c->bg_textures[0] = charger_image(gs->renderer, "assets/background/bg1.png");
    c->bg_textures[1] = charger_image(gs->renderer, "assets/background/bg2.png");
    c->bg_textures[2] = charger_image(gs->renderer, "assets/background/bg3.png");

    c->portrait_champion = NULL;
}

void campagne_liberer(Campagne *c)
{
    int i;
    libererJoueur(&c->joueur);
    if (c->thanos.initialise) thanos_free(&c->thanos);
    entites_liberer_textures(&c->entites);
    for (i = 0; i < 3; i++) {
        if (c->bg_textures[i]) {
            SDL_DestroyTexture(c->bg_textures[i]);
            c->bg_textures[i] = NULL;
        }
    }
    if (c->portrait_champion) {
        SDL_DestroyTexture(c->portrait_champion);
        c->portrait_champion = NULL;
    }
}

void campagne_revive(Campagne *c)
{
    c->joueur.x                 = 60.0f;
    c->joueur.y                 = (float)SOL_Y;
    c->joueur.vy                = 0.0f;
    c->joueur.au_sol            = 1;
    c->joueur.est_vivant        = 1;
    c->joueur.vies              = 5;
    c->joueur.attaque_cooldown  = 0;
    c->joueur.attaque_timer     = 0;
    c->joueur.touche_gauche     = 0;
    c->joueur.touche_droite     = 0;
    c->joueur.touche_courir     = 0;
    c->joueur.etat              = ETAT_IDLE;
}

void campagne_reset(GameState *gs, Campagne *c)
{

    int i;
    SDL_Texture *bgs[3], *port;
    SDL_Texture *otex[4];
    SDL_Texture *etex   = c->entites.enemi_tex;
    SDL_Texture *e2tex  = c->entites.enemi2_tex;
    SDL_Texture *lokitex= c->entites.loki_tex;

    for (i = 0; i < 3; i++) bgs[i] = c->bg_textures[i];
    port = c->portrait_champion;
    for (i = 0; i < 4; i++) otex[i] = c->entites.obstacle_tex[i];

    libererJoueur(&c->joueur);
    if (c->thanos.initialise) thanos_free(&c->thanos);

    memset(c, 0, sizeof(*c));


    for (i = 0; i < 3; i++) c->bg_textures[i] = bgs[i];
    c->portrait_champion = port;
    for (i = 0; i < 4; i++) c->entites.obstacle_tex[i] = otex[i];
    c->entites.enemi_tex  = etex;
    c->entites.enemi2_tex = e2tex;
    c->entites.loki_tex   = lokitex;

    initialiserJoueur1(gs, &c->joueur);
    c->joueur.vies         = 5;
    c->joueur.world_w_max  = CAMPAGNE_WORLD_W;
    c->etape               = ETAPE_LEVEL_1A;
    c->etape_courante_init = -1;
    c->world_w             = CAMPAGNE_WORLD_W;
    c->camera_x            = 0;


    joueur_set_champion(gs, &c->joueur, (int)gs->champion_choisi);


    if (c->portrait_champion) {
        SDL_DestroyTexture(c->portrait_champion);
    }
    c->portrait_champion = charger_image(gs->renderer,
                                          champion_portrait_path(gs->champion_choisi));
}


void campagne_input(GameState *gs, Campagne *c)
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) {
            gs->continuer = 0;
        } else if (ev.type == SDL_KEYDOWN) {
            SDL_Keycode k = ev.key.keysym.sym;

            if (k == SDLK_ESCAPE) {
                gs->backg.niveau = NIVEAU_MENU;
                return;
            }

            if (c->etape == ETAPE_VICTOIRE || c->etape == ETAPE_GAMEOVER) {
                if (k == SDLK_RETURN || k == SDLK_SPACE) {
                    gs->backg.niveau = NIVEAU_MENU;
                }
                return;
            }

            gererToucheAppuyee(&c->joueur, k);
        } else if (ev.type == SDL_KEYUP) {
            gererToucheRelachee(&c->joueur, ev.key.keysym.sym);
        }
    }
}


void campagne_update(GameState *gs, Campagne *c)
{

    if (c->etape_courante_init != (int)c->etape) {
        switch (c->etape) {
            case ETAPE_LEVEL_1A: setup_level_1a(c);     break;
            case ETAPE_LEVEL_1B: setup_level_1b(c);     break;
            case ETAPE_LEVEL_2:  setup_level_2(c);      break;
            case ETAPE_BOSS:     setup_boss_thanos(gs, c); break;
            case ETAPE_VICTOIRE:
                afficher_message_camp(c, "VICTOIRE !", 99999);
                c->victory_start_ms = SDL_GetTicks();
                break;
            case ETAPE_GAMEOVER: afficher_message_camp(c, "GAME OVER", 99999);   break;
        }
        c->etape_courante_init = (int)c->etape;
    }

    if (c->message_timer > 0) c->message_timer--;

    if (c->etape == ETAPE_VICTOIRE) {

        if (c->victory_start_ms != 0 &&
            SDL_GetTicks() - c->victory_start_ms >= 3000) {
            gs->backg.niveau = NIVEAU_MENU;
        }
        return;
    }

    if (c->etape == ETAPE_GAMEOVER) return;


    {
        float old_x = c->joueur.x;
        float old_y = c->joueur.y;
        c->joueur.world_w_max = c->world_w;
        mettreAJourJoueur(&c->joueur);


        if (c->joueur.y < (float)SOL_Y) {
            c->joueur.au_sol = 0;
        }

        entites_collision_obstacles(&c->entites, &c->joueur, old_x, old_y);
    }


    {
        int target_cam = (int)(c->joueur.x + SPRITE_RENDER_W / 2.0f) - SCREEN_W / 2;
        if (target_cam < 0) target_cam = 0;
        if (target_cam > c->world_w - SCREEN_W) target_cam = c->world_w - SCREEN_W;
        c->camera_x = target_cam;
    }


    entites_update(&c->entites, &c->joueur);
    entites_player_attack(&c->entites, &c->joueur);

    if (c->etape == ETAPE_BOSS) {
        thanos_update(&c->thanos, &c->joueur);
    }


    if (c->joueur.vies <= 0 && !c->joueur.est_vivant) {
        gs->death_count++;
        gs->puzzle_origin = NIVEAU_CAMPAGNE;

        gs->backg.niveau = NIVEAU_DEATH_CHOICE;
        return;
    }


    switch (c->etape) {
        case ETAPE_LEVEL_1A:
            if (c->joueur.x >= (float)(c->world_w - SPRITE_RENDER_W - 5)) {
                c->joueur.x = 60.0f;
                c->etape    = ETAPE_LEVEL_1B;
            }
            break;

        case ETAPE_LEVEL_1B:
            update_spawn_1b(c);

            if (entites_loki_mort(&c->entites)) {
                c->joueur.x = 60.0f;
                c->etape    = ETAPE_LEVEL_2;
            }
            break;

        case ETAPE_LEVEL_2:
            update_spawn_level2(c);
            if (c->wave_index >= 4 && entites_compter_enemies_vivants(&c->entites) == 0) {
                c->etape = ETAPE_BOSS;
            }
            break;

        case ETAPE_BOSS:

            if (c->thanos.initialise && c->thanos.npc.dead) {
                c->attente_finale++;
                if (c->attente_finale >= 60) {
                    c->etape = ETAPE_VICTOIRE;
                }
            }
            break;

        default: break;
    }
}


void draw_minimap(GameState *gs, Campagne *c)
{
    SDL_Renderer *r = gs->renderer;
    int mm_x = SCREEN_W - MINIMAP_W - 12;
    int mm_y = 56;
    int i;


    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
    SDL_RenderFillRect(r, &(SDL_Rect){ mm_x - 2, mm_y - 2, MINIMAP_W + 4, MINIMAP_H + 4 });
    SDL_SetRenderDrawColor(r, 200, 200, 220, 200);
    SDL_RenderDrawRect(r, &(SDL_Rect){ mm_x - 2, mm_y - 2, MINIMAP_W + 4, MINIMAP_H + 4 });


    SDL_SetRenderDrawColor(r, 60, 80, 60, 220);
    SDL_RenderFillRect(r, &(SDL_Rect){ mm_x, mm_y, MINIMAP_W, MINIMAP_H });

    float sx = (float)MINIMAP_W / (float)c->world_w;
    float sy = (float)MINIMAP_H / (float)SCREEN_H;


    {
        int cam_w = (int)(SCREEN_W * sx);
        int cam_x = mm_x + (int)(c->camera_x * sx);
        SDL_SetRenderDrawColor(r, 200, 200, 200, 100);
        SDL_RenderFillRect(r, &(SDL_Rect){ cam_x, mm_y, cam_w, MINIMAP_H });
    }


    for (i = 0; i < MAX_ENEMIES; i++) {
        Enemy *en = &c->entites.enemies[i];
        if (!en->actif || en->mort) continue;
        int dx = mm_x + (int)(en->x * sx);
        int dy = mm_y + (int)(en->y * sy);
        Uint8 R = 220, G = 60, B = 60;
        if (en->type == ENEMY_RANGED) { R = 80;  G = 100; B = 220; }
        else if (en->type == ENEMY_LOKI) { R = 200; G = 80; B = 220; }
        SDL_SetRenderDrawColor(r, R, G, B, 255);
        SDL_RenderFillRect(r, &(SDL_Rect){ dx - 2, dy - 2, 5, 5 });
    }


    if (c->etape == ETAPE_BOSS && c->thanos.initialise && !c->thanos.npc.dead) {
        int dx = mm_x + (int)(c->thanos.npc.x * sx);
        int dy = mm_y + (int)(c->thanos.npc.y * sy);
        SDL_SetRenderDrawColor(r, 200, 50, 200, 255);
        SDL_RenderFillRect(r, &(SDL_Rect){ dx - 3, dy - 3, 7, 7 });
    }


    {
        int dx = mm_x + (int)((c->joueur.x + SPRITE_RENDER_W / 2.0f) * sx);
        int dy = mm_y + (int)((c->joueur.y + SPRITE_RENDER_H / 2.0f) * sy);
        SDL_SetRenderDrawColor(r, 50, 220, 80, 255);
        SDL_RenderFillRect(r, &(SDL_Rect){ dx - 3, dy - 3, 7, 7 });
    }


    {
        SDL_Color blanc = {220, 220, 220, 220};
        afficher_texte(r, gs->font, "Carte", blanc, mm_x, mm_y - 18);
    }
}


void draw_hud_campagne(GameState *gs, Campagne *c)
{
    SDL_Renderer *r = gs->renderer;
    SDL_Color blanc = {255, 255, 255, 255};
    SDL_Color jaune = {255, 220, 50, 255};
    SDL_Color rouge = {220,  60, 60, 255};


    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 150);
    SDL_RenderFillRect(r, &(SDL_Rect){ 0, 0, SCREEN_W, 50 });


    if (c->portrait_champion) {
        SDL_Rect pr = { 8, 6, 38, 38 };
        SDL_RenderCopy(r, c->portrait_champion, NULL, &pr);
    }


    int hx_start = 56;
    int i;
    for (i = 0; i < 5; i++) {
        SDL_Color col = (i < c->joueur.vies) ? rouge : (SDL_Color){60, 60, 60, 255};
        SDL_SetRenderDrawColor(r, col.r, col.g, col.b, 255);
        SDL_Rect hg = { hx_start + i * 26,     14, 9, 9 };
        SDL_Rect hd = { hx_start + i * 26 + 9, 14, 9, 9 };
        SDL_Rect ba = { hx_start + i * 26 + 2, 21, 14, 11 };
        SDL_RenderFillRect(r, &hg);
        SDL_RenderFillRect(r, &hd);
        SDL_RenderFillRect(r, &ba);
    }

    {
        char buf[64];
        snprintf(buf, sizeof(buf), "Score: %d", c->joueur.score);
        afficher_texte(r, gs->font, buf, jaune, SCREEN_W - 240, 16);
    }

    {
        const char *nom = "?";
        switch (c->etape) {
            case ETAPE_LEVEL_1A: nom = "Niv 1-A"; break;
            case ETAPE_LEVEL_1B: nom = "Niv 1-B"; break;
            case ETAPE_LEVEL_2:  nom = "Niv 2";   break;
            case ETAPE_BOSS:     nom = "BOSS";    break;
            default: break;
        }
        afficher_texte_centre(r, gs->font, nom, blanc, 200, 200, 16);
    }

    {
        SDL_Color gris = {200, 200, 200, 220};
        afficher_texte_centre(r, gs->font,
            "Q/D = bouger   Espace = saut   A = attaque   LShift = courir   Echap = menu",
            gris, 0, SCREEN_W, SCREEN_H - 22);
    }
}


void campagne_draw(GameState *gs, Campagne *c)
{
    SDL_Renderer *r = gs->renderer;


    gs->camera_x = c->camera_x;


    SDL_SetRenderDrawColor(r, 60, 130, 70, 255);
    SDL_RenderClear(r);


    {
        SDL_Texture *bg = NULL;
        if (c->etape == ETAPE_LEVEL_1A) bg = c->bg_textures[0];
        else if (c->etape == ETAPE_LEVEL_1B) bg = c->bg_textures[1];
        else if (c->etape == ETAPE_LEVEL_2 || c->etape == ETAPE_BOSS) bg = c->bg_textures[2];

        if (bg) {

            SDL_Rect src = { 0, 0, 0, 0 };
            SDL_QueryTexture(bg, NULL, NULL, &src.w, &src.h);


            float scale = (float)c->world_w / (float)src.w;
            int dst_w = c->world_w;
            int dst_h = (int)(src.h * scale);
            if (dst_h < SCREEN_H) dst_h = SCREEN_H;

            SDL_Rect dst = { -c->camera_x, 0, dst_w, dst_h };
            SDL_RenderCopy(r, bg, NULL, &dst);
        }
    }


    {
        int ground_y = SOL_Y + SPRITE_RENDER_H;
        SDL_SetRenderDrawColor(r, 80, 50, 20, 230);
        SDL_RenderFillRect(r, &(SDL_Rect){ 0, ground_y, SCREEN_W, SCREEN_H - ground_y });
        SDL_SetRenderDrawColor(r, 50, 30, 10, 255);
        SDL_RenderDrawLine(r, 0, ground_y, SCREEN_W, ground_y);
    }


    if (c->etape == ETAPE_LEVEL_1A) {
        int gx = c->world_w - 30 - c->camera_x;
        int gy = SOL_Y + SPRITE_RENDER_H;
        SDL_SetRenderDrawColor(r, 255, 235, 80, 255);
        SDL_Rect porte = { gx, gy - 80, 18, 80 };
        SDL_RenderFillRect(r, &porte);
        SDL_SetRenderDrawColor(r, 200, 170, 30, 255);
        SDL_RenderDrawRect(r, &porte);
    }


    entites_draw(gs, &c->entites);


    afficherJoueur(gs, &c->joueur);


    if (c->etape == ETAPE_BOSS) {

        float saved_x = c->thanos.npc.x;
        c->thanos.npc.x -= c->camera_x;
        thanos_draw(gs, &c->thanos);
        c->thanos.npc.x = saved_x;
    }


    gs->camera_x = 0;


    draw_hud_campagne(gs, c);


    draw_minimap(gs, c);


    if (c->message_timer > 0) {
        SDL_Color blanc = {255, 255, 255, 255};
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
        SDL_RenderFillRect(r, &(SDL_Rect){ 0, SCREEN_H / 2 - 30, SCREEN_W, 60 });
        afficher_texte_centre(r, gs->font_large ? gs->font_large : gs->font,
                              c->message, blanc, 0, SCREEN_W, SCREEN_H / 2 - 12);
    }


    if (c->etape == ETAPE_VICTOIRE) {

        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 200);
        SDL_RenderFillRect(r, &(SDL_Rect){ 0, 0, SCREEN_W, SCREEN_H });

        SDL_Color jaune = {255, 220, 50, 255};
        SDL_Color blanc = {255, 255, 255, 255};


        TTF_Font *big = gs->font_large ? gs->font_large : gs->font;
        afficher_texte_centre(r, big, "VICTOIRE",
                              jaune, 0, SCREEN_W, SCREEN_H / 2 - 60);
        afficher_texte_centre(r, big, "!!!",
                              jaune, 0, SCREEN_W, SCREEN_H / 2 - 10);

        char score_str[64];
        snprintf(score_str, sizeof(score_str), "Score final : %d", c->joueur.score);
        afficher_texte_centre(r, gs->font, score_str, blanc,
                              0, SCREEN_W, SCREEN_H / 2 + 60);

        afficher_texte_centre(r, gs->font,
                              "Retour automatique au menu...",
                              blanc, 0, SCREEN_W, SCREEN_H / 2 + 100);
    } else if (c->etape == ETAPE_GAMEOVER) {

        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
        SDL_RenderFillRect(r, &(SDL_Rect){ 0, 0, SCREEN_W, SCREEN_H });

        SDL_Color blanc = {255, 255, 255, 255};
        SDL_Color rouge = {220, 60, 60, 255};

        afficher_texte_centre(r, gs->font_large ? gs->font_large : gs->font,
                              "GAME OVER", rouge, 0, SCREEN_W, SCREEN_H / 2 - 60);

        char score_str[64];
        snprintf(score_str, sizeof(score_str), "Score final : %d", c->joueur.score);
        afficher_texte_centre(r, gs->font, score_str, blanc,
                              0, SCREEN_W, SCREEN_H / 2);

        afficher_texte_centre(r, gs->font,
                              "Appuie sur Entree ou Espace pour le menu",
                              blanc, 0, SCREEN_W, SCREEN_H / 2 + 40);
    }
}
