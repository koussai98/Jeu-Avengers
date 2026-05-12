#include "menu_principal.h"
#include <string.h>

/* Disposition verticale centrée des 6 boutons */
static const char *labels[NB_BOUTONS_MENU] = {
    "PvP (1v1)", "Campagne", "Options", "Meilleurs Scores", "Histoire", "Quitter"
};

void init_menu_principal(GameState *gs, MenuPrincipal *m)
{
    init_background(gs, &m->backg,
                    "assets/images/background1.png",
                    "assets/sounds/music_joueur.ogg",
                    NIVEAU_MENU);

    int bw = 220, bh = 50, gap = 15;
    int total_h = NB_BOUTONS_MENU * bh + (NB_BOUTONS_MENU - 1) * gap;
    int start_y = (SCREEN_H - total_h) / 2 + 30;
    int bx      = (SCREEN_W - bw) / 2 - 80; /* légèrement à gauche */

    for (int i = 0; i < NB_BOUTONS_MENU; i++) {
        init_bouton(gs, &m->boutons[i],
                    NULL, NULL,      /* pas de fichiers image requis */
                    bx, start_y + i * (bh + gap),
                    bw, bh, labels[i]);
    }

    /* Lancer la musique */
    if (m->backg.musique) {
        Mix_VolumeMusic(m->backg.volume);
        Mix_PlayMusic(m->backg.musique, -1);
    }
}

void afficher_menu_principal(GameState *gs, MenuPrincipal *m)
{
    afficher_background(gs->renderer, &m->backg);

    /* Titre */
    SDL_Color jaune = {255, 220, 50, 255};
    afficher_texte_centre(gs->renderer, gs->font_large, "AVENGERS",
                          jaune, 0, SCREEN_W, 60);

    /* Boutons */
    for (int i = 0; i < NB_BOUTONS_MENU; i++)
        afficher_bouton(gs->renderer, gs->font, &m->boutons[i]);
}

void input_menu_principal(GameState *gs)
{
    SDL_Event ev;
    gs->click  = 0;
    gs->motion = 0;
    strcpy(gs->input, "");

    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) {
            gs->continuer = 0;
        }
        else if (ev.type == SDL_KEYDOWN) {
            switch (ev.key.keysym.sym) {
                case SDLK_j:      strcpy(gs->input, "jouer");   break;
                case SDLK_o:      strcpy(gs->input, "options"); break;
                case SDLK_m:      strcpy(gs->input, "scores");  break;
                case SDLK_ESCAPE: strcpy(gs->input, "quitter"); break;
                default: break;
            }
        }
        else if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
            gs->click   = 1;
            gs->mouse_x = ev.button.x;
            gs->mouse_y = ev.button.y;
        }
        else if (ev.type == SDL_MOUSEMOTION) {
            gs->motion  = 1;
            gs->mouse_x = ev.motion.x;
            gs->mouse_y = ev.motion.y;
        }
    }
}

void maj_menu_principal(GameState *gs, MenuPrincipal *m)
{
    /* Touche clavier */
    if (strcmp(gs->input, "jouer") == 0) {
        gs->backg.niveau = NIVEAU_SAUVEGARDE;
        if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
    }
    else if (strcmp(gs->input, "options") == 0) {
        gs->backg.niveau = NIVEAU_OPTIONS;
        if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
    }
    else if (strcmp(gs->input, "quitter") == 0) {
        gs->continuer = 0;
    }

    /* Survol souris → activer/désactiver boutons */
    if (gs->motion) {
        for (int i = 0; i < NB_BOUTONS_MENU; i++) {
            int was = m->boutons[i].actif;
            m->boutons[i].actif = souris_sur_bouton(&m->boutons[i],
                                                    gs->mouse_x, gs->mouse_y);
            if (!was && m->boutons[i].actif && gs->son_bref)
                Mix_PlayChannel(-1, gs->son_bref, 0);
        }
    }

    /* Clic souris */
    if (gs->click) {
        /* PvP (1v1) */
        if (souris_sur_bouton(&m->boutons[0], gs->mouse_x, gs->mouse_y)) {
            gs->backg.niveau = NIVEAU_SAUVEGARDE;
            if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
        }
        /* Campagne -> champion select first */
        else if (souris_sur_bouton(&m->boutons[1], gs->mouse_x, gs->mouse_y)) {
            gs->backg.niveau = NIVEAU_CHAMPION;
            if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
        }
        /* Options */
        else if (souris_sur_bouton(&m->boutons[2], gs->mouse_x, gs->mouse_y)) {
            gs->backg.niveau = NIVEAU_OPTIONS;
            if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
        }
        /* Meilleurs Scores */
        else if (souris_sur_bouton(&m->boutons[3], gs->mouse_x, gs->mouse_y)) {
            gs->backg.niveau = NIVEAU_SCORES;
            if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
        }
        /* Quitter */
        else if (souris_sur_bouton(&m->boutons[5], gs->mouse_x, gs->mouse_y)) {
            gs->continuer = 0;
        }
    }
}

void liberer_menu_principal(MenuPrincipal *m)
{
    liberer_background(&m->backg);
    for (int i = 0; i < NB_BOUTONS_MENU; i++)
        liberer_bouton(&m->boutons[i]);
}
