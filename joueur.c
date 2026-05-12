

#include "joueur.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>   


const SDL_Rect FRAMES_IDLE_RECTS[4] = {
    { 314, IDLE_Y, 147, IDLE_H },   
    { 524, IDLE_Y, 147, IDLE_H },   
    { 728, IDLE_Y, 147, IDLE_H },   
    { 940, IDLE_Y, 147, IDLE_H },   
};


const SDL_Rect FRAMES_WALK_RECTS[7] = {
    {  183, WALK_Y, 148, WALK_H },  
    {  364, WALK_Y, 147, WALK_H },  
    {  540, WALK_Y, 144, WALK_H },  
    {  698, WALK_Y, 143, WALK_H },  
    {  865, WALK_Y, 139, WALK_H },  
    { 1030, WALK_Y, 144, WALK_H },  
    { 1209, WALK_Y, 146, WALK_H },  
};


const SDL_Rect FRAMES_ATTACK_RECTS[4] = {
    {  273, ATTACK_Y, 156, ATTACK_H },  
    {  507, ATTACK_Y, 187, ATTACK_H },  
    {  756, ATTACK_Y, 173, ATTACK_H },  
    { 1020, ATTACK_Y, 162, ATTACK_H },  
};


const SDL_Rect FRAMES_JUMP_RECTS[3] = {
    {  389, JUMP_Y, 153, JUMP_H },  
    {  662, JUMP_Y, 142, JUMP_H },  
    {  910, JUMP_Y, 148, JUMP_H },  
};



const char *NOM_COSTUMES[NB_COSTUMES] = {
    "Costume Classique",
    "Costume Ninja",
    "Costume Robot"
};

const SDL_Color COULEURS_J1[NB_COSTUMES] = {
    { 70, 130, 220, 255 },   
    { 50, 180,  80, 255 },   
    {180,  60,  60, 255 }    
};

const SDL_Color COULEURS_J2[NB_COSTUMES] = {
    {220, 140,  50, 255 },   
    {160,  60, 200, 255 },   
    {220, 220,  50, 255 }    
};


const int NB_FRAMES_PAR_ETAT[5] = {
    FRAMES_IDLE, FRAMES_WALK, FRAMES_RUN, FRAMES_JUMP, FRAMES_ATTACK
};

const int VITESSE_PAR_ETAT[5] = {
    ANIM_SPEED_IDLE, ANIM_SPEED_WALK, ANIM_SPEED_RUN,
    ANIM_SPEED_JUMP, ANIM_SPEED_ATTACK
};


/**
 * @brief Charge une spritesheet et applique la transparence.
 *
 * Charge une image SDL, convertit son format
 * et crée une texture exploitable par le renderer.
 *
 * @param renderer Renderer SDL utilisé.
 * @param chemin Chemin vers la spritesheet.
 * @return SDL_Texture* Texture créée ou NULL en cas d'erreur.
 */
SDL_Texture *charger_sprite_knight(SDL_Renderer *renderer, const char *chemin)
{
    SDL_Surface *src = IMG_Load(chemin);
    if (!src) {
        fprintf(stderr, "[WARN] charger_sprite_knight: %s → %s\n",
                chemin, IMG_GetError());
        return NULL;
    }

    
    SDL_Surface *rgba = SDL_ConvertSurfaceFormat(src, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(src);
    if (!rgba) {
        fprintf(stderr, "[WARN] ConvertSurface: %s\n", SDL_GetError());
        return NULL;
    }

    
    SDL_LockSurface(rgba);
    Uint32 *pixels = (Uint32 *)rgba->pixels;
    int total = rgba->w * rgba->h;
    int i;
    for (i = 0; i < total; i++) {
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixels[i], rgba->format, &r, &g, &b, &a);

        int dr = abs((int)r - SPRITE_COLORKEY_R);
        int dg = abs((int)g - SPRITE_COLORKEY_G);
        int db = abs((int)b - SPRITE_COLORKEY_B);

        if (dr <= SPRITE_COLORKEY_TOL &&
            dg <= SPRITE_COLORKEY_TOL &&
            db <= SPRITE_COLORKEY_TOL &&
            (int)r > (int)b - 80)
        {
            pixels[i] = SDL_MapRGBA(rgba->format, 0, 0, 0, 0);
        }
    }
    SDL_UnlockSurface(rgba);

    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, rgba);
    SDL_FreeSurface(rgba);

    if (!tex) {
        fprintf(stderr, "[WARN] CreateTexture: %s\n", SDL_GetError());
        return NULL;
    }

    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    fprintf(stderr, "[INFO] Sprite chargée : %s\n", chemin);
    return tex;
}


