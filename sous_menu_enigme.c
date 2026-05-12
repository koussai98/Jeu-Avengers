#include "sous_menu_enigme.h"
#include <string.h>

void init_sous_menu_enigme(GameState *gs, SousMenuEnigme *sm)
{
    init_background(gs, &sm->backg,
                    "assets/images/background4.png",
                    NULL, NIVEAU_ENIGME);
    sm->etat = ENIGME_CHOIX;
    int cx = SCREEN_W / 2;
    init_bouton(gs, &sm->btn_quiz,   NULL, NULL, cx - 140, 280, 120, 50, "QUIZ");
    init_bouton(gs, &sm->btn_puzzle, NULL, NULL, cx +  20, 280, 120, 50, "Puzzle");
    init_bouton(gs, &sm->btn_a,      NULL, NULL, cx - 200, 420,  90, 45, "A");
    init_bouton(gs, &sm->btn_b,      NULL, NULL, cx -  45, 420,  90, 45, "B");
    init_bouton(gs, &sm->btn_c,      NULL, NULL, cx + 110, 420,  90, 45, "C");
}

void afficher_sous_menu_enigme(GameState *gs, SousMenuEnigme *sm)
{
    afficher_background(gs->renderer, &sm->backg);
    draw_rounded_rect(gs->renderer, SCREEN_W/2 - 240, 140, 480, 360, 20, 20, 60, 200);

    SDL_Color jaune = {255, 220, 50, 255};
    SDL_Color blanc = {255, 255, 255, 255};
    afficher_texte_centre(gs->renderer, gs->font_large, "ENIGME", jaune, 0, SCREEN_W, 70);

    if (sm->etat == ENIGME_CHOIX) {
        afficher_bouton(gs->renderer, gs->font, &sm->btn_quiz);
        afficher_bouton(gs->renderer, gs->font, &sm->btn_puzzle);
    } else {
        afficher_texte_centre(gs->renderer, gs->font_large, "QUIZ", jaune, 0, SCREEN_W, 170);
        draw_rounded_rect(gs->renderer, SCREEN_W/2 - 200, 230, 400, 60, 30, 60, 100, 220);
        afficher_texte_centre(gs->renderer, gs->font,
                              "Quelle est la capitale de la France ?",
                              blanc, SCREEN_W/2 - 200, 400, 248);
        afficher_bouton(gs->renderer, gs->font, &sm->btn_a);
        afficher_bouton(gs->renderer, gs->font, &sm->btn_b);
        afficher_bouton(gs->renderer, gs->font, &sm->btn_c);
    }
}

void input_sous_menu_enigme(GameState *gs)
{
    SDL_Event ev;
    gs->click = 0; gs->motion = 0; strcpy(gs->input, "");
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) gs->continuer = 0;
        else if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)
            strcpy(gs->input, "quitter");
        else if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
            gs->click = 1; gs->mouse_x = ev.button.x; gs->mouse_y = ev.button.y;
        }
        else if (ev.type == SDL_MOUSEMOTION) {
            gs->motion = 1; gs->mouse_x = ev.motion.x; gs->mouse_y = ev.motion.y;
        }
    }
}

static void hover(GameState *gs, Bouton *b) {
    int was = b->actif;
    b->actif = souris_sur_bouton(b, gs->mouse_x, gs->mouse_y);
    if (!was && b->actif && gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
}

void maj_sous_menu_enigme(GameState *gs, SousMenuEnigme *sm)
{
    if (strcmp(gs->input, "quitter") == 0) { gs->continuer = 0; return; }
    if (gs->motion) {
        hover(gs, &sm->btn_quiz); hover(gs, &sm->btn_puzzle);
        hover(gs, &sm->btn_a);   hover(gs, &sm->btn_b); hover(gs, &sm->btn_c);
    }
    if (gs->click) {
        if (sm->etat == ENIGME_CHOIX && souris_sur_bouton(&sm->btn_quiz, gs->mouse_x, gs->mouse_y)) {
            sm->etat = ENIGME_QUIZ;
            if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
        }
    }
}

void liberer_sous_menu_enigme(SousMenuEnigme *sm)
{
    liberer_background(&sm->backg);
    liberer_bouton(&sm->btn_quiz); liberer_bouton(&sm->btn_puzzle);
    liberer_bouton(&sm->btn_a);    liberer_bouton(&sm->btn_b); liberer_bouton(&sm->btn_c);
}
