#ifndef DEATH_CHOICE_H
#define DEATH_CHOICE_H

#include "types.h"
#include "utils.h"


typedef struct {
    Bouton btn_puzzle;
    Bouton btn_quiz;
    Bouton btn_retour;
} DeathChoice;

void death_choice_init  (GameState *gs, DeathChoice *dc);
void death_choice_input (GameState *gs, DeathChoice *dc);
void death_choice_update(GameState *gs, DeathChoice *dc);
void death_choice_draw  (GameState *gs, DeathChoice *dc);
void death_choice_free  (DeathChoice *dc);

#endif