/**
 * @brief Initialise les données de base d'un personnage.
 *
 * Configure la position, les animations,
 * les statistiques et les contrôles.
 *
 * @param gs Etat global du jeu.
 * @param p Personnage à initialiser.
 * @param numero Numéro du joueur.
 * @param nom Nom du personnage.
 * @param x Position horizontale initiale.
 * @param touches Touches du joueur.
 * @param couleur Couleur de remplacement.
 */
void init_personnage_base(GameState *gs,
                          Personnage *p,
                          NumeroJoueur numero,
                          const char *nom,
                          float x,
                          TouchesJoueur touches,
                          SDL_Color couleur)
{
    
    strncpy(p->nom, nom, NOM_MAX - 1);
    p->nom[NOM_MAX - 1] = '\0';
    p->numero      = numero;
    p->costume_idx = 0;

    p->x         = x;
    p->y         = (float)SOL_Y;
    p->vy        = 0.0f;
    p->au_sol    = 1;
    p->direction = DIR_DROITE;

    p->etat          = ETAT_IDLE;
    p->attaque_timer = 0;


    {
        int i;
        for (i = 0; i < 5; i++) {
            p->anims[i].frame_courante    = 0;
            p->anims[i].nb_frames         = NB_FRAMES_PAR_ETAT[i];
            p->anims[i].timer             = 0;
            p->anims[i].vitesse           = VITESSE_PAR_ETAT[i];
            p->anims[i].ligne_spritesheet = i;
        }
    }


    {
        char chemin[128];
        snprintf(chemin, sizeof(chemin),
                 "assets/images/joueur%d.png", (int)numero);
        p->sprite_sheet = charger_sprite_knight(gs->renderer, chemin);
    }
    p->couleur_placeholder = couleur;


    p->vies       = VIES_INITIALES;
    p->score      = 0;
    p->est_vivant = 1;


    p->touches       = touches;
    p->touche_gauche = 0;
    p->touche_droite = 0;
    p->touche_courir = 0;
    p->attaque_cooldown = 0;
    p->world_w_max      = 0;  /* 0 = clamp to SCREEN_W */
    p->use_grid_animation = 0;
}


/**
 * @brief Initialise le joueur 1.
 *
 * Configure les touches et paramètres du premier joueur.
 *
 * @param gs Etat global du jeu.
 * @param p Structure du joueur.
 */
void initialiserJoueur1(GameState *gs, Personnage *p)
{
    TouchesJoueur t;
    t.gauche   = SDLK_q;
    t.droite   = SDLK_d;
    t.courir   = SDLK_LSHIFT;
    t.sauter   = SDLK_SPACE;
    t.attaquer = SDLK_a;

    init_personnage_base(gs, p, JOUEUR_1, "Joueur 1", 100.0f, t, COULEURS_J1[0]);
}


/**
 * @brief Initialise le joueur 2.
 *
 * Configure les touches et le champion du second joueur.
 *
 * @param gs Etat global du jeu.
 * @param p Structure du joueur.
 */
void initialiserJoueur2(GameState *gs, Personnage *p)
{
    TouchesJoueur t;
    t.gauche   = SDLK_LEFT;
    t.droite   = SDLK_RIGHT;
    t.courir   = SDLK_RSHIFT;
    t.sauter   = SDLK_KP_ENTER;
    t.attaquer = SDLK_KP_0;

    init_personnage_base(gs, p, JOUEUR_2, "Captain America", SCREEN_W - 150.0f, t, COULEURS_J2[0]);


    joueur_set_champion(gs, p, 3);
    p->direction = DIR_GAUCHE;
}


