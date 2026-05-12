#ifndef SOUS_MENU_SAUVEGARDE_H
#define SOUS_MENU_SAUVEGARDE_H

#include "types.h"
#include "utils.h"

/*
 * États internes de ce sous-menu (machine à états)
 * ─────────────────────────────────────────────────
 *  ETAT_QUESTION  : affiche "Voulez-vous sauvegarder?" + OUI / NON
 *  ETAT_CHOIX     : affiche "Charger le jeu" + "Nouvelle Partie"
 */
typedef enum {
    ETAT_QUESTION = 0,
    ETAT_CHOIX    = 1
} EtatSauvegarde;

#define NB_BTN_QUESTION 2   /* OUI, NON                       */
#define NB_BTN_CHOIX    2   /* Charger le jeu, Nouvelle Partie */

typedef struct {
    Background      backg;
    EtatSauvegarde  etat;

    /* Phase 1 – question OUI/NON */
    Bouton btn_oui;
    Bouton btn_non;

    /* Phase 2 – choix après OUI */
    Bouton btn_charger;
    Bouton btn_nouvelle;
} SousMenuSauvegarde;

/* ── Game-loop functions (following conception guide) ── */
void init_sous_menu_sauvegarde(GameState *gs, SousMenuSauvegarde *sm);
void afficher_sous_menu_sauvegarde(GameState *gs, SousMenuSauvegarde *sm);
void input_sous_menu_sauvegarde(GameState *gs);
void maj_sous_menu_sauvegarde(GameState *gs, SousMenuSauvegarde *sm);
void liberer_sous_menu_sauvegarde(SousMenuSauvegarde *sm);

#endif /* SOUS_MENU_SAUVEGARDE_H */
