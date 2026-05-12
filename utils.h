#ifndef UTILS_H
#define UTILS_H

#include "types.h"

/* ── Texture helpers ─────────────────────────────────────────────── */
SDL_Texture *charger_image(SDL_Renderer *renderer, const char *chemin);
SDL_Texture *creer_texture_couleur(SDL_Renderer *renderer,
                                   int w, int h,
                                   Uint8 r, Uint8 g, Uint8 b, Uint8 a);

/* ── Text rendering ──────────────────────────────────────────────── */
void afficher_texte(SDL_Renderer *renderer, TTF_Font *font,
                    const char *texte, SDL_Color couleur, int x, int y);
void afficher_texte_centre(SDL_Renderer *renderer, TTF_Font *font,
                           const char *texte, SDL_Color couleur,
                           int zone_x, int zone_w, int y);

/* ── Button helpers ──────────────────────────────────────────────── */
void init_bouton(GameState *gs, Bouton *b,
                 const char *img_n, const char *img_a,
                 int x, int y, int w, int h, const char *label);
void afficher_bouton(SDL_Renderer *renderer, TTF_Font *font, Bouton *b);
int  souris_sur_bouton(Bouton *b, int mx, int my);
void liberer_bouton(Bouton *b);

/* ── Background helpers ──────────────────────────────────────────── */
void init_background(GameState *gs, Background *bg,
                     const char *img_path, const char *music_path,
                     Niveau niveau);
void afficher_background(SDL_Renderer *renderer, Background *bg);
void liberer_background(Background *bg);

/* ── Rounded rectangle (fallback drawing) ───────────────────────── */
void draw_rounded_rect(SDL_Renderer *r,
                       int x, int y, int w, int h,
                       Uint8 R, Uint8 G, Uint8 B, Uint8 A);

#endif /* UTILS_H */