/**
 * @brief Affiche un personnage à l'écran.
 *
 * Gère les animations, les sprites,
 * les effets visuels et la barre de vie.
 *
 * @param gs Etat global du jeu.
 * @param p Joueur à afficher.
 */
void afficherJoueur(GameState *gs, Personnage *p)
{
    SDL_Renderer   *r    = gs->renderer;
    int             px   = (int)p->x - gs->camera_x; /*écran = monde - camera */
    int             py   = (int)p->y;
    SDL_Rect        dst  = { px, py, SPRITE_RENDER_W, SPRITE_RENDER_H };
    SDL_RendererFlip flip = (p->direction == DIR_GAUCHE)
                            ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    if (p->sprite_sheet) {

        SDL_Rect src_rect;
        const SDL_Rect *src = &src_rect;

        if (p->use_grid_animation) {

            int sheet_w = 0, sheet_h = 0;
            SDL_QueryTexture(p->sprite_sheet, NULL, NULL, &sheet_w, &sheet_h);
            int cols    = 6;
            int rows    = 4;
            int frame_w = sheet_w / cols;
            int frame_h = sheet_h / rows;

            int row;
            int frames_in_row;
            switch (p->etat) {
                case ETAT_IDLE:   row = 0; frames_in_row = 4; break;
                case ETAT_WALK:
                case ETAT_RUN:    row = 1; frames_in_row = 6; break;
                case ETAT_ATTACK: row = 2; frames_in_row = 4; break;
                case ETAT_JUMP:   row = 3; frames_in_row = 4; break;
                default:          row = 0; frames_in_row = 4; break;
            }
            int frame_idx = p->anims[p->etat].frame_courante % frames_in_row;
            if (frame_idx < 0) frame_idx = 0;

            src_rect.x = frame_idx * frame_w;
            src_rect.y = row       * frame_h;
            src_rect.w = frame_w;
            src_rect.h = frame_h;
        } else {

            const SDL_Rect *frame_table = FRAMES_IDLE_RECTS;
            int             frame_idx   = p->anims[p->etat].frame_courante;
            int             max_frames  = FRAMES_IDLE;

            switch (p->etat) {
                case ETAT_IDLE:
                    frame_table = FRAMES_IDLE_RECTS;
                    max_frames  = FRAMES_IDLE;
                    break;
                case ETAT_WALK:
                case ETAT_RUN:
                    frame_table = FRAMES_WALK_RECTS;
                    max_frames  = FRAMES_WALK;
                    break;
                case ETAT_ATTACK:
                    frame_table = FRAMES_ATTACK_RECTS;
                    max_frames  = FRAMES_ATTACK;
                    break;
                case ETAT_JUMP:
                    frame_table = FRAMES_JUMP_RECTS;
                    max_frames  = FRAMES_JUMP;
                    break;
                default:
                    break;
            }

            if (frame_idx >= max_frames) frame_idx = max_frames - 1;
            if (frame_idx < 0)           frame_idx = 0;
            src = &frame_table[frame_idx];
        }


        Uint8 alpha = (p->etat == ETAT_JUMP) ? 210 : 255;
        SDL_SetTextureAlphaMod(p->sprite_sheet, alpha);

        SDL_RenderCopyEx(r, p->sprite_sheet, src, &dst, 0.0, NULL, flip);
        SDL_SetTextureAlphaMod(p->sprite_sheet, 255);

        
        if (p->etat == ETAT_ATTACK) {
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 255, 60, 60, 70);
            SDL_RenderFillRect(r, &dst);
        }

    } else {
        
        SDL_Color c = p->couleur_placeholder;

        SDL_SetRenderDrawColor(r, c.r, c.g, c.b, 255);
        SDL_RenderFillRect(r, &dst);

        
        SDL_SetRenderDrawColor(r,
            (Uint8)SDL_min(255, c.r + 60),
            (Uint8)SDL_min(255, c.g + 60),
            (Uint8)SDL_min(255, c.b + 60), 255);
        {
            SDL_Rect tete = { px + 24, py - 20, 28, 20 };
            SDL_RenderFillRect(r, &tete);
        }

        
        SDL_SetRenderDrawColor(r, 255, 255, 255, 200);
        {
            int ax = (p->direction == DIR_DROITE)
                     ? px + SPRITE_RENDER_W - 8 : px + 2;
            SDL_Rect fleche = { ax, py + SPRITE_RENDER_H / 2 - 4, 6, 8 };
            SDL_RenderFillRect(r, &fleche);
        }

        if (p->etat == ETAT_ATTACK) {
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 255, 50, 50, 100);
            SDL_RenderFillRect(r, &dst);
        }
    }

    
    {
        int barre_w = SPRITE_RENDER_W;
        int vie_w   = (p->vies * barre_w) / VIES_INITIALES;
        int barre_x = px;
        int barre_y = py - 10;

        SDL_SetRenderDrawColor(r, 50, 50, 50, 255);
        {
            SDL_Rect fond_b = { barre_x, barre_y, barre_w, 5 };
            SDL_RenderFillRect(r, &fond_b);
        }

        {
            Uint8 rv = (p->vies <= 1) ? 220 : (p->vies == 2) ? 220 :  50;
            Uint8 gv = (p->vies <= 1) ?  50 : (p->vies == 2) ? 140 : 200;
            SDL_SetRenderDrawColor(r, rv, gv, 50, 255);
            SDL_Rect vie_r = { barre_x, barre_y, vie_w, 5 };
            SDL_RenderFillRect(r, &vie_r);
        }
    }
}


