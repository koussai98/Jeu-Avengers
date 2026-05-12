#include "quiz.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


const QuizQuestion QUIZ_BASE[QUIZ_NB_CATEGORIES * QUIZ_NB_PAR_CAT] = {

    { "Avengers", "Quel est le vrai nom d'Iron Man ?",
      { "Steve Rogers", "Tony Stark", "Bruce Banner", "Peter Parker" }, 1 },
    { "Avengers", "Quelle est l'arme de Thor ?",
      { "Bouclier", "Mjolnir", "Arc", "Repulsor" }, 1 },
    { "Avengers", "Qui est le grand mechant de Infinity War ?",
      { "Loki", "Ultron", "Thanos", "Hela" }, 2 },
    { "Avengers", "De quelle couleur est Hulk ?",
      { "Bleu", "Rouge", "Vert", "Violet" }, 2 },


    { "Science", "Quel est le symbole chimique de l'eau ?",
      { "O2", "CO2", "H2O", "NaCl" }, 2 },
    { "Science", "Combien de planetes dans le systeme solaire ?",
      { "7", "8", "9", "10" }, 1 },
    { "Science", "Quelle vitesse approche la lumiere ?",
      { "300 km/s", "300 000 km/s", "3 000 km/s", "30 000 km/s" }, 1 },
    { "Science", "Quel organe pompe le sang ?",
      { "Foie", "Cerveau", "Coeur", "Poumon" }, 2 },


    { "Math",     "Combien font 7 x 8 ?",
      { "54", "56", "58", "64" }, 1 },
    { "Math",     "Quelle est la racine carree de 81 ?",
      { "7", "8", "9", "11" }, 2 },
    { "Math",     "Combien de degres dans un cercle ?",
      { "90", "180", "270", "360" }, 3 },
    { "Math",     "Quel nombre est premier ?",
      { "9", "15", "17", "21" }, 2 }
};


void quiz_init(GameState *gs, Quiz *q)
{
    int i;
    int total = QUIZ_NB_CATEGORIES * QUIZ_NB_PAR_CAT;

    static int seed_done = 0;
    if (!seed_done) {
        srand((unsigned)time(NULL));
        seed_done = 1;
    }

    q->question_idx  = rand() % total;
    q->choix_survol  = -1;
    q->resultat      = QUIZ_EN_COURS;
    q->fin_timer     = 0;


    int btn_w = 540;
    int btn_h = 50;
    int gap   = 14;
    int total_h = QUIZ_NB_OPTIONS * btn_h + (QUIZ_NB_OPTIONS - 1) * gap;
    int start_y = SCREEN_H / 2 - total_h / 2 + 30;
    int x       = (SCREEN_W - btn_w) / 2;

    for (i = 0; i < QUIZ_NB_OPTIONS; i++) {
        q->rects_options[i].x = x;
        q->rects_options[i].y = start_y + i * (btn_h + gap);
        q->rects_options[i].w = btn_w;
        q->rects_options[i].h = btn_h;
    }

    init_bouton(gs, &q->btn_retour, NULL, NULL,
                40, SCREEN_H - 70, 140, 45, "Retour");
}

void quiz_input(GameState *gs, Quiz *q)
{
    SDL_Event ev;
    (void)q;
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
        }
    }
}

