#include "champion_select.h"
#include <string.h>
#include <stdio.h>

const char *CHAMPION_NOMS[CHAMPION_COUNT] = {
    "THOR", "HULK", "IRON MAN", "CAPTAIN AMERICA"
};

const char *CHAMPION_PORTRAIT_PATHS[CHAMPION_COUNT] = {
    "assets/puzzle/thor_full.png",
    "assets/puzzle/hulk_full.png",
    "assets/puzzle/ironman_full.png",
    "assets/puzzle/capamerica_full.png"
};

const char *champion_nom(Champion c)
{
    if (c < 0 || c >= CHAMPION_COUNT) return "?";
    return CHAMPION_NOMS[c];
}

const char *champion_portrait_path(Champion c)
{
    if (c < 0 || c >= CHAMPION_COUNT) return CHAMPION_PORTRAIT_PATHS[0];
    return CHAMPION_PORTRAIT_PATHS[c];
}


void champion_select_init(GameState *gs, ChampionSelect *cs)
{
    int i;
    int card_w = 150;
    int card_h = 200;
    int gap    = 30;
    int total_w = CHAMPION_COUNT * card_w + (CHAMPION_COUNT - 1) * gap;
    int start_x = (SCREEN_W - total_w) / 2;
    int y       = 180;

    cs->hover      = -1;
    cs->choix_temp = (int)gs->champion_choisi;

    for (i = 0; i < CHAMPION_COUNT; i++) {
        cs->rects[i].x = start_x + i * (card_w + gap);
        cs->rects[i].y = y;
        cs->rects[i].w = card_w;
        cs->rects[i].h = card_h;

        cs->portraits[i] = charger_image(gs->renderer, CHAMPION_PORTRAIT_PATHS[i]);
        if (!cs->portraits[i]) {
            fprintf(stderr, "[CHAMPION] portrait introuvable: %s\n",
                    CHAMPION_PORTRAIT_PATHS[i]);
        }
    }

    init_bouton(gs, &cs->btn_retour,  NULL, NULL,
                40, SCREEN_H - 70, 140, 45, "Retour");
    init_bouton(gs, &cs->btn_valider, NULL, NULL,
                SCREEN_W - 200, SCREEN_H - 70, 160, 45, "Valider");
}

void champion_select_input(GameState *gs, ChampionSelect *cs)
{
    SDL_Event ev;
    (void)cs;
    gs->click  = 0;
    gs->motion = 0;

    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) {
            gs->continuer = 0;
        } else if (ev.type == SDL_MOUSEBUTTONDOWN &&
                   ev.button.button == SDL_BUTTON_LEFT) {
            gs->click   = 1;
            gs->mouse_x = ev.button.x;
            gs->mouse_y = ev.button.y;
        } else if (ev.type == SDL_MOUSEMOTION) {
            gs->motion  = 1;
            gs->mouse_x = ev.motion.x;
            gs->mouse_y = ev.motion.y;
        } else if (ev.type == SDL_KEYDOWN &&
                   ev.key.keysym.sym == SDLK_ESCAPE) {
            gs->backg.niveau = NIVEAU_MENU;
        }
    }
}

void champion_select_update(GameState *gs, ChampionSelect *cs)
{
    int i;
    cs->hover = -1;

    if (gs->motion || gs->click) {
        for (i = 0; i < CHAMPION_COUNT; i++) {
            int mx = gs->mouse_x, my = gs->mouse_y;
            if (mx >= cs->rects[i].x && mx <= cs->rects[i].x + cs->rects[i].w &&
                my >= cs->rects[i].y && my <= cs->rects[i].y + cs->rects[i].h) {
                cs->hover = i;
                if (gs->click) {
                    cs->choix_temp = i;
                    if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
                }
            }
        }

        cs->btn_retour.actif  = souris_sur_bouton(&cs->btn_retour,
                                                   gs->mouse_x, gs->mouse_y);
        cs->btn_valider.actif = souris_sur_bouton(&cs->btn_valider,
                                                   gs->mouse_x, gs->mouse_y);
    }

    if (gs->click) {
        if (souris_sur_bouton(&cs->btn_retour, gs->mouse_x, gs->mouse_y)) {
            gs->backg.niveau = NIVEAU_MENU;
            if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
        } else if (souris_sur_bouton(&cs->btn_valider, gs->mouse_x, gs->mouse_y)) {
            gs->champion_choisi = (Champion)cs->choix_temp;
            gs->backg.niveau    = NIVEAU_CAMPAGNE;
            if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
        }
    }
}

void champion_select_draw(GameState *gs, ChampionSelect *cs)
{
    SDL_Renderer *r = gs->renderer;
    int i;


    SDL_SetRenderDrawColor(r, 15, 15, 35, 255);
    SDL_RenderClear(r);


    {
        SDL_Color jaune = {255, 220, 50, 255};
        SDL_Color blanc = {220, 220, 230, 255};
        afficher_texte_centre(r, gs->font_large ? gs->font_large : gs->font,
                              "CHOISIS TON CHAMPION", jaune, 0, SCREEN_W, 60);
        afficher_texte_centre(r, gs->font,
                              "Clique sur un heros, puis VALIDER pour commencer",
                              blanc, 0, SCREEN_W, 110);
    }


    for (i = 0; i < CHAMPION_COUNT; i++) {
        SDL_Rect *rc = &cs->rects[i];

        if (i == cs->choix_temp) {
            SDL_SetRenderDrawColor(r, 255, 200, 30, 255);
            SDL_Rect halo = { rc->x - 6, rc->y - 6, rc->w + 12, rc->h + 12 };
            SDL_RenderFillRect(r, &halo);
        } else if (i == cs->hover) {
            SDL_SetRenderDrawColor(r, 100, 100, 130, 255);
            SDL_Rect halo = { rc->x - 4, rc->y - 4, rc->w + 8, rc->h + 8 };
            SDL_RenderFillRect(r, &halo);
        }


        SDL_SetRenderDrawColor(r, 30, 30, 50, 255);
        SDL_RenderFillRect(r, rc);

        if (cs->portraits[i]) {

            SDL_Rect img_dst = { rc->x + 8, rc->y + 8, rc->w - 16, rc->h - 50 };
            SDL_RenderCopy(r, cs->portraits[i], NULL, &img_dst);
        }


        SDL_Color blanc = {235, 235, 245, 255};
        afficher_texte_centre(r, gs->font, CHAMPION_NOMS[i],
                              blanc, rc->x, rc->w, rc->y + rc->h - 32);
    }


    afficher_bouton(r, gs->font, &cs->btn_retour);
    afficher_bouton(r, gs->font, &cs->btn_valider);
}

void champion_select_free(ChampionSelect *cs)
{
    int i;
    for (i = 0; i < CHAMPION_COUNT; i++) {
        if (cs->portraits[i]) {
            SDL_DestroyTexture(cs->portraits[i]);
            cs->portraits[i] = NULL;
        }
    }
    liberer_bouton(&cs->btn_retour);
    liberer_bouton(&cs->btn_valider);
}