/**
 * @brief Affiche l'interface d'un joueur.
 *
 * Affiche le nom, le score et les vies restantes.
 *
 * @param gs Etat global du jeu.
 * @param p Joueur concerné.
 * @param ui_x Position X de l'interface.
 * @param ui_y Position Y de l'interface.
 */
void afficherUI_Joueur(GameState *gs, Personnage *p, int ui_x, int ui_y)
{
    SDL_Renderer *r   = gs->renderer;
    TTF_Font     *fnt = gs->font;

    SDL_Color blanc = {255, 255, 255, 255};
    SDL_Color jaune = {255, 220,  50, 255};
    SDL_Color rouge = {220,  60,  60, 255};

    
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 150);
    {
        SDL_Rect fond = { ui_x - 6, ui_y - 4, 190, 68 };
        SDL_RenderFillRect(r, &fond);
    }

    
    afficher_texte(r, fnt, p->nom, blanc, ui_x, ui_y);

    
    {
        int i;
        for (i = 0; i < VIES_INITIALES; i++) {
            SDL_Color c = (i < p->vies) ? rouge : (SDL_Color){60, 60, 60, 255};
            SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
            
            SDL_Rect hg = { ui_x + i * 24,     ui_y + 24, 9, 9 };
            SDL_Rect hd = { ui_x + i * 24 + 9, ui_y + 24, 9, 9 };
            SDL_Rect ba = { ui_x + i * 24 + 2, ui_y + 31, 14, 10 };
            SDL_RenderFillRect(r, &hg);
            SDL_RenderFillRect(r, &hd);
            SDL_RenderFillRect(r, &ba);
        }
    }

    
    {
        char score_str[32];
        snprintf(score_str, sizeof(score_str), "Score: %d", p->score);
        afficher_texte(r, fnt, score_str, jaune, ui_x, ui_y + 46);
    }
}

/**
 * @brief Change l'état du personnage.
 *
 * Réinitialise l'animation liée au nouvel état.
 *
 * @param p Personnage concerné.
 * @param nouvel_etat Nouvel état du joueur.
 */
void changerEtat(Personnage *p, EtatJoueur nouvel_etat)
{
    if (p->etat == nouvel_etat && nouvel_etat != ETAT_ATTACK) return;

    p->etat = nouvel_etat;
    p->anims[nouvel_etat].frame_courante = 0;
    p->anims[nouvel_etat].timer          = 0;

    if (nouvel_etat == ETAT_ATTACK)
        p->attaque_timer = DUREE_ATTAQUE;
}