void quiz_update(GameState *gs, Quiz *q)
{
    int i;
    q->choix_survol = -1;


    if (q->resultat != QUIZ_EN_COURS) {
        if (q->fin_timer == 0) q->fin_timer = SDL_GetTicks() + 1500;
        return;
    }

    if (gs->motion) {
        for (i = 0; i < QUIZ_NB_OPTIONS; i++) {
            int mx = gs->mouse_x, my = gs->mouse_y;
            SDL_Rect *rc = &q->rects_options[i];
            if (mx >= rc->x && mx <= rc->x + rc->w &&
                my >= rc->y && my <= rc->y + rc->h) {
                q->choix_survol = i;
            }
        }
        q->btn_retour.actif = souris_sur_bouton(&q->btn_retour,
                                                  gs->mouse_x, gs->mouse_y);
    }

    if (gs->click) {
        if (souris_sur_bouton(&q->btn_retour, gs->mouse_x, gs->mouse_y)) {

            q->resultat = QUIZ_ECHOUE;
            q->fin_timer = SDL_GetTicks() + 800;
            return;
        }

        for (i = 0; i < QUIZ_NB_OPTIONS; i++) {
            SDL_Rect *rc = &q->rects_options[i];
            if (gs->mouse_x >= rc->x && gs->mouse_x <= rc->x + rc->w &&
                gs->mouse_y >= rc->y && gs->mouse_y <= rc->y + rc->h) {
                int correct = QUIZ_BASE[q->question_idx].bonne_reponse;
                q->resultat  = (i == correct) ? QUIZ_REUSSI : QUIZ_ECHOUE;
                q->fin_timer = SDL_GetTicks() + 1500;
                if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
                return;
            }
        }
    }
}

void quiz_draw(GameState *gs, Quiz *q)
{
    SDL_Renderer *r = gs->renderer;
    int i;
    const QuizQuestion *qu = &QUIZ_BASE[q->question_idx];


    SDL_SetRenderDrawColor(r, 12, 18, 35, 255);
    SDL_RenderClear(r);


    SDL_Color jaune = {255, 220,  50, 255};
    SDL_Color blanc = {235, 235, 245, 255};
    SDL_Color gris  = {170, 170, 190, 255};

    afficher_texte_centre(r, gs->font_large ? gs->font_large : gs->font,
                          "QUIZ - SECONDE CHANCE", jaune, 0, SCREEN_W, 60);

    {
        char cat_buf[64];
        snprintf(cat_buf, sizeof(cat_buf), "Categorie : %s", qu->categorie);
        afficher_texte_centre(r, gs->font, cat_buf, gris, 0, SCREEN_W, 110);
    }

    afficher_texte_centre(r, gs->font_large ? gs->font_large : gs->font,
                          qu->question, blanc, 0, SCREEN_W, 160);


    for (i = 0; i < QUIZ_NB_OPTIONS; i++) {
        SDL_Rect *rc = &q->rects_options[i];
        Uint8 br = 50, bg = 50, bb = 80;

        if (q->resultat != QUIZ_EN_COURS && i == qu->bonne_reponse) {
            br = 50;  bg = 200; bb = 80;
        } else if (q->resultat == QUIZ_ECHOUE && i == q->choix_survol) {
            br = 200; bg = 50;  bb = 60;
        } else if (i == q->choix_survol) {
            br = 90;  bg = 90;  bb = 130;
        }

        SDL_SetRenderDrawColor(r, br, bg, bb, 255);
        SDL_RenderFillRect(r, rc);
        SDL_SetRenderDrawColor(r, 200, 200, 220, 255);
        SDL_RenderDrawRect(r, rc);

        char buf[160];
        snprintf(buf, sizeof(buf), "%c. %s", 'A' + i, qu->options[i]);
        afficher_texte(r, gs->font, buf, blanc, rc->x + 16, rc->y + 14);
    }


    if (q->resultat != QUIZ_EN_COURS) {
        const char *txt = (q->resultat == QUIZ_REUSSI)
                          ? "BONNE REPONSE !"
                          : "MAUVAISE REPONSE";
        SDL_Color col = (q->resultat == QUIZ_REUSSI)
                        ? (SDL_Color){80, 220, 100, 255}
                        : (SDL_Color){220, 80, 80, 255};
        afficher_texte_centre(r, gs->font_large ? gs->font_large : gs->font,
                              txt, col, 0, SCREEN_W, SCREEN_H - 130);
    }

    afficher_bouton(r, gs->font, &q->btn_retour);
}

void quiz_free(Quiz *q)
{
    liberer_bouton(&q->btn_retour);
}

int quiz_est_termine(Quiz *q)
{
    if (q->resultat == QUIZ_EN_COURS) return 0;
    if (q->fin_timer == 0) return 0;
    return SDL_GetTicks() >= q->fin_timer;
}

int quiz_resultat(Quiz *q)
{
    return (int)q->resultat;
}
