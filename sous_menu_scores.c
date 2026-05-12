#include "sous_menu_scores.h"
#include <string.h>
#include <stdio.h>

void init_sous_menu_scores(GameState *gs, SousMenuScores *sm)
{
    init_background(gs, &sm->backg,
                    "assets/images/background3.png",
                    NULL,           /* pas de musique au départ */
                    NIVEAU_SCORES);

    sm->etat   = SCORES_SAISIE;
    sm->nom_len = 0;
    memset(sm->nom_saisi, 0, sizeof(sm->nom_saisi));

    /* Scores fictifs pour la démo */
    snprintf(sm->meilleurs[0].nom, MAX_NOM, "Alice"); sm->meilleurs[0].score = 9500;
    snprintf(sm->meilleurs[1].nom, MAX_NOM, "Bob");   sm->meilleurs[1].score = 7200;
    snprintf(sm->meilleurs[2].nom, MAX_NOM, "Carol");  sm->meilleurs[2].score = 5100;

    int cx = SCREEN_W / 2;
    init_bouton(gs, &sm->btn_valider, NULL, NULL, cx -  60, 360, 120, 45, "Valider");
    init_bouton(gs, &sm->btn_retour,  NULL, NULL, cx - 130, 500, 120, 40, "Retour");
    init_bouton(gs, &sm->btn_quitter, NULL, NULL, cx +  10, 500, 120, 40, "Quitter");
}

void afficher_sous_menu_scores(GameState *gs, SousMenuScores *sm)
{
    afficher_background(gs->renderer, &sm->backg);
    draw_rounded_rect(gs->renderer, SCREEN_W/2 - 220, 120, 440, 360, 20, 20, 60, 200);

    SDL_Color jaune  = {255, 220,  50, 255};
    SDL_Color blanc  = {255, 255, 255, 255};
    SDL_Color gris   = {180, 180, 180, 255};

    afficher_texte_centre(gs->renderer, gs->font_large,
                          "MEILLEURS SCORES", jaune, 0, SCREEN_W, 70);

    if (sm->etat == SCORES_SAISIE) {
        afficher_texte_centre(gs->renderer, gs->font,
                              "Saisir Nom / Pseudo :", blanc, 0, SCREEN_W, 230);

        /* Champ de saisie */
        draw_rounded_rect(gs->renderer, SCREEN_W/2 - 120, 270, 240, 40, 40, 40, 80, 220);
        char affiche[MAX_NOM + 2];
        snprintf(affiche, sizeof(affiche), "%s_", sm->nom_saisi);
        afficher_texte_centre(gs->renderer, gs->font, affiche, blanc,
                              SCREEN_W/2 - 120, 240, 278);

        afficher_bouton(gs->renderer, gs->font, &sm->btn_valider);

    } else { /* SCORES_LISTE */
        afficher_texte_centre(gs->renderer, gs->font,
                              "Top 3 :", blanc, 0, SCREEN_W, 165);

        char ligne[64];
        int  stars_y[] = {200, 255, 310};
        for (int i = 0; i < MAX_SCORES; i++) {
            snprintf(ligne, sizeof(ligne), "%d. %s   %d pts",
                     i + 1, sm->meilleurs[i].nom, sm->meilleurs[i].score);
            afficher_texte_centre(gs->renderer, gs->font, ligne,
                                  i == 0 ? jaune : (i == 1 ? blanc : gris),
                                  0, SCREEN_W, stars_y[i]);
        }
        afficher_bouton(gs->renderer, gs->font, &sm->btn_retour);
        afficher_bouton(gs->renderer, gs->font, &sm->btn_quitter);
    }
}

void input_sous_menu_scores(GameState *gs, SousMenuScores *sm)
{
    SDL_Event ev;
    gs->click = 0; gs->motion = 0; strcpy(gs->input, "");

    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) { gs->continuer = 0; return; }

        if (ev.type == SDL_KEYDOWN) {
            SDL_Keycode k = ev.key.keysym.sym;
            if (sm->etat == SCORES_SAISIE) {
                if (k == SDLK_RETURN) {
                    strcpy(gs->input, "valider");
                } else if (k == SDLK_BACKSPACE && sm->nom_len > 0) {
                    sm->nom_saisi[--sm->nom_len] = '\0';
                } else if (k == SDLK_e) {
                    strcpy(gs->input, "enigme");
                }
            } else {
                if (k == SDLK_ESCAPE) strcpy(gs->input, "quitter");
                if (k == SDLK_e)      strcpy(gs->input, "enigme");
            }
        }
        else if (ev.type == SDL_TEXTINPUT && sm->etat == SCORES_SAISIE) {
            if (sm->nom_len < MAX_NOM - 1) {
                sm->nom_saisi[sm->nom_len++] = ev.text.text[0];
                sm->nom_saisi[sm->nom_len]   = '\0';
            }
        }
        else if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
            gs->click = 1; gs->mouse_x = ev.button.x; gs->mouse_y = ev.button.y;
        }
        else if (ev.type == SDL_MOUSEMOTION) {
            gs->motion = 1; gs->mouse_x = ev.motion.x; gs->mouse_y = ev.motion.y;
        }
    }
}

void maj_sous_menu_scores(GameState *gs, SousMenuScores *sm)
{
    if (strcmp(gs->input, "enigme")  == 0) { gs->backg.niveau = NIVEAU_ENIGME; return; }
    if (strcmp(gs->input, "quitter") == 0) { gs->continuer    = 0; return; }
    if (strcmp(gs->input, "valider") == 0 && sm->etat == SCORES_SAISIE) {
        sm->etat = SCORES_LISTE;
        /* Lancer musique victoire */
        if (sm->backg.musique) Mix_PlayMusic(sm->backg.musique, -1);
        return;
    }

    if (gs->motion) {
        int was;
        was = sm->btn_valider.actif; sm->btn_valider.actif = souris_sur_bouton(&sm->btn_valider, gs->mouse_x, gs->mouse_y);
        if (!was && sm->btn_valider.actif && gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
        was = sm->btn_retour.actif;  sm->btn_retour.actif  = souris_sur_bouton(&sm->btn_retour,  gs->mouse_x, gs->mouse_y);
        if (!was && sm->btn_retour.actif  && gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
        was = sm->btn_quitter.actif; sm->btn_quitter.actif = souris_sur_bouton(&sm->btn_quitter, gs->mouse_x, gs->mouse_y);
        if (!was && sm->btn_quitter.actif && gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
    }

    if (gs->click) {
        if (souris_sur_bouton(&sm->btn_valider, gs->mouse_x, gs->mouse_y) && sm->etat == SCORES_SAISIE)
            sm->etat = SCORES_LISTE;
        else if (souris_sur_bouton(&sm->btn_retour,  gs->mouse_x, gs->mouse_y)) gs->backg.niveau = NIVEAU_MENU;
        else if (souris_sur_bouton(&sm->btn_quitter, gs->mouse_x, gs->mouse_y)) gs->continuer    = 0;
    }
}

void liberer_sous_menu_scores(SousMenuScores *sm)
{
    liberer_background(&sm->backg);
    liberer_bouton(&sm->btn_valider);
    liberer_bouton(&sm->btn_retour);
    liberer_bouton(&sm->btn_quitter);
}