/**
 * @brief Met à jour l'animation du personnage.
 *
 * Gère les changements de frames et les boucles d'animation.
 *
 * @param p Personnage concerné.
 */
void mettreAJourAnimation(Personnage *p)
{
    Animation *anim = &p->anims[p->etat];

    anim->timer++;
    if (anim->timer >= anim->vitesse) {
        anim->timer = 0;
        anim->frame_courante++;

        if (anim->frame_courante >= anim->nb_frames) {
            if (p->etat == ETAT_ATTACK) {
                anim->frame_courante = 0;
                changerEtat(p, p->au_sol ? ETAT_IDLE : ETAT_JUMP);
            } else {
                anim->frame_courante = 0;   /* boucle */
            }
        }
    }
}


/**
 * @brief Gère l'appui sur une touche clavier.
 *
 * Active les déplacements, le saut ou les attaques.
 *
 * @param p Personnage concerné.
 * @param touche Touche pressée.
 */
void gererToucheAppuyee(Personnage *p, SDL_Keycode touche)
{
    if (touche == p->touches.gauche)  p->touche_gauche = 1;
    if (touche == p->touches.droite)  p->touche_droite = 1;
    if (touche == p->touches.courir)  p->touche_courir = 1;

    
    if (touche == p->touches.sauter && p->au_sol) {
        p->vy     = VITESSE_SAUT;
        p->au_sol = 0;
        changerEtat(p, ETAT_JUMP);
    }

    
    if (touche == p->touches.attaquer) {
        changerEtat(p, ETAT_ATTACK);
    }
}


/**
 * @brief Gère le relâchement des touches clavier.
 *
 * Désactive les actions liées aux déplacements.
 *
 * @param p Personnage concerné.
 * @param touche Touche relâchée.
 */
void gererToucheRelachee(Personnage *p, SDL_Keycode touche)
{
    if (touche == p->touches.gauche)  p->touche_gauche = 0;
    if (touche == p->touches.droite)  p->touche_droite = 0;
    if (touche == p->touches.courir)  p->touche_courir = 0;
}


/**
 * @brief Met à jour le joueur à chaque frame.
 *
 * Gère les déplacements, la gravité,
 * les collisions et les états du personnage.
 *
 * @param p Joueur à mettre à jour.
 */
void mettreAJourJoueur(Personnage *p)
{
    int vx = 0;

    
    if (p->etat != ETAT_ATTACK || !p->au_sol) {
        if (p->etat != ETAT_ATTACK) {
            int vitesse = p->touche_courir ? VITESSE_COURSE : VITESSE_MARCHE;
            if (p->touche_droite) { vx += vitesse; p->direction = DIR_DROITE; }
            if (p->touche_gauche) { vx -= vitesse; p->direction = DIR_GAUCHE; }
        }
    }

    p->x += (float)vx;

    
    {
        int max_x = (p->world_w_max > 0 ? p->world_w_max : SCREEN_W) - SPRITE_RENDER_W;
        if (p->x < 0)         p->x = 0;
        if (p->x > (float)max_x) p->x = (float)max_x;
    }

    
    if (!p->au_sol) p->vy += GRAVITE;

    
    p->y += p->vy;

    
    if (p->y >= (float)SOL_Y) {
        p->y      = (float)SOL_Y;
        p->vy     = 0.0f;
        p->au_sol = 1;
    }

    
    if (p->attaque_timer > 0) p->attaque_timer--;

    
    if (p->attaque_cooldown > 0) p->attaque_cooldown--;

    
    if (p->attaque_timer > 0) {
        
    } else if (!p->au_sol) {
        changerEtat(p, ETAT_JUMP);
    } else if (vx != 0) {
        changerEtat(p, p->touche_courir ? ETAT_RUN : ETAT_WALK);
    } else {
        changerEtat(p, ETAT_IDLE);
    }

    
    mettreAJourAnimation(p);
}


