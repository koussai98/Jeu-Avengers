#ifndef SOUS_MENU_ENIGME_H
#define SOUS_MENU_ENIGME_H

#include "types.h"
#include "utils.h"

typedef enum { ENIGME_CHOIX = 0, ENIGME_QUIZ = 1 } EtatEnigme;

typedef struct {
    Background  backg;
    EtatEnigme  etat;
    Bouton      btn_quiz;
    Bouton      btn_puzzle;
    Bouton      btn_a, btn_b, btn_c;
} SousMenuEnigme;

void init_sous_menu_enigme(GameState *gs, SousMenuEnigme *sm);
void afficher_sous_menu_enigme(GameState *gs, SousMenuEnigme *sm);
void input_sous_menu_enigme(GameState *gs);
void maj_sous_menu_enigme(GameState *gs, SousMenuEnigme *sm);
void liberer_sous_menu_enigme(SousMenuEnigme *sm);

#endif
