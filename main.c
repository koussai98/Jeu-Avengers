
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "utils.h"
#include "menu_principal.h"
#include "sous_menu_options.h"
#include "sous_menu_sauvegarde.h"
#include "sous_menu_joueur.h"
#include "sous_menu_scores.h"
#include "sous_menu_enigme.h"
#include "joueur.h"
#include "puzzle_bridge.h"
#include "campagne.h"
#include "champion_select.h"
#include "quiz.h"
#include "death_choice.h"


int init_sdl(GameState *gs)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 0;
    }
    if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG))) {
        fprintf(stderr, "IMG_Init: %s\n", IMG_GetError());
        return 0;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "Mix_OpenAudio: %s\n", Mix_GetError());
        /* non fatal – continue sans son */
    }
    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        return 0;
    }

    gs->window = SDL_CreateWindow(
        "Jeu 2D – Lot 1 : Joueur",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H, 0);
    if (!gs->window) {
        fprintf(stderr, "Window: %s\n", SDL_GetError());
        return 0;
    }

    gs->renderer = SDL_CreateRenderer(gs->window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!gs->renderer) {
        fprintf(stderr, "Renderer: %s\n", SDL_GetError());
        return 0;
    }

    gs->font       = TTF_OpenFont("assets/fonts/font.ttf", 20);
    gs->font_large = TTF_OpenFont("assets/fonts/font.ttf", 32);
    if (!gs->font || !gs->font_large)
        fprintf(stderr, "[WARN] Police absente : %s\n", TTF_GetError());

    gs->son_bref = Mix_LoadWAV("assets/sounds/click.wav");
    /* non fatal si absent */

    gs->continuer    = 1;
    gs->click        = 0;
    gs->motion       = 0;
    gs->mouse_x      = 0;
    gs->mouse_y      = 0;
    gs->backg.niveau = NIVEAU_MENU;
    memset(gs->input, 0, sizeof(gs->input));

    SDL_StartTextInput();
    
   gs->backg.image = IMG_LoadTexture(gs->renderer, "assets/background/bg1.png");
   if (!gs->backg.image) {
	fprintf(stderr, "Could not load PvP background: %s\n", IMG_GetError());
   }
    return 1;
}


void quitter_sdl(GameState *gs)
{
    if (gs->son_bref)   Mix_FreeChunk(gs->son_bref);
    if (gs->font)       TTF_CloseFont(gs->font);
    if (gs->font_large) TTF_CloseFont(gs->font_large);
    SDL_DestroyRenderer(gs->renderer);
    SDL_DestroyWindow(gs->window);
    SDL_StopTextInput();
    TTF_Quit();
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();
}