/**
 * @brief Vérifie la collision d'une attaque.
 *
 * Détecte si une attaque touche un autre joueur.
 *
 * @param attaquant Joueur attaquant.
 * @param cible Joueur ciblé.
 * @return int 1 si collision détectée, sinon 0.
 */
int verifierCollisionAttaque(Personnage *attaquant, Personnage *cible)
{
    
    int hit_x, hit_y, hit_w, hit_h;
    
    int cib_x, cib_y, cib_w, cib_h;

    if (attaquant->etat != ETAT_ATTACK)   return 0;
    if (attaquant->attaque_timer <= 0)    return 0;
    if (!cible->est_vivant)               return 0;
    if (cible->attaque_cooldown > 0)      return 0;

    hit_y = (int)attaquant->y;
    hit_h = HITBOX_H;

    if (attaquant->direction == DIR_DROITE) {
        
        hit_x = (int)attaquant->x + SPRITE_RENDER_W;
        hit_w = HITBOX_REACH;
    } else {
        
        hit_x = (int)attaquant->x - HITBOX_REACH;
        hit_w = HITBOX_REACH;
    }

    
    cib_x = (int)cible->x;
    cib_y = (int)cible->y;
    cib_w = SPRITE_RENDER_W;
    cib_h = SPRITE_RENDER_H;

    
    if (hit_x           < cib_x + cib_w &&
        hit_x + hit_w   > cib_x          &&
        hit_y           < cib_y + cib_h  &&
        hit_y + hit_h   > cib_y) {

        
        perdreVie(cible);
        ajouterScore(attaquant, SCORE_ATTAQUE);

        
        cible->attaque_cooldown = INVINCIBILITE_FRAMES;

        return 1;   /* coup infligé */
    }

    return 0;   /* pas de collision */
}


/**
 * @brief Ajoute des points au score du joueur.
 *
 * @param p Joueur concerné.
 * @param points Nombre de points à ajouter.
 */
void ajouterScore(Personnage *p, int points)
{
    p->score += points;
}


/**
 * @brief Retire une vie au joueur.
 *
 * Met à jour l'état du joueur si ses vies atteignent zéro.
 *
 * @param p Joueur concerné.
 */
void perdreVie(Personnage *p)
{
    if (p->vies > 0) p->vies--;
    if (p->vies == 0) p->est_vivant = 0;
}


/**
 * @brief Réinitialise un joueur.
 *
 * Replace le joueur dans un état jouable après une défaite.
 *
 * @param p Joueur à réinitialiser.
 */
void reinitialiserJoueur(Personnage *p)
{
    int i;
    p->y          = (float)SOL_Y;
    p->vy         = 0.0f;
    p->au_sol     = 1;
    p->etat       = ETAT_IDLE;
    p->est_vivant = 1;
    if (p->vies <= 0) p->vies = 1;

    p->attaque_timer    = 0;
    p->touche_gauche    = 0;
    p->touche_droite    = 0;
    p->touche_courir    = 0;
    p->attaque_cooldown = 0;   /* plus d'invincibilité après résurrection */

    for (i = 0; i < 5; i++) {
        p->anims[i].frame_courante = 0;
        p->anims[i].timer          = 0;
    }
}


/**
 * @brief Initialise le sous-menu des costumes.
 *
 * Crée les boutons de sélection des costumes.
 *
 * @param gs Etat global du jeu.
 * @param sm Sous-menu de costumes.
 * @param cible Joueur ciblé.
 */
void init_sous_menu_costume(GameState *gs, SousMenuCostume *sm, Personnage *cible)
{
    int i;
    int cx      = SCREEN_W / 2;
    int start_y = 220;
    int btn_w   = 240, btn_h = 50, gap = 18;

    sm->joueur_cible   = cible;
    sm->costume_survol = -1;

    for (i = 0; i < NB_COSTUMES; i++) {
        init_bouton(gs, &sm->boutons[i], NULL, NULL,
                    cx - btn_w / 2,
                    start_y + i * (btn_h + gap),
                    btn_w, btn_h,
                    NOM_COSTUMES[i]);
    }


    init_bouton(gs, &sm->boutons[NB_COSTUMES], NULL, NULL,
                cx - 70,
                start_y + NB_COSTUMES * (btn_h + gap) + 10,
                140, 45, "Retour");
}

