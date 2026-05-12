#ifndef SOUS_MENU_OPTIONS_H
#define SOUS_MENU_OPTIONS_H

#include "types.h"
#include "utils.h"

#define NB_BOUTONS_OPTIONS 5  /* Diminuer, Augmenter, Normal, Plein écran, Retour */

typedef struct {
    Background backg;
    Bouton     boutons[NB_BOUTONS_OPTIONS];
} SousMenuOptions;

void init_sous_menu_options(GameState *gs, SousMenuOptions *sm);
void afficher_sous_menu_options(GameState *gs, SousMenuOptions *sm);
void input_sous_menu_options(GameState *gs);
void maj_sous_menu_options(GameState *gs, SousMenuOptions *sm);
void liberer_sous_menu_options(SousMenuOptions *sm);

#endif
