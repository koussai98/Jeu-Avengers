#include "sous_menu_joueur.h"
#include <string.h>

void init_sous_menu_joueur(GameState *gs, SousMenuJoueur *sm)
{
    init_background(gs, &sm->backg,
                    "assets/images/background2.png",
                    "assets/sounds/music_joueur.ogg",
                    NIVEAU_JOUEUR);

    sm->etat = JOUEUR_CHOIX_MODE;
    int cx = SCREEN_W / 2;

    init_bouton(gs, &sm->btn_mono,    NULL, NULL, cx - 140, 280, 130, 50, "Mono joueur");
    init_bouton(gs, &sm->btn_multi,   NULL, NULL, cx +  10, 280, 130, 50, "Multi joueurs");
    init_bouton(gs, &sm->btn_avatar1, NULL, NULL, cx - 140, 240, 120, 45, "Avatar 1");
    init_bouton(gs, &sm->btn_avatar2, NULL, NULL, cx +  20, 240, 120, 45, "Avatar 2");
    init_bouton(gs, &sm->btn_input1,  NULL, NULL, cx - 140, 300, 120, 45, "Input 1");
    init_bouton(gs, &sm->btn_input2,  NULL, NULL, cx +  20, 300, 120, 45, "Input 2");
    init_bouton(gs, &sm->btn_valider, NULL, NULL, cx -  60, 370, 120, 45, "Valider");
    init_bouton(gs, &sm->btn_retour,  NULL, NULL, cx -  60, 430, 120, 40, "Retour");

    if (sm->backg.musique) {
        Mix_VolumeMusic(sm->backg.volume);
        Mix_PlayMusic(sm->backg.musique, -1);
    }
}

void afficher_sous_menu_joueur(GameState *gs, SousMenuJoueur *sm)
{
    afficher_background(gs->renderer, &sm->backg);
    draw_rounded_rect(gs->renderer, SCREEN_W/2 - 200, 160, 400, 330, 20, 20, 60, 200);

    SDL_Color jaune = {255, 220,  50, 255};
    SDL_Color blanc = {255, 255, 255, 255};
    afficher_texte_centre(gs->renderer, gs->font_large, "JOUEUR", jaune, 0, SCREEN_W, 80);

    if (sm->etat == JOUEUR_CHOIX_MODE) {
        afficher_texte_centre(gs->renderer, gs->font, "Choisissez le mode de jeu :",
                              blanc, 0, SCREEN_W, 240);
        afficher_bouton(gs->renderer, gs->font, &sm->btn_mono);
        afficher_bouton(gs->renderer, gs->font, &sm->btn_multi);
        afficher_bouton(gs->renderer, gs->font, &sm->btn_retour);
    } else {
        afficher_texte_centre(gs->renderer, gs->font, "Choisissez votre avatar et input :",
                              blanc, 0, SCREEN_W, 210);
        afficher_bouton(gs->renderer, gs->font, &sm->btn_avatar1);
        afficher_bouton(gs->renderer, gs->font, &sm->btn_avatar2);
        afficher_bouton(gs->renderer, gs->font, &sm->btn_input1);
        afficher_bouton(gs->renderer, gs->font, &sm->btn_input2);
        afficher_bouton(gs->renderer, gs->font, &sm->btn_valider);
        afficher_bouton(gs->renderer, gs->font, &sm->btn_retour);
    }
}

void input_sous_menu_joueur(GameState *gs)
{
    SDL_Event ev;
    gs->click = 0; gs->motion = 0; strcpy(gs->input, "");
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) gs->continuer = 0;
        else if (ev.type == SDL_KEYDOWN) {
            if (ev.key.keysym.sym == SDLK_RETURN) strcpy(gs->input, "valider");
            else if (ev.key.keysym.sym == SDLK_ESCAPE) strcpy(gs->input, "retour");
        }
        else if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
            gs->click = 1; gs->mouse_x = ev.button.x; gs->mouse_y = ev.button.y;
        }
        else if (ev.type == SDL_MOUSEMOTION) {
            gs->motion = 1; gs->mouse_x = ev.motion.x; gs->mouse_y = ev.motion.y;
        }
    }
}

static void update_hover(GameState *gs, Bouton *b) {
    int was = b->actif;
    b->actif = souris_sur_bouton(b, gs->mouse_x, gs->mouse_y);
    if (!was && b->actif && gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
}

void maj_sous_menu_joueur(GameState *gs, SousMenuJoueur *sm)
{
    if (strcmp(gs->input, "retour") == 0)  { gs->backg.niveau = NIVEAU_MENU; return; }
    if (strcmp(gs->input, "valider") == 0 && sm->etat == JOUEUR_CHOIX_CONFIG) {
        gs->backg.niveau = NIVEAU_SCORES; return;
    }

    if (gs->motion) {
        if (sm->etat == JOUEUR_CHOIX_MODE) {
            update_hover(gs, &sm->btn_mono);
            update_hover(gs, &sm->btn_multi);
            update_hover(gs, &sm->btn_retour);
        } else {
            update_hover(gs, &sm->btn_avatar1);
            update_hover(gs, &sm->btn_avatar2);
            update_hover(gs, &sm->btn_input1);
            update_hover(gs, &sm->btn_input2);
            update_hover(gs, &sm->btn_valider);
            update_hover(gs, &sm->btn_retour);
        }
    }

    if (gs->click) {
        if (sm->etat == JOUEUR_CHOIX_MODE) {
            if (souris_sur_bouton(&sm->btn_mono,  gs->mouse_x, gs->mouse_y) ||
                souris_sur_bouton(&sm->btn_multi, gs->mouse_x, gs->mouse_y)) {
                sm->etat = JOUEUR_CHOIX_CONFIG;
                if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
            }
            else if (souris_sur_bouton(&sm->btn_retour, gs->mouse_x, gs->mouse_y))
                gs->backg.niveau = NIVEAU_MENU;
        } else {
            if (souris_sur_bouton(&sm->btn_valider, gs->mouse_x, gs->mouse_y)) {
                gs->backg.niveau = NIVEAU_JEU;
                if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
            }
            else if (souris_sur_bouton(&sm->btn_retour, gs->mouse_x, gs->mouse_y))
                gs->backg.niveau = NIVEAU_MENU;
        }
    }
}

void liberer_sous_menu_joueur(SousMenuJoueur *sm)
{
    liberer_background(&sm->backg);
    liberer_bouton(&sm->btn_mono);   liberer_bouton(&sm->btn_multi);
    liberer_bouton(&sm->btn_avatar1);liberer_bouton(&sm->btn_avatar2);
    liberer_bouton(&sm->btn_input1); liberer_bouton(&sm->btn_input2);
    liberer_bouton(&sm->btn_valider);liberer_bouton(&sm->btn_retour);
}