/**
 * @brief Affiche le sous-menu des costumes.
 *
 * @param gs Etat global du jeu.
 * @param sm Sous-menu à afficher.
 */
void afficher_sous_menu_costume(GameState *gs, SousMenuCostume *sm)
{
    int i;
    SDL_Renderer *r = gs->renderer;

    SDL_SetRenderDrawColor(r, 15, 15, 40, 255);
    SDL_RenderClear(r);

    draw_rounded_rect(r, SCREEN_W/2 - 200, 100, 400, 390, 25, 25, 70, 240);

    {
        SDL_Color jaune = {255, 220, 50, 255};
        SDL_Color blanc = {255, 255, 255, 255};
        SDL_Color gris  = {160, 160, 160, 255};

        afficher_texte_centre(r, gs->font_large,
                              "CHOIX DU COSTUME", jaune, 0, SCREEN_W, 120);

        {
            char sous_titre[64];
            snprintf(sous_titre, sizeof(sous_titre),
                     "Joueur : %s", sm->joueur_cible->nom);
            afficher_texte_centre(r, gs->font, sous_titre, gris, 0, SCREEN_W, 160);
        }

        {
            char actif_str[64];
            snprintf(actif_str, sizeof(actif_str),
                     "Actuel : %s", NOM_COSTUMES[sm->joueur_cible->costume_idx]);
            afficher_texte_centre(r, gs->font, actif_str, blanc, 0, SCREEN_W, 185);
        }
    }

    for (i = 0; i < NB_BTN_COSTUME; i++) {
        if (i < NB_COSTUMES && i == sm->joueur_cible->costume_idx) {
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 50, 200, 80, 60);
            SDL_RenderFillRect(r, &sm->boutons[i].rect);
        }
        afficher_bouton(r, gs->font, &sm->boutons[i]);
    }
}

/**
 * @brief Gère les événements du sous-menu costume.
 *
 * @param gs Etat global du jeu.
 * @param sm Sous-menu concerné.
 */
void input_sous_menu_costume(GameState *gs, SousMenuCostume *sm)
{
    SDL_Event ev;
    (void)sm;
    gs->click  = 0;
    gs->motion = 0;

    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) {
            gs->continuer = 0;
        } else if (ev.type == SDL_MOUSEBUTTONDOWN &&
                   ev.button.button == SDL_BUTTON_LEFT) {
            gs->click   = 1;
            gs->mouse_x = ev.button.x;
            gs->mouse_y = ev.button.y;
        } else if (ev.type == SDL_MOUSEMOTION) {
            gs->motion  = 1;
            gs->mouse_x = ev.motion.x;
            gs->mouse_y = ev.motion.y;
        } else if (ev.type == SDL_KEYDOWN &&
                   ev.key.keysym.sym == SDLK_ESCAPE) {
            gs->backg.niveau = NIVEAU_MENU;
        }
    }
}

/**
 * @brief Met à jour le sous-menu des costumes.
 *
 * Gère les interactions utilisateur et le changement de costume.
 *
 * @param gs Etat global du jeu.
 * @param sm Sous-menu concerné.
 */
