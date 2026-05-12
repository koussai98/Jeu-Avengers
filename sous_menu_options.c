#include "sous_menu_options.h"
#include <string.h>
#include <stdio.h>

void init_sous_menu_options(GameState *gs, SousMenuOptions *sm)
{
    init_background(gs, &sm->backg,
                    "assets/images/background4.png",
                    "assets/sounds/music_options.ogg",
                    NIVEAU_OPTIONS);

    int cx = SCREEN_W / 2;
    /* Volume – Diminuer / Augmenter */
    init_bouton(gs, &sm->boutons[0], NULL, NULL, cx - 100, 190, 120, 35, "Diminuer");
    init_bouton(gs, &sm->boutons[1], NULL, NULL, cx +  70, 190, 120, 35, "Augmenter");
    /* Affichage – Normal / Plein écran */
    init_bouton(gs, &sm->boutons[2], NULL, NULL, cx - 50, 290, 120, 35, "Normal");
    init_bouton(gs, &sm->boutons[3], NULL, NULL, cx + 100, 290, 120, 35, "Plein ecran");
    /* Retour */
    init_bouton(gs, &sm->boutons[4], NULL, NULL, cx -  30, 370, 120, 35, "Retour");

    if (sm->backg.musique) {
        Mix_VolumeMusic(sm->backg.volume);
        Mix_PlayMusic(sm->backg.musique, -1);
    }
}

void afficher_sous_menu_options(GameState *gs, SousMenuOptions *sm)
{
    afficher_background(gs->renderer, &sm->backg);

    SDL_Color blanc  = {255, 255, 255, 255};
    SDL_Color jaune  = {255, 220,  50, 255};



    /* Volume label + valeur */

    char vol_str[8];
    snprintf(vol_str, sizeof(vol_str), "%d", sm->backg.volume);
    afficher_texte_centre(gs->renderer, gs->font, vol_str, blanc,
                          SCREEN_W / 2 - 30, 150, 200);

    /* Affichage label */


    for (int i = 0; i < NB_BOUTONS_OPTIONS; i++)
        afficher_bouton(gs->renderer, gs->font, &sm->boutons[i]);
}

void input_sous_menu_options(GameState *gs)
{
    SDL_Event ev;
    gs->click  = 0;
    gs->motion = 0;
    strcpy(gs->input, "");

    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) gs->continuer = 0;
        else if (ev.type == SDL_KEYDOWN) {
            switch (ev.key.keysym.sym) {
                case SDLK_PLUS:
                case SDLK_KP_PLUS:  strcpy(gs->input, "+"); break;
                case SDLK_MINUS:
                case SDLK_KP_MINUS: strcpy(gs->input, "-"); break;
                case SDLK_f:        strcpy(gs->input, "F"); break;
                case SDLK_n:        strcpy(gs->input, "N"); break;
                case SDLK_ESCAPE:   strcpy(gs->input, "retour"); break;
                default: break;
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

void maj_sous_menu_options(GameState *gs, SousMenuOptions *sm)
{
    /* Clavier */
    if      (strcmp(gs->input, "+")      == 0) { sm->backg.volume = SDL_min(128, sm->backg.volume + 8); Mix_VolumeMusic(sm->backg.volume); }
    else if (strcmp(gs->input, "-")      == 0) { sm->backg.volume = SDL_max(0,   sm->backg.volume - 8); Mix_VolumeMusic(sm->backg.volume); }
    else if (strcmp(gs->input, "F")      == 0) { sm->backg.mode = 1; SDL_SetWindowFullscreen(gs->window, SDL_WINDOW_FULLSCREEN_DESKTOP); }
    else if (strcmp(gs->input, "N")      == 0) { sm->backg.mode = 0; SDL_SetWindowFullscreen(gs->window, 0); }
    else if (strcmp(gs->input, "retour") == 0) { gs->backg.niveau = NIVEAU_MENU; }

    /* Survol souris */
    if (gs->motion) {
        for (int i = 0; i < NB_BOUTONS_OPTIONS; i++) {
            int was = sm->boutons[i].actif;
            sm->boutons[i].actif = souris_sur_bouton(&sm->boutons[i], gs->mouse_x, gs->mouse_y);
            if (!was && sm->boutons[i].actif && gs->son_bref)
                Mix_PlayChannel(-1, gs->son_bref, 0);
        }
    }

    /* Clic */
    if (gs->click) {
        if (souris_sur_bouton(&sm->boutons[0], gs->mouse_x, gs->mouse_y)) {
            sm->backg.volume = SDL_max(0, sm->backg.volume - 8);
            Mix_VolumeMusic(sm->backg.volume);
        }
        else if (souris_sur_bouton(&sm->boutons[1], gs->mouse_x, gs->mouse_y)) {
            sm->backg.volume = SDL_min(128, sm->backg.volume + 8);
            Mix_VolumeMusic(sm->backg.volume);
        }
        else if (souris_sur_bouton(&sm->boutons[2], gs->mouse_x, gs->mouse_y)) {
            sm->backg.mode = 0;
            SDL_SetWindowFullscreen(gs->window, 0);
        }
        else if (souris_sur_bouton(&sm->boutons[3], gs->mouse_x, gs->mouse_y)) {
            sm->backg.mode = 1;
            SDL_SetWindowFullscreen(gs->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        }
        else if (souris_sur_bouton(&sm->boutons[4], gs->mouse_x, gs->mouse_y)) {
            gs->backg.niveau = NIVEAU_MENU;
            if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
        }
    }
}

void liberer_sous_menu_options(SousMenuOptions *sm)
{
    liberer_background(&sm->backg);
    for (int i = 0; i < NB_BOUTONS_OPTIONS; i++)
        liberer_bouton(&sm->boutons[i]);
}
