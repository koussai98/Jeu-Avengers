#ifndef QUIZ_H
#define QUIZ_H

#include "types.h"
#include "utils.h"

#define QUIZ_NB_CATEGORIES   3
#define QUIZ_NB_PAR_CAT      4
#define QUIZ_NB_OPTIONS      4

typedef struct {
    const char *categorie;
    const char *question;
    const char *options[QUIZ_NB_OPTIONS];
    int         bonne_reponse;
} QuizQuestion;

typedef enum {
    QUIZ_EN_COURS = 0,
    QUIZ_REUSSI   = 1,
    QUIZ_ECHOUE   = -1
} QuizResultat;

typedef struct {
    int          question_idx;
    int          choix_survol;
    QuizResultat resultat;
    SDL_Rect     rects_options[QUIZ_NB_OPTIONS];
    Bouton       btn_retour;
    Uint32       fin_timer;
} Quiz;


extern const QuizQuestion QUIZ_BASE[QUIZ_NB_CATEGORIES * QUIZ_NB_PAR_CAT];


void quiz_init   (GameState *gs, Quiz *q);
void quiz_input  (GameState *gs, Quiz *q);
void quiz_update (GameState *gs, Quiz *q);
void quiz_draw   (GameState *gs, Quiz *q);
void quiz_free   (Quiz *q);

int  quiz_est_termine (Quiz *q);
int  quiz_resultat    (Quiz *q);

#endif
