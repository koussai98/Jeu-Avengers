/*
 * sous_menu_sauvegarde.c
 * ──────────────────────
 * Implémentation du "Sous Menu Sauvegarde et Chargement"
 * conforme au cahier des charges et à la méthodologie de
 * conception (boucle de jeu : Init → Affichage → Input → MAJ).
 *
 * Logique :
 *   Phase ETAT_QUESTION  → "Voulez-vous sauvegarder votre jeu ?"
 *                           [OUI]  → passe à ETAT_CHOIX
 *                           [NON]  → retour au menu principal
 *
 *   Phase ETAT_CHOIX     → [Charger le jeu] → (à brancher sur la logique de chargement)
 *                           [Nouvelle Partie] ou touche 'n' → NIVEAU_JOUEUR
 */

#include "sous_menu_sauvegarde.h"
#include <string.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════════════
 *  INITIALISATION
 *  Organigramme figure 6 (Init Background + Init Boutons)
 * ═══════════════════════════════════════════════════════════════════ */
void init_sous_menu_sauvegarde(GameState *gs, SousMenuSauvegarde *sm)
{
    /* ── Initialisation Background (figure 7) ── */
    init_background(gs, &sm->backg,
                    "assets/images/background2.png",   /* arrière-plan 2        */
                    "assets/sounds/music_options.ogg",  /* musique de fond       */
                    NIVEAU_SAUVEGARDE);

    sm->etat = ETAT_QUESTION;   /* toujours démarrer par la question */

    /* ── Initialisation Boutons Phase 1 : OUI / NON (figure 8) ── */
    int cx = SCREEN_W / 2;

    init_bouton(gs, &sm->btn_oui,
                NULL, NULL,
                cx - 140, 340, 120, 50,
                "OUI");

    init_bouton(gs, &sm->btn_non,
                NULL, NULL,
                cx + 20,  340, 120, 50,
                "NON");

    /* ── Initialisation Boutons Phase 2 : Charger / Nouvelle Partie ── */
    init_bouton(gs, &sm->btn_charger,
                NULL, NULL,
                cx - 130, 280, 260, 55,
                "Charger le jeu");

    init_bouton(gs, &sm->btn_nouvelle,
                NULL, NULL,
                cx - 130, 355, 260, 55,
                "Nouvelle Partie");

    /* ── Lancer la musique de fond ── */
    if (sm->backg.musique) {
        Mix_VolumeMusic(sm->backg.volume);
        Mix_PlayMusic(sm->backg.musique, -1);
    }
}

/* ═══════════════════════════════════════════════════════════════════
 *  AFFICHAGE
 *  Organigramme figure 10 – branche NIVEAU_SAUVEGARDE
 * ═══════════════════════════════════════════════════════════════════ */
void afficher_sous_menu_sauvegarde(GameState *gs, SousMenuSauvegarde *sm)
{
    /* Afficher Background */
    afficher_background(gs->renderer, &sm->backg);

    /* ── Panneau central semi-transparent ── */
    draw_rounded_rect(gs->renderer,
                      SCREEN_W / 2 - 200, 160,
                      400, 300,
                      20, 20, 60, 200);

    /* ── Titre ── */
    SDL_Color jaune = {255, 220, 50, 255};
    SDL_Color blanc = {255, 255, 255, 255};

    afficher_texte_centre(gs->renderer, gs->font_large,
                          "SAUVEGARDE",
                          jaune, 0, SCREEN_W, 80);

    if (sm->etat == ETAT_QUESTION) {
        /* ── Phase 1 : Question OUI / NON ── */
        afficher_texte_centre(gs->renderer, gs->font,
                              "Voulez-vous sauvegarder votre jeu ?",
                              blanc, 0, SCREEN_W, 260);

        afficher_bouton(gs->renderer, gs->font, &sm->btn_oui);
        afficher_bouton(gs->renderer, gs->font, &sm->btn_non);

    } else {
        /* ── Phase 2 : Charger / Nouvelle Partie ── */
        afficher_texte_centre(gs->renderer, gs->font,
                              "Choisissez une option :",
                              blanc, 0, SCREEN_W, 240);

        afficher_bouton(gs->renderer, gs->font, &sm->btn_charger);
        afficher_bouton(gs->renderer, gs->font, &sm->btn_nouvelle);
    }
}

/* ═══════════════════════════════════════════════════════════════════
 *  LECTURE ENTRÉE (INPUT)
 *  Organigramme figure 11
 * ═══════════════════════════════════════════════════════════════════ */