int main(int argc, char *argv[])
{
    GameState gs;
    (void)argc; (void)argv;
    memset(&gs, 0, sizeof(gs));

    if (!init_sdl(&gs)) return 1;

 
    MenuPrincipal      menu_p;
    SousMenuOptions    menu_opt;
    SousMenuSauvegarde menu_sauv;
    SousMenuJoueur     menu_joueur;
    SousMenuScores     menu_scores;
    SousMenuEnigme     menu_enigme;
    Personnage         joueur1, joueur2;
    SousMenuCostume    menu_c1, menu_c2;
    PuzzleContext      puzzle_ctx;
    Campagne           campagne;
    ChampionSelect     champ_sel;
    Quiz               quiz;
    DeathChoice        death_choice;
    int                quiz_active = 0;

    init_menu_principal      (&gs, &menu_p);
    init_sous_menu_options   (&gs, &menu_opt);
    init_sous_menu_sauvegarde(&gs, &menu_sauv);
    init_sous_menu_joueur    (&gs, &menu_joueur);
    init_sous_menu_scores    (&gs, &menu_scores);
    init_sous_menu_enigme    (&gs, &menu_enigme);
    initialiserJoueur1       (&gs, &joueur1);
    initialiserJoueur2       (&gs, &joueur2);
    init_sous_menu_costume   (&gs, &menu_c1, &joueur1);
    init_sous_menu_costume   (&gs, &menu_c2, &joueur2);
    campagne_init            (&gs, &campagne);
    champion_select_init     (&gs, &champ_sel);
    death_choice_init        (&gs, &death_choice);


    memset(&quiz, 0, sizeof(quiz));
    memset(&puzzle_ctx, 0, sizeof(puzzle_ctx));

    Niveau niveau_prec = NIVEAU_MENU;


    while (gs.continuer) {

        
        puzzle_ctx.g.frame_tick = SDL_GetTicks();


        if (gs.backg.niveau != niveau_prec) {
            Niveau old_prec = niveau_prec;
            Mix_HaltMusic();
            niveau_prec = gs.backg.niveau;

            if (gs.backg.niveau == NIVEAU_MENU && menu_p.backg.musique)
                Mix_PlayMusic(menu_p.backg.musique, -1);
            if (gs.backg.niveau == NIVEAU_OPTIONS && menu_opt.backg.musique)
                Mix_PlayMusic(menu_opt.backg.musique, -1);
            if (gs.backg.niveau == NIVEAU_SCORES) {
                menu_scores.etat    = SCORES_SAISIE;
                menu_scores.nom_len = 0;
                memset(menu_scores.nom_saisi, 0, MAX_NOM);
            }
            if (gs.backg.niveau == NIVEAU_SAUVEGARDE)
                menu_sauv.etat = ETAT_QUESTION;


            if (gs.backg.niveau == NIVEAU_PUZZLE) {
                puzzle_bridge_init(&puzzle_ctx, &gs);
            }

            if (gs.backg.niveau == NIVEAU_QUIZ) {
                if (quiz_active) quiz_free(&quiz);
                quiz_init(&gs, &quiz);
                quiz_active = 1;
            }

            if (gs.backg.niveau == NIVEAU_CAMPAGNE &&
                old_prec != NIVEAU_PUZZLE &&
                old_prec != NIVEAU_QUIZ &&
                old_prec != NIVEAU_DEATH_CHOICE) {
                campagne_reset(&gs, &campagne);
            }
        }


        switch (gs.backg.niveau) {

            case NIVEAU_MENU:
                input_menu_principal(&gs);
                break;

            case NIVEAU_OPTIONS:
                input_sous_menu_options(&gs);
                break;

            case NIVEAU_SAUVEGARDE:
                input_sous_menu_sauvegarde(&gs);
                break;

            case NIVEAU_JOUEUR:
                input_sous_menu_joueur(&gs);
                break;

            case NIVEAU_SCORES:
                input_sous_menu_scores(&gs, &menu_scores);
                break;

            case NIVEAU_ENIGME:
                input_sous_menu_enigme(&gs);
                break;

            case NIVEAU_COSTUME_J1:
                input_sous_menu_costume(&gs, &menu_c1);
                break;

            case NIVEAU_COSTUME_J2:
                input_sous_menu_costume(&gs, &menu_c2);
                break;


            case NIVEAU_JEU: {
                SDL_Event ev;
                while (SDL_PollEvent(&ev)) {
                    if (ev.type == SDL_QUIT) {
                        gs.continuer = 0;
                    } else if (ev.type == SDL_KEYDOWN) {
                        SDL_Keycode k = ev.key.keysym.sym;
                        if (k == SDLK_ESCAPE) {
                            gs.backg.niveau = NIVEAU_MENU;
                        } else {
                            gererToucheAppuyee(&joueur1, k);
                            gererToucheAppuyee(&joueur2, k);
                        }
                    } else if (ev.type == SDL_KEYUP) {
                        SDL_Keycode k = ev.key.keysym.sym;
                        gererToucheRelachee(&joueur1, k);
                        gererToucheRelachee(&joueur2, k);
                    }
                }
                break;
            }

            
            case NIVEAU_PUZZLE: {
                SDL_Event ev;
                while (SDL_PollEvent(&ev)) {
                    if (ev.type == SDL_QUIT) {
                        gs.continuer = 0;
                    } else {
                        puzzle_bridge_handle_event(&puzzle_ctx, &ev);
                    }
                }
                break;
            }

            case NIVEAU_CAMPAGNE:
                campagne_input(&gs, &campagne);
                break;

            case NIVEAU_CHAMPION:
                champion_select_input(&gs, &champ_sel);
                break;

            case NIVEAU_QUIZ:
                quiz_input(&gs, &quiz);
                break;

            case NIVEAU_DEATH_CHOICE:
                death_choice_input(&gs, &death_choice);
                break;

            default:
                break;
        }


        switch (gs.backg.niveau) {

            case NIVEAU_MENU:
                maj_menu_principal(&gs, &menu_p);
                break;

            case NIVEAU_OPTIONS:
                maj_sous_menu_options(&gs, &menu_opt);
                break;

            case NIVEAU_SAUVEGARDE:
                maj_sous_menu_sauvegarde(&gs, &menu_sauv);
                break;

            case NIVEAU_JOUEUR:
                maj_sous_menu_joueur(&gs, &menu_joueur);
                break;

            case NIVEAU_SCORES:
                maj_sous_menu_scores(&gs, &menu_scores);
                break;

            case NIVEAU_ENIGME:
                maj_sous_menu_enigme(&gs, &menu_enigme);
                break;

            case NIVEAU_COSTUME_J1:
                maj_sous_menu_costume(&gs, &menu_c1);
                break;

            case NIVEAU_COSTUME_J2:
                maj_sous_menu_costume(&gs, &menu_c2);
                break;


            case NIVEAU_JEU:
                mettreAJourJoueur(&joueur1);
                mettreAJourJoueur(&joueur2);

                verifierCollisionAttaque(&joueur1, &joueur2);
                verifierCollisionAttaque(&joueur2, &joueur1);

                if (!joueur1.est_vivant || !joueur2.est_vivant) {
                    gs.death_count++;
                    gs.puzzle_origin = NIVEAU_JEU;
                    gs.backg.niveau  = NIVEAU_DEATH_CHOICE;
                }
                break;


            case NIVEAU_PUZZLE:
                if (puzzle_ctx.result == 1) {

                    if (gs.puzzle_origin == NIVEAU_CAMPAGNE) {
                        campagne_revive(&campagne);
                        gs.backg.niveau = NIVEAU_CAMPAGNE;
                    } else {
                        reinitialiserJoueur(&joueur1);
                        reinitialiserJoueur(&joueur2);
                        gs.backg.niveau = NIVEAU_JEU;
                    }
                    puzzle_ctx.result = 0;

                } else if (puzzle_ctx.result == -1) {

                    if (gs.puzzle_origin == NIVEAU_CAMPAGNE) {
                        campagne.etape  = ETAPE_GAMEOVER;
                        gs.backg.niveau = NIVEAU_CAMPAGNE;
                    } else {
                        gs.backg.niveau = NIVEAU_MENU;
                    }
                    puzzle_ctx.result = 0;
                }
                break;

            case NIVEAU_CAMPAGNE:
                campagne_update(&gs, &campagne);
                break;

            case NIVEAU_CHAMPION:
                champion_select_update(&gs, &champ_sel);
                break;

            case NIVEAU_DEATH_CHOICE:
                death_choice_update(&gs, &death_choice);
                break;

            case NIVEAU_QUIZ:
                quiz_update(&gs, &quiz);
                if (quiz_est_termine(&quiz)) {
                    int res = quiz_resultat(&quiz);
                    if (res == QUIZ_REUSSI) {
                        if (gs.puzzle_origin == NIVEAU_CAMPAGNE) {
                            campagne_revive(&campagne);
                            gs.backg.niveau = NIVEAU_CAMPAGNE;
                        } else {
                            reinitialiserJoueur(&joueur1);
                            reinitialiserJoueur(&joueur2);
                            gs.backg.niveau = NIVEAU_JEU;
                        }
                    } else {
                        if (gs.puzzle_origin == NIVEAU_CAMPAGNE) {
                            campagne.etape  = ETAPE_GAMEOVER;
                            gs.backg.niveau = NIVEAU_CAMPAGNE;
                        } else {
                            gs.backg.niveau = NIVEAU_MENU;
                        }
                    }
                }
                break;

            default:
                break;
        }


        SDL_SetRenderDrawColor(gs.renderer, 0, 0, 0, 255);
        SDL_RenderClear(gs.renderer);

        switch (gs.backg.niveau) {

            case NIVEAU_MENU:
                afficher_menu_principal(&gs, &menu_p);
                break;

            case NIVEAU_OPTIONS:
                afficher_sous_menu_options(&gs, &menu_opt);
                break;

            case NIVEAU_SAUVEGARDE:
                afficher_sous_menu_sauvegarde(&gs, &menu_sauv);
                break;

            case NIVEAU_JOUEUR:
                afficher_sous_menu_joueur(&gs, &menu_joueur);
                break;

            case NIVEAU_SCORES:
                afficher_sous_menu_scores(&gs, &menu_scores);
                break;

            case NIVEAU_ENIGME:
                afficher_sous_menu_enigme(&gs, &menu_enigme);
                break;

            case NIVEAU_COSTUME_J1:
                afficher_sous_menu_costume(&gs, &menu_c1);
                break;

            case NIVEAU_COSTUME_J2:
                afficher_sous_menu_costume(&gs, &menu_c2);
                break;


            case NIVEAU_JEU: {
                
                if (gs.backg.image) {
        		SDL_RenderCopy(gs.renderer, gs.backg.image, NULL, NULL);
   		} else {
        		SDL_SetRenderDrawColor(gs.renderer, 30, 100, 50, 255);
        		SDL_RenderClear(gs.renderer);
    		}


                {
                    int ground_y = SOL_Y + SPRITE_RENDER_H;   /* = 580 */
                    SDL_Rect sol = { 0, ground_y, SCREEN_W, SCREEN_H - ground_y };
                    SDL_SetRenderDrawColor(gs.renderer, 80, 50, 20, 255);
                    SDL_RenderFillRect(gs.renderer, &sol);
                    SDL_SetRenderDrawColor(gs.renderer, 50, 30, 10, 255);
                    SDL_RenderDrawLine(gs.renderer, 0, ground_y, SCREEN_W, ground_y);
                }

                
                afficherJoueur(&gs, &joueur1);
                afficherJoueur(&gs, &joueur2);

                
                afficherUI_Joueur(&gs, &joueur1, 10, 10);
                afficherUI_Joueur(&gs, &joueur2, SCREEN_W - 195, 10);

                
                {
                    SDL_Color gris = {180, 180, 180, 180};
                    afficher_texte_centre(gs.renderer, gs.font,
                        "J1:Q/D+LShift+Espace+A   J2:Fleches+RShift+PavEntr+Pav0   Echap=Menu",
                        gris, 0, SCREEN_W, SCREEN_H - 20);
                }
                break;
            }


            case NIVEAU_PUZZLE:
                puzzle_bridge_draw(&puzzle_ctx);
                break;

            case NIVEAU_CAMPAGNE:
                campagne_draw(&gs, &campagne);
                break;

            case NIVEAU_CHAMPION:
                champion_select_draw(&gs, &champ_sel);
                break;

            case NIVEAU_QUIZ:
                quiz_draw(&gs, &quiz);
                break;

            case NIVEAU_DEATH_CHOICE:
                death_choice_draw(&gs, &death_choice);
                break;

            default:
                break;
        }

        SDL_RenderPresent(gs.renderer);
        SDL_Delay(16);   
    }


    liberer_menu_principal      (&menu_p);
    liberer_sous_menu_options   (&menu_opt);
    liberer_sous_menu_sauvegarde(&menu_sauv);
    liberer_sous_menu_joueur    (&menu_joueur);
    liberer_sous_menu_scores    (&menu_scores);
    liberer_sous_menu_enigme    (&menu_enigme);
    libererJoueur               (&joueur1);
    libererJoueur               (&joueur2);
    liberer_sous_menu_costume   (&menu_c1);
    liberer_sous_menu_costume   (&menu_c2);
    puzzle_bridge_free          (&puzzle_ctx);
    campagne_liberer            (&campagne);
    champion_select_free        (&champ_sel);
    death_choice_free           (&death_choice);
    if (quiz_active) quiz_free(&quiz);
    quitter_sdl(&gs);
    return 0;
}
