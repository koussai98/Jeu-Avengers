#ifndef SOUS_MENU_JOUEUR_H
#define SOUS_MENU_JOUEUR_H

#include "types.h"
#include "utils.h"

typedef enum { JOUEUR_CHOIX_MODE = 0, JOUEUR_CHOIX_CONFIG = 1 } EtatMenuJoueur;

typedef struct {
    Background  backg;
    EtatMenuJoueur  etat;
    /* Mode */
    Bouton      btn_mono;
    Bouton      btn_multi;
    /* Config */
    Bouton      btn_avatar1;
    Bouton      btn_avatar2;
    Bouton      btn_input1;
    Bouton      btn_input2;
    Bouton      btn_valider;
    Bouton      btn_retour;
} SousMenuJoueur;

void init_sous_menu_joueur(GameState *gs, SousMenuJoueur *sm);
void afficher_sous_menu_joueur(GameState *gs, SousMenuJoueur *sm);
void input_sous_menu_joueur(GameState *gs);
void maj_sous_menu_joueur(GameState *gs, SousMenuJoueur *sm);
void liberer_sous_menu_joueur(SousMenuJoueur *sm);

#endif
