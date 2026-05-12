#include "utils.h"
#include <stdio.h>
#include <string.h>

/* ── Texture helpers ─────────────────────────────────────────────────────── */

SDL_Texture *charger_image(SDL_Renderer *renderer, const char *chemin)
{
    SDL_Surface *surf = IMG_Load(chemin);
    if (!surf) {
        fprintf(stderr, "[WARN] IMG_Load(%s): %s\n", chemin, IMG_GetError());
        return NULL;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

SDL_Texture *creer_texture_couleur(SDL_Renderer *renderer,
                                   int w, int h,
                                   Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    SDL_Surface *surf = SDL_CreateRGBSurface(0, w, h, 32,
                                             0xFF000000, 0x00FF0000,
                                             0x0000FF00, 0x000000FF);
    SDL_FillRect(surf, NULL, SDL_MapRGBA(surf->format, r, g, b, a));
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_FreeSurface(surf);
    return tex;
}

/* ── Text rendering ──────────────────────────────────────────────────────── */

void afficher_texte(SDL_Renderer *renderer, TTF_Font *font,
                    const char *texte, SDL_Color couleur, int x, int y)
{
    if (!font || !texte) return;
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, texte, couleur);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_Rect dst = {x, y, surf->w, surf->h};
    SDL_RenderCopy(renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

void afficher_texte_centre(SDL_Renderer *renderer, TTF_Font *font,
                           const char *texte, SDL_Color couleur,
                           int zone_x, int zone_w, int y)
{
    if (!font || !texte) return;
    int tw, th;
    TTF_SizeUTF8(font, texte, &tw, &th);
    int x = zone_x + (zone_w - tw) / 2;
    afficher_texte(renderer, font, texte, couleur, x, y);
}

/* ── Draw rounded rect (pure SDL2 – no extra lib) ────────────────────────── */

void draw_rounded_rect(SDL_Renderer *r,
                       int x, int y, int w, int h,
                       Uint8 R, Uint8 G, Uint8 B, Uint8 A)
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, R, G, B, A);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(r, &rect);
    /* Border */
    SDL_SetRenderDrawColor(r, R > 30 ? R - 30 : 0,
                              G > 30 ? G - 30 : 0,
                              B > 30 ? B - 30 : 0, 255);
    SDL_RenderDrawRect(r, &rect);
}

/* ── Button helpers ──────────────────────────────────────────────────────── */

void init_bouton(GameState *gs, Bouton *b,
                 const char *img_n, const char *img_a,
                 int x, int y, int w, int h, const char *label)
{
    b->rect.x = x; b->rect.y = y;
    b->rect.w = w; b->rect.h = h;
    b->actif  = 0;
    strncpy(b->label, label, sizeof(b->label) - 1);
    b->label[sizeof(b->label) - 1] = '\0';

    /* Try to load image files; fall back to NULL (drawn with colours) */
    b->img_normal = img_n ? charger_image(gs->renderer, img_n) : NULL;
    b->img_actif  = img_a ? charger_image(gs->renderer, img_a) : NULL;
}

void afficher_bouton(SDL_Renderer *renderer, TTF_Font *font, Bouton *b)
{
    SDL_Texture *tex = b->actif ? b->img_actif : b->img_normal;

    if (tex) {
        SDL_RenderCopy(renderer, tex, NULL, &b->rect);
    } else {
        /* Fallback: draw coloured rectangle */
        if (b->actif)
            draw_rounded_rect(renderer,
                              b->rect.x, b->rect.y,
                              b->rect.w, b->rect.h,
                              0x00, 0xCC, 0xCC, 230);
        else
            draw_rounded_rect(renderer,
                              b->rect.x, b->rect.y,
                              b->rect.w, b->rect.h,
                              0x20, 0x80, 0xA0, 210);
    }

    /* Label centré */
    if (font && b->label[0]) {
        SDL_Color blanc = {255, 255, 255, 255};
        int tw, th;
        TTF_SizeUTF8(font, b->label, &tw, &th);
        int tx = b->rect.x + (b->rect.w - tw) / 2;
        int ty = b->rect.y + (b->rect.h - th) / 2;
        afficher_texte(renderer, font, b->label, blanc, tx, ty);
    }
}

int souris_sur_bouton(Bouton *b, int mx, int my)
{
    return (mx >= b->rect.x && mx <= b->rect.x + b->rect.w &&
            my >= b->rect.y && my <= b->rect.y + b->rect.h);
}

void liberer_bouton(Bouton *b)
{
    if (b->img_normal) { SDL_DestroyTexture(b->img_normal); b->img_normal = NULL; }
    if (b->img_actif)  { SDL_DestroyTexture(b->img_actif);  b->img_actif  = NULL; }
}

/* ── Background helpers ──────────────────────────────────────────────────── */

void init_background(GameState *gs, Background *bg,
                     const char *img_path, const char *music_path,
                     Niveau niveau)
{
    bg->niveau = niveau;
    bg->volume = 64;
    bg->mode   = 0;
    bg->rect.x = 0; bg->rect.y = 0;
    bg->rect.w = SCREEN_W; bg->rect.h = SCREEN_H;

    bg->image   = img_path   ? charger_image(gs->renderer, img_path)   : NULL;
    bg->musique = music_path ? Mix_LoadMUS(music_path) : NULL;

    if (!bg->image) {
        fprintf(stderr, "[INFO] Pas d'image background (%s) – fond coloré utilisé\n",
                img_path ? img_path : "null");
    }
    if (music_path && !bg->musique) {
        fprintf(stderr, "[WARN] Musique non chargée: %s\n", music_path);
    }
}

void afficher_background(SDL_Renderer *renderer, Background *bg)
{
    if (bg->image) {
        SDL_RenderCopy(renderer, bg->image, NULL, &bg->rect);
    } else {
        /* Gradient bleu/vert de fond selon niveau */
        switch (bg->niveau) {
            case NIVEAU_MENU:
                SDL_SetRenderDrawColor(renderer, 30, 80, 140, 255); break;
            case NIVEAU_OPTIONS:
            case NIVEAU_SAUVEGARDE:
            case NIVEAU_JOUEUR:
                SDL_SetRenderDrawColor(renderer, 20, 60, 100, 255); break;
            case NIVEAU_SCORES:
                SDL_SetRenderDrawColor(renderer, 50, 30, 100, 255); break;
            case NIVEAU_ENIGME:
                SDL_SetRenderDrawColor(renderer, 20, 20,  60, 255); break;
            default:
                SDL_SetRenderDrawColor(renderer, 30, 30,  50, 255); break;
        }
        SDL_RenderClear(renderer);
    }
}

void liberer_background(Background *bg)
{
    if (bg->image)   { SDL_DestroyTexture(bg->image); bg->image = NULL; }
    if (bg->musique) { Mix_FreeMusic(bg->musique);    bg->musique = NULL; }
}