void input_sous_menu_sauvegarde(GameState *gs)
{
    SDL_Event ev;
    gs->click  = 0;
    gs->motion = 0;
    strcpy(gs->input, "");

    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) {
            gs->continuer = 0;

        } else if (ev.type == SDL_KEYDOWN) {
            switch (ev.key.keysym.sym) {
                /* Touche 'n' → Nouvelle Partie (depuis ETAT_CHOIX) */
                case SDLK_n:
                    strcpy(gs->input, "nouvelle");
                    break;
                /* Échap → retour menu principal */
                case SDLK_ESCAPE:
                    strcpy(gs->input, "retour");
                    break;
                default:
                    break;
            }

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

/* ═══════════════════════════════════════════════════════════════════
 *  MISE À JOUR (UPDATE)
 *  Organigrammes figures 12-14
 * ═══════════════════════════════════════════════════════════════════ */
void maj_sous_menu_sauvegarde(GameState *gs, SousMenuSauvegarde *sm)
{
    /* ── Touche clavier ── */
    if (strcmp(gs->input, "retour")   == 0) {
        gs->backg.niveau = NIVEAU_MENU;
        if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
        return;
    }
    if (strcmp(gs->input, "nouvelle") == 0 && sm->etat == ETAT_CHOIX) {
        gs->backg.niveau = NIVEAU_JOUEUR;
        if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
        return;
    }

    /* ── Survol souris → boutons actifs + son bref ── */
    if (gs->motion) {
        if (sm->etat == ETAT_QUESTION) {
            /* Bouton OUI */
            int was = sm->btn_oui.actif;
            sm->btn_oui.actif = souris_sur_bouton(&sm->btn_oui,
                                                  gs->mouse_x, gs->mouse_y);
            if (!was && sm->btn_oui.actif && gs->son_bref)
                Mix_PlayChannel(-1, gs->son_bref, 0);

            /* Bouton NON */
            was = sm->btn_non.actif;
            sm->btn_non.actif = souris_sur_bouton(&sm->btn_non,
                                                  gs->mouse_x, gs->mouse_y);
            if (!was && sm->btn_non.actif && gs->son_bref)
                Mix_PlayChannel(-1, gs->son_bref, 0);

        } else { /* ETAT_CHOIX */
            /* Bouton Charger */
            int was = sm->btn_charger.actif;
            sm->btn_charger.actif = souris_sur_bouton(&sm->btn_charger,
                                                      gs->mouse_x, gs->mouse_y);
            if (!was && sm->btn_charger.actif && gs->son_bref)
                Mix_PlayChannel(-1, gs->son_bref, 0);

            /* Bouton Nouvelle Partie */
            was = sm->btn_nouvelle.actif;
            sm->btn_nouvelle.actif = souris_sur_bouton(&sm->btn_nouvelle,
                                                       gs->mouse_x, gs->mouse_y);
            if (!was && sm->btn_nouvelle.actif && gs->son_bref)
                Mix_PlayChannel(-1, gs->son_bref, 0);
        }
    }

    /* ── Clic souris ── */
    if (gs->click) {
        if (sm->etat == ETAT_QUESTION) {
            /* [OUI] → passer à la phase de choix */
            if (souris_sur_bouton(&sm->btn_oui, gs->mouse_x, gs->mouse_y)) {
                sm->etat = ETAT_CHOIX;
                /* Réinitialiser état actif */
                sm->btn_oui.actif     = 0;
                sm->btn_non.actif     = 0;
                sm->btn_charger.actif = 0;
                sm->btn_nouvelle.actif= 0;
                if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
            }
            /* [NON] → retour menu principal */
            else if (souris_sur_bouton(&sm->btn_non, gs->mouse_x, gs->mouse_y)) {
                gs->backg.niveau = NIVEAU_MENU;
                if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
            }

        } else { /* ETAT_CHOIX */
            /* [Charger le jeu] – à connecter à votre logique de sauvegarde */
            if (souris_sur_bouton(&sm->btn_charger, gs->mouse_x, gs->mouse_y)) {
                printf("[INFO] Chargement de la sauvegarde...\n");
                /* TODO: appeler votre fonction de chargement ici */
                gs->backg.niveau = NIVEAU_JOUEUR;   /* ou NIVEAU_JEU quand disponible */
                if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
            }
            /* [Nouvelle Partie] → sous menu joueur */
            else if (souris_sur_bouton(&sm->btn_nouvelle, gs->mouse_x, gs->mouse_y)) {
                gs->backg.niveau = NIVEAU_JOUEUR;
                if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
            }
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════
 *  LIBÉRATION ESPACE MÉMOIRE
 *  Organigramme figure 15
 * ═══════════════════════════════════════════════════════════════════ */
void liberer_sous_menu_sauvegarde(SousMenuSauvegarde *sm)
{
    liberer_background(&sm->backg);  /* libère image + musique */
    liberer_bouton(&sm->btn_oui);
    liberer_bouton(&sm->btn_non);
    liberer_bouton(&sm->btn_charger);
    liberer_bouton(&sm->btn_nouvelle);
}
