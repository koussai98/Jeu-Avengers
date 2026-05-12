#ifndef SOUS_MENU_SCORES_H
#define SOUS_MENU_SCORES_H

#include "types.h"
#include "utils.h"

#define MAX_NOM 32
#define MAX_SCORES 3

typedef enum { SCORES_SAISIE = 0, SCORES_LISTE = 1 } EtatScores;

typedef struct {
    char nom[MAX_NOM];
    int  score;
} EntreeScore;

typedef struct {
    Background  backg;
    EtatScores  etat;
    char        nom_saisi[MAX_NOM];
    int         nom_len;
    EntreeScore meilleurs[MAX_SCORES];
    Bouton      btn_valider;
    Bouton      btn_retour;
    Bouton      btn_quitter;
} SousMenuScores;

void init_sous_menu_scores(GameState *gs, SousMenuScores *sm);
void afficher_sous_menu_scores(GameState *gs, SousMenuScores *sm);
void input_sous_menu_scores(GameState *gs, SousMenuScores *sm);
void maj_sous_menu_scores(GameState *gs, SousMenuScores *sm);
void liberer_sous_menu_scores(SousMenuScores *sm);

#endif