void maj_sous_menu_costume(GameState *gs, SousMenuCostume *sm)
{
    int i;
    Personnage *p = sm->joueur_cible;
    sm->costume_survol = -1;

    if (gs->motion) {
        for (i = 0; i < NB_BTN_COSTUME; i++) {
            int was = sm->boutons[i].actif;
            sm->boutons[i].actif = souris_sur_bouton(&sm->boutons[i],
                                                      gs->mouse_x, gs->mouse_y);
            if (!was && sm->boutons[i].actif) {
                if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
                if (i < NB_COSTUMES) sm->costume_survol = i;
            }
        }
    }

    if (gs->click) {
        for (i = 0; i < NB_COSTUMES; i++) {
            if (souris_sur_bouton(&sm->boutons[i], gs->mouse_x, gs->mouse_y)) {
                char chemin[128];

                p->costume_idx         = i;
                p->couleur_placeholder = (p->numero == JOUEUR_1)
                                         ? COULEURS_J1[i] : COULEURS_J2[i];

                
                if (p->sprite_sheet) {
                    SDL_DestroyTexture(p->sprite_sheet);
                    p->sprite_sheet = NULL;
                }

                snprintf(chemin, sizeof(chemin),
                         "assets/images/joueur%d_costume%d.png",
                         (int)p->numero, i);
                p->sprite_sheet = charger_sprite_knight(gs->renderer, chemin);

                
                if (!p->sprite_sheet) {
                    snprintf(chemin, sizeof(chemin),
                             "assets/images/joueur%d.png", (int)p->numero);
                    p->sprite_sheet = charger_sprite_knight(gs->renderer, chemin);
                }

                if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
            }
        }

        if (souris_sur_bouton(&sm->boutons[NB_COSTUMES],
                              gs->mouse_x, gs->mouse_y)) {
            gs->backg.niveau = NIVEAU_MENU;
            if (gs->son_bref) Mix_PlayChannel(-1, gs->son_bref, 0);
        }
    }
}

/**
 * @brief Libère les ressources du sous-menu costume.
 *
 * @param sm Sous-menu concerné.
 */
void liberer_sous_menu_costume(SousMenuCostume *sm)
{
    int i;
    for (i = 0; i < NB_BTN_COSTUME; i++)
        liberer_bouton(&sm->boutons[i]);
}



/**
 * @brief Libère les ressources du joueur.
 *
 * Détruit les textures associées au personnage.
 *
 * @param p Joueur concerné.
 */
void libererJoueur(Personnage *p)
{
    if (p->sprite_sheet) {
        SDL_DestroyTexture(p->sprite_sheet);
        p->sprite_sheet = NULL;
    }
}


/**
 * @brief Retourne le chemin d'une spritesheet de champion.
 *
 * @param champion_id Identifiant du champion.
 * @return const char* Chemin vers la spritesheet.
 */
const char *champion_sprite_path(int champion_id)
{
    switch (champion_id) {
        case 0: return "assets/champions sprite sheet/thor.png";        /* CHAMPION_THOR */
        case 1: return "assets/champions sprite sheet/hulk.png";        /* CHAMPION_HULK */
        case 2: return "assets/champions sprite sheet/ironman.png";     /* CHAMPION_IRONMAN */
        case 3: return "assets/champions sprite sheet/capamerica.png";  /* CHAMPION_CAPAMERICA */
        default: return "assets/champions sprite sheet/thor.png";
    }
}


/**
 * @brief Change le champion associé à un joueur.
 *
 * Charge une nouvelle spritesheet et réinitialise les animations.
 *
 * @param gs Etat global du jeu.
 * @param p Joueur concerné.
 * @param champion_id Identifiant du champion.
 */
void joueur_set_champion(GameState *gs, Personnage *p, int champion_id)
{
    const char *path = champion_sprite_path(champion_id);
    SDL_Texture *new_tex = charger_image(gs->renderer, path);

    if (new_tex) {
        if (p->sprite_sheet) {
            SDL_DestroyTexture(p->sprite_sheet);
        }
        p->sprite_sheet = new_tex;

        p->use_grid_animation = 1;
        p->anims[ETAT_IDLE]  .nb_frames = 4;
        p->anims[ETAT_WALK]  .nb_frames = 6;
        p->anims[ETAT_RUN]   .nb_frames = 6;
        p->anims[ETAT_JUMP]  .nb_frames = 4;
        p->anims[ETAT_ATTACK].nb_frames = 4;
        int i;
        for (i = 0; i < 5; i++) {
            p->anims[i].frame_courante = 0;
            p->anims[i].timer          = 0;
        }
        fprintf(stderr, "[CHAMPION] loaded sprite: %s (grid mode 6x4, frames 4-6-4-4)\n", path);
    } else {
        fprintf(stderr, "[CHAMPION] FAILED to load %s -- keeping previous sheet\n", path);
    }
}
