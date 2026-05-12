#include "death_choice.h"
#include <string.h>

void death_choice_init(GameState *gs, DeathChoice *dc)
{
    int btn_w = 320, btn_h = 60;
    int cx = SCREEN_W / 2;
    int gap = 26;
    int start_y = SCREEN_H / 2 - btn_h - gap / 2;

    init_bouton(gs, &dc->btn_puzzle, NULL, NULL,
                cx - btn_w / 2, start_y, btn_w, btn_h, "Take the Puzzle");
    init_bouton(gs, &dc->btn_quiz, NULL, NULL,
                cx - btn_w / 2, start_y + btn_h + gap, btn_w, btn_h, "Take the Quiz");
    init_bouton(gs, &dc->btn_retour, NULL, NULL,
                40, SCREEN_H - 70, 140, 45, "Abandonner");
}

void death_choice_input(GameState *gs, DeathChoice *dc)
{
    SDL_Event ev;
    (void)dc;
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

void death_choice_update(GameState *gs, DeathChoice *dc)
{
    if (gs->motion) {
        dc->btn_puzzle.actif = souris_sur_bouton(&dc->btn_puzzle,
                                                  gs->mouse_x, gs->mouse_y);
        dc->btn_quiz.actif   = souris_sur_bouton(&dc->btn_quiz,
                                                  gs->mouse_x, gs->mouse_y);
        dc->btn_retour.actif = souris_sur_bouton(&dc->btn_retour,
                                                  gs->mouse_x, gs->mouse_y);
    }

    if (gs->click) {
        if (souris_sur_bouton(&dc->btn_puzzle, gs->mouse_x, gs->mouse_y)) {
            gs->backg.niveau = NIVEAU_PUZZLE;
            if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
        } else if (souris_sur_bouton(&dc->btn_quiz, gs->mouse_x, gs->mouse_y)) {
            gs->backg.niveau = NIVEAU_QUIZ;
            if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
        } else if (souris_sur_bouton(&dc->btn_retour, gs->mouse_x, gs->mouse_y)) {
            gs->backg.niveau = NIVEAU_MENU;
            if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
        }
    }
}

void death_choice_draw(GameState *gs, DeathChoice *dc)
{
    SDL_Renderer *r = gs->renderer;

    SDL_SetRenderDrawColor(r, 12, 12, 28, 255);
    SDL_RenderClear(r);

    SDL_Color rouge = {220,  60,  60, 255};
    SDL_Color blanc = {235, 235, 245, 255};
    SDL_Color gris  = {180, 180, 200, 255};

    afficher_texte_centre(r, gs->font_large ? gs->font_large : gs->font,
                          "TU ES MORT !", rouge, 0, SCREEN_W, 100);
    afficher_texte_centre(r, gs->font,
                          "Choisis ton epreuve pour respawn :",
                          blanc, 0, SCREEN_W, 160);
    afficher_texte_centre(r, gs->font,
                          "(Reussir = revivre   |   Echouer = retour menu)",
                          gris, 0, SCREEN_W, SCREEN_H - 120);

    afficher_bouton(r, gs->font_large ? gs->font_large : gs->font, &dc->btn_puzzle);
    afficher_bouton(r, gs->font_large ? gs->font_large : gs->font, &dc->btn_quiz);
    afficher_bouton(r, gs->font, &dc->btn_retour);
}

void death_choice_free(DeathChoice *dc)
{
    liberer_bouton(&dc->btn_puzzle);
    liberer_bouton(&dc->btn_quiz);
    liberer_bouton(&dc->btn_retour);
}
