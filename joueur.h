#ifndef JOUEUR_H
#define JOUEUR_H



#include "types.h"
#include "utils.h"


#define SPRITE_RENDER_W    140
#define SPRITE_RENDER_H    140


#define SHEET_W           1536
#define SHEET_H           1024

#define SPRITE_COLORKEY_R   255
#define SPRITE_COLORKEY_G   255
#define SPRITE_COLORKEY_B   255
#define SPRITE_COLORKEY_TOL  30     


#define FRAMES_IDLE         3
#define FRAMES_WALK         6
#define FRAMES_RUN          7
#define FRAMES_JUMP         4
#define FRAMES_ATTACK       3


#define ANIM_SPEED_IDLE     14
#define ANIM_SPEED_WALK      8
#define ANIM_SPEED_RUN       4      
#define ANIM_SPEED_JUMP     10
#define ANIM_SPEED_ATTACK    5


#define IDLE_Y    30
#define IDLE_H   165
extern const SDL_Rect FRAMES_IDLE_RECTS[4];

#define WALK_Y   211
#define WALK_H   208
extern const SDL_Rect FRAMES_WALK_RECTS[7];

#define ATTACK_Y 431
#define ATTACK_H 204
extern const SDL_Rect FRAMES_ATTACK_RECTS[4];

#define JUMP_Y   648
#define JUMP_H   235
extern const SDL_Rect FRAMES_JUMP_RECTS[3];


#define GRAVITE             0.5f
#define VITESSE_SAUT       -12.0f
#define VITESSE_MARCHE      3
#define VITESSE_COURSE      6

#define SOL_Y              (SCREEN_H - SPRITE_RENDER_H - 10)

#define VIES_INITIALES      3
#define SCORE_ATTAQUE       10
#define DUREE_ATTAQUE       20
#define NB_COSTUMES         3
#define NOM_MAX             32

#define INVINCIBILITE_FRAMES  45


#define HITBOX_REACH   70
#define HITBOX_H       90


typedef enum {
    ETAT_IDLE   = 0,
    ETAT_WALK   = 1,
    ETAT_RUN    = 2,
    ETAT_JUMP   = 3,
    ETAT_ATTACK = 4
} EtatJoueur;

typedef enum {
    DIR_DROITE =  1,
    DIR_GAUCHE = -1
} Direction;

typedef enum {
    JOUEUR_1 = 1,
    JOUEUR_2 = 2
} NumeroJoueur;


typedef struct {
    int frame_courante;
    int nb_frames;
    int timer;
    int vitesse;
    int ligne_spritesheet;
} Animation;


typedef struct {
    SDL_Keycode gauche;
    SDL_Keycode droite;
    SDL_Keycode courir;
    SDL_Keycode sauter;
    SDL_Keycode attaquer;
} TouchesJoueur;

typedef struct {
    
    char         nom[NOM_MAX];
    NumeroJoueur numero;
    int          costume_idx;

    
    float        x, y;
    float        vy;
    int          au_sol;
    Direction    direction;

    EtatJoueur   etat;
    Animation    anims[5];
    int          attaque_timer;

    SDL_Texture *sprite_sheet;
    SDL_Color    couleur_placeholder;


    int          vies;
    int          score;
    int          est_vivant;

    TouchesJoueur touches;

    int          touche_gauche;
    int          touche_droite;
    int          touche_courir;

    int          attaque_cooldown;


    int          world_w_max;


    int          use_grid_animation;

} Personnage;


#define NB_BTN_COSTUME  (NB_COSTUMES + 1)

typedef struct {
    Personnage  *joueur_cible;
    Bouton       boutons[NB_BTN_COSTUME];
    int          costume_survol;
} SousMenuCostume;

SDL_Texture *charger_sprite_knight(SDL_Renderer *renderer, const char *chemin);


const char *champion_sprite_path(int champion_id);
void        joueur_set_champion (GameState *gs, Personnage *p, int champion_id);


void init_personnage_base(GameState *gs, Personnage *p,
                          NumeroJoueur numero, const char *nom,
                          float x, TouchesJoueur touches, SDL_Color couleur);
void initialiserJoueur1(GameState *gs, Personnage *p);
void initialiserJoueur2(GameState *gs, Personnage *p);


void afficherJoueur(GameState *gs, Personnage *p);
void afficherUI_Joueur(GameState *gs, Personnage *p, int ui_x, int ui_y);


void changerEtat(Personnage *p, EtatJoueur nouvel_etat);
void mettreAJourAnimation(Personnage *p);


void gererToucheAppuyee(Personnage *p, SDL_Keycode touche);
void gererToucheRelachee(Personnage *p, SDL_Keycode touche);


void mettreAJourJoueur(Personnage *p);



int verifierCollisionAttaque(Personnage *attaquant, Personnage *cible);


void ajouterScore(Personnage *p, int points);
void perdreVie(Personnage *p);
void reinitialiserJoueur(Personnage *p);

void init_sous_menu_costume(GameState *gs, SousMenuCostume *sm, Personnage *cible);
void afficher_sous_menu_costume(GameState *gs, SousMenuCostume *sm);
void input_sous_menu_costume(GameState *gs, SousMenuCostume *sm);
void maj_sous_menu_costume(GameState *gs, SousMenuCostume *sm);
void liberer_sous_menu_costume(SousMenuCostume *sm);


void libererJoueur(Personnage *p);

#endif 
