#ifndef MENU_PRINCIPAL_H
#define MENU_PRINCIPAL_H

#include "types.h"
#include "utils.h"

/* 6 boutons : PvP, Campagne, Options, Meilleurs Scores, Histoire, Quitter */
#define NB_BOUTONS_MENU 6

typedef struct {
    Background backg;
    Bouton     boutons[NB_BOUTONS_MENU];
} MenuPrincipal;

void init_menu_principal(GameState *gs, MenuPrincipal *m);
void afficher_menu_principal(GameState *gs, MenuPrincipal *m);
void input_menu_principal(GameState *gs);
void maj_menu_principal(GameState *gs, MenuPrincipal *m);
void liberer_menu_principal(MenuPrincipal *m);

#endif
