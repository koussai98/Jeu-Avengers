#define _USE_MATH_DEFINES
#include "menu_puzzle.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE   44100
#define CHUNK_SIZE    2048
#define PUZZLE_COUNT  5
#define GRID_SIZE     3
#define PIECE_SIZE    80
#define OPTION_PIECES 3
#define TIME_LIMIT_MS 60000

int           puzzle_seeded         = 0;
int           current_puzzle        = 0;
PuzzlePiece   grid[GRID_SIZE][GRID_SIZE];
OptionPiece   options_puzzle[OPTION_PIECES];
int           missing_px            = 0;
int           missing_py            = 0;
int           puzzle_solved         = 0;
int           puzzle_failed         = 0;
Button        btn_back_puzzle;
int           hover_played_puzzle   = 0;
Uint32        start_time_puzzle     = 0;
float         timer_progress_puzzle = 0.0f;
float         result_scale_puzzle   = 1.0f;
int           result_fade_puzzle    = 0;

SDL_Texture  *puzzle_textures[PUZZLE_COUNT];
int           option_src_char[OPTION_PIECES];
int           option_src_x[OPTION_PIECES];
int           option_src_y[OPTION_PIECES];
int           consecutive_correct   = 0;
int           consecutive_wrong     = 0;
char          bonus_msg[64];
char          current_puzzle_name[32];
int           puzzle_timed_out      = 0;
int           game_over             = 0;

void write_le16(Uint8 *buf, Uint16 val)
{
    buf[0] =  val        & 0xFF;
    buf[1] = (val >>  8) & 0xFF;
}

void write_le32(Uint8 *buf, Uint32 val)
{
    buf[0] =  val        & 0xFF;
    buf[1] = (val >>  8) & 0xFF;
    buf[2] = (val >> 16) & 0xFF;
    buf[3] = (val >> 24) & 0xFF;
}

Mix_Chunk *make_beep_chunk(int freq, int duration_ms, float vol_factor)
{
    int    samples    = SAMPLE_RATE * duration_ms / 1000;
    int    data_bytes = samples * (int)sizeof(Sint16);
    int    wav_size   = 44 + data_bytes;
    Uint8 *wav        = (Uint8 *)malloc(wav_size);
    float  dur, attack, release;
    Sint16 *pcm;
    SDL_RWops  *rw;
    Mix_Chunk  *chunk;
    int i;
    float t, env, s;

    if (!wav) return NULL;

    dur     = (float)duration_ms / 1000.0f;
    attack  = 0.01f;
    release = SDL_min((float)duration_ms / 1000.0f * 0.5f, 0.08f);
    pcm     = (Sint16 *)(wav + 44);

    for (i = 0; i < samples; i++) {
        t = (float)i / SAMPLE_RATE;
        if (t < attack)
            env = t / attack;
        else if (t > dur - release)
            env = (dur - t) / release;
        else
            env = 1.0f;
        s      = sinf(2.0f * (float)M_PI * freq * t);
        pcm[i] = (Sint16)(s * env * 20000.0f * vol_factor);
    }

    memcpy(wav + 0,  "RIFF", 4);
    write_le32(wav + 4,  (Uint32)(wav_size - 8));
    memcpy(wav + 8,  "WAVE", 4);
    memcpy(wav + 12, "fmt ", 4);
    write_le32(wav + 16, 16);
    write_le16(wav + 20, 1);
    write_le16(wav + 22, 1);
    write_le32(wav + 24, SAMPLE_RATE);
    write_le32(wav + 28, SAMPLE_RATE * (Uint32)sizeof(Sint16));
    write_le16(wav + 32, (Uint16)sizeof(Sint16));
    write_le16(wav + 34, 16);
    memcpy(wav + 36, "data", 4);
    write_le32(wav + 40, (Uint32)data_bytes);

    rw    = SDL_RWFromConstMem(wav, wav_size);
    chunk = NULL;
    if (rw)
        chunk = Mix_LoadWAV_RW(rw, 1);

    free(wav);
    return chunk;
}

void puzzle_audio_init(Game *g)
{
    const char  *img_paths[PUZZLE_COUNT];
    SDL_Surface *surf;
    int          i;

    if (Mix_OpenAudio(SAMPLE_RATE, AUDIO_S16SYS, 1, CHUNK_SIZE) < 0) {
        SDL_Log("Mix_OpenAudio error: %s", Mix_GetError());
        return;
    }
    Mix_AllocateChannels(8);
    g->sfx_hover     = make_beep_chunk(880,  60, 0.4f);
    g->sfx_click     = make_beep_chunk(1200, 80, 0.5f);
    g->music_main    = NULL;
    g->music_sub     = NULL;
    g->music_victory = NULL;
    g->music_quiz    = NULL;
    Mix_VolumeMusic((int)(MIX_MAX_VOLUME * g->volume / 100.0f));
    Mix_Volume(-1, (int)(MIX_MAX_VOLUME * g->volume / 100.0f));

    IMG_Init(IMG_INIT_PNG);

    img_paths[0] = "assets/images/thor.png";
    img_paths[1] = "assets/images/hulk.png";
    img_paths[2] = "assets/images/ironman.png";
    img_paths[3] = "assets/images/capamerica.png";
    img_paths[4] = "assets/images/thanos.png";

    for (i = 0; i < PUZZLE_COUNT; i++) {
        puzzle_textures[i] = NULL;
        surf = IMG_Load(img_paths[i]);
        if (surf) {
            puzzle_textures[i] = SDL_CreateTextureFromSurface(g->renderer, surf);
            SDL_FreeSurface(surf);
        } else {
            SDL_Log("Could not load image %s: %s", img_paths[i], IMG_GetError());
        }
    }
}

void puzzle_audio_free(Game *g)
{
    int i;
    if (g->sfx_hover) { Mix_FreeChunk(g->sfx_hover); g->sfx_hover = NULL; }
    if (g->sfx_click) { Mix_FreeChunk(g->sfx_click); g->sfx_click = NULL; }
    for (i = 0; i < PUZZLE_COUNT; i++) {
        if (puzzle_textures[i]) {
            SDL_DestroyTexture(puzzle_textures[i]);
            puzzle_textures[i] = NULL;
        }
    }
    IMG_Quit();
    Mix_CloseAudio();
}

void draw_text_puzzle(SDL_Renderer *r, TTF_Font *font, const char *text,
                      SDL_Color color, int x, int y, int centered)
{
    SDL_Surface *surf;
    SDL_Texture *tex;
    SDL_Rect dst;

    if (!font || !text) return;
    surf = TTF_RenderUTF8_Blended(font, text, color);
    if (!surf) return;
    tex = SDL_CreateTextureFromSurface(r, surf);
    if (tex) {
        dst.w = surf->w;
        dst.h = surf->h;
        dst.x = centered ? x - surf->w / 2 : x;
        dst.y = y;
        SDL_RenderCopy(r, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

void draw_text_shadow_puzzle(SDL_Renderer *r, TTF_Font *font, const char *text,
                             SDL_Color color, int x, int y, int centered)
{
    SDL_Color shadow;
    shadow.r = 0; shadow.g = 0; shadow.b = 0; shadow.a = 180;
    draw_text_puzzle(r, font, text, shadow, x + 2, y + 2, centered);
    draw_text_puzzle(r, font, text, color,  x,     y,     centered);
}

void draw_filled_circle_puzzle(SDL_Renderer *r, int cx, int cy, int rad, SDL_Color c)
{
    int dy, dx;
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    for (dy = -rad; dy <= rad; dy++) {
        dx = (int)sqrt((double)(rad * rad - dy * dy));
        SDL_RenderDrawLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

void draw_bg_enigma_puzzle(SDL_Renderer *r, Uint32 tick)
{
    SDL_Color gc;
    int i, cx, cy;
    float phase;

    SDL_SetRenderDrawColor(r, 4, 4, 4, 255);
    SDL_RenderClear(r);

    srand(42);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    for (i = 0; i < 80; i++) {
        int sx     = rand() % WINDOW_W;
        int sy     = rand() % WINDOW_H;
        int bright = 100 + rand() % 155;
        float flk  = (float)(sinf((tick * 0.002f) + i * 0.7f) * 0.5f + 0.5f);
        Uint8 a    = (Uint8)(bright * flk);
        SDL_SetRenderDrawColor(r, 255, 255, 255, a);
        SDL_RenderDrawPoint(r, sx, sy);
        if (i % 5 == 0) SDL_RenderDrawPoint(r, sx + 1, sy);
    }
    srand((unsigned)SDL_GetTicks());

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_ADD);
    for (i = 0; i < 3; i++) {
        phase = tick * 0.001f + i * 2.094f;
        cx    = WINDOW_W / 2 + (int)(cos(phase) * 200);
        cy    = WINDOW_H / 2 + (int)(sin(phase) * 100);
        gc.r  = 180; gc.g = 20; gc.b = 20; gc.a = 30;
        draw_filled_circle_puzzle(r, cx, cy, 120, gc);
    }
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
}

void draw_panel_puzzle(SDL_Renderer *r, SDL_Rect rect)
{
    SDL_Color gold = color_gold();
    SDL_Rect  inner;

    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 18, 18, 22, 210);
    SDL_RenderFillRect(r, &rect);
    SDL_SetRenderDrawColor(r, gold.r, gold.g, gold.b, 200);
    SDL_RenderDrawRect(r, &rect);
    inner.x = rect.x + 1; inner.y = rect.y + 1;
    inner.w = rect.w - 2; inner.h = rect.h - 2;
    SDL_SetRenderDrawColor(r, 40, 40, 50, 120);
    SDL_RenderDrawRect(r, &inner);
}

void draw_avengers_logo_puzzle(SDL_Renderer *r, int cx, int cy, int size)
{
    SDL_Color gold = color_gold();
    int half, h, apex_x, apex_y, bl_x, bl_y, br_x, br_y;
    int bar_y, bar_left, bar_right, notch_top, notch_bot, t;

    SDL_SetRenderDrawColor(r, gold.r, gold.g, gold.b, 255);
    half   = size / 2;
    h      = (int)(size * 0.866f);
    apex_x = cx;
    apex_y = cy - h * 2 / 3;
    bl_x   = cx - half;
    bl_y   = cy + h / 3;
    br_x   = cx + half;
    br_y   = cy + h / 3;

    for (t = -1; t <= 1; t++) {
        SDL_RenderDrawLine(r, apex_x + t, apex_y, bl_x + t, bl_y);
        SDL_RenderDrawLine(r, apex_x + t, apex_y, br_x + t, br_y);
        SDL_RenderDrawLine(r, bl_x + t,   bl_y,   br_x + t, br_y);
    }

    bar_y     = cy - h / 8;
    bar_left  = cx - half * 2 / 5;
    bar_right = cx + half * 2 / 5;
    for (t = -1; t <= 1; t++)
        SDL_RenderDrawLine(r, bar_left, bar_y + t, bar_right, bar_y + t);

    notch_top = cy - h * 2 / 5;
    notch_bot = cy + h / 6;
    SDL_RenderDrawLine(r, cx, notch_top, cx, notch_bot);
}

void button_init_puzzle(Button *b, int x, int y, int w, int h, const char *label,
                        SDL_Color cn, SDL_Color ch, SDL_Color ct)
{
    b->rect.x      = x; b->rect.y = y;
    b->rect.w      = w; b->rect.h = h;
    b->hovered     = 0;
    b->active      = 0;
    b->color_normal = cn;
    b->color_hover  = ch;
    b->color_text   = ct;
    strncpy(b->label, label, 63);
    b->label[63] = '\0';
}

void button_draw_puzzle(SDL_Renderer *r, TTF_Font *font, Button *b)
{
    SDL_Color bg, border;
    SDL_Surface *surf;
    SDL_Texture *tex;
    SDL_Rect dst;

    bg = b->hovered ? b->color_hover : b->color_normal;
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderFillRect(r, &b->rect);
    border = b->hovered ? color_gold() : color_grey();
    SDL_SetRenderDrawColor(r, border.r, border.g, border.b, 255);
    SDL_RenderDrawRect(r, &b->rect);

    if (font && b->label[0]) {
        surf = TTF_RenderUTF8_Blended(font, b->label, b->color_text);
        if (surf) {
            tex = SDL_CreateTextureFromSurface(r, surf);
            if (tex) {
                dst.w = surf->w; dst.h = surf->h;
                dst.x = b->rect.x + (b->rect.w - dst.w) / 2;
                dst.y = b->rect.y + (b->rect.h - dst.h) / 2;
                SDL_RenderCopy(r, tex, NULL, &dst);
                SDL_DestroyTexture(tex);
            }
            SDL_FreeSurface(surf);
        }
    }
}

int button_hit_puzzle(Button *b, int mx, int my)
{
    return mx >= b->rect.x && mx <= b->rect.x + b->rect.w &&
           my >= b->rect.y && my <= b->rect.y + b->rect.h;
}

int button_update_puzzle(Button *b, int mx, int my, int *played, Mix_Chunk *sfx)
{
    int was = b->hovered;
    b->hovered = button_hit_puzzle(b, mx, my);
    if (b->hovered && !was && !(*played)) {
        if (sfx) Mix_PlayChannel(-1, sfx, 0);
        *played = 1;
    }
    return b->hovered;
}

void generate_puzzle(void)
{
    const char *names[PUZZLE_COUNT];
    SDL_Color   pcolors[PUZZLE_COUNT];
    SDL_Color   base_color, wrong_base;
    int         grid_start_x, grid_start_y;
    int         option_y, spacing, option_start_x;
    int         correct_index, color_var;
    int         i, x, y;

    names[0] = "THOR";
    names[1] = "HULK";
    names[2] = "IRON MAN";
    names[3] = "CAPTAIN AMERICA";
    names[4] = "THANOS";

    pcolors[0].r = 100; pcolors[0].g = 150; pcolors[0].b = 220; pcolors[0].a = 255;
    pcolors[1].r =  80; pcolors[1].g = 180; pcolors[1].b =  80; pcolors[1].a = 255;
    pcolors[2].r = 200; pcolors[2].g =  50; pcolors[2].b =  50; pcolors[2].a = 255;
    pcolors[3].r =  50; pcolors[3].g = 100; pcolors[3].b = 200; pcolors[3].a = 255;
    pcolors[4].r = 150; pcolors[4].g = 100; pcolors[4].b = 200; pcolors[4].a = 255;

    if (!puzzle_seeded) {
        srand((unsigned int)time(NULL));
        puzzle_seeded = 1;
    }

    current_puzzle = rand() % PUZZLE_COUNT;
    strncpy(current_puzzle_name, names[current_puzzle], 31);
    current_puzzle_name[31] = '\0';

    base_color     = pcolors[current_puzzle];
    grid_start_x   = WINDOW_W / 2 - (GRID_SIZE * PIECE_SIZE) / 2;
    grid_start_y   = 180;

    for (y = 0; y < GRID_SIZE; y++) {
        for (x = 0; x < GRID_SIZE; x++) {
            grid[y][x].px     = x;
            grid[y][x].py     = y;
            grid[y][x].active = 1;
            grid[y][x].rect.x = grid_start_x + x * PIECE_SIZE;
            grid[y][x].rect.y = grid_start_y + y * PIECE_SIZE;
            grid[y][x].rect.w = PIECE_SIZE;
            grid[y][x].rect.h = PIECE_SIZE;
            color_var           = (x + y) * 15;
            grid[y][x].color.r  = (Uint8)CLAMP_VAL(base_color.r + color_var, 0, 255);
            grid[y][x].color.g  = (Uint8)CLAMP_VAL(base_color.g + color_var, 0, 255);
            grid[y][x].color.b  = (Uint8)CLAMP_VAL(base_color.b + color_var, 0, 255);
            grid[y][x].color.a  = 255;
        }
    }

    do {
        missing_px = rand() % GRID_SIZE;
        missing_py = rand() % GRID_SIZE;
    } while (missing_px == 1 && missing_py == 1);

    grid[missing_py][missing_px].active = 0;

    option_y       = grid_start_y + GRID_SIZE * PIECE_SIZE + 60;
    spacing        = 120;
    option_start_x = WINDOW_W / 2 - (OPTION_PIECES * spacing) / 2 + 40;
    correct_index  = rand() % OPTION_PIECES;

    for (i = 0; i < OPTION_PIECES; i++) {
        options_puzzle[i].rect.x   = option_start_x + i * spacing;
        options_puzzle[i].rect.y   = option_y;
        options_puzzle[i].rect.w   = PIECE_SIZE;
        options_puzzle[i].rect.h   = PIECE_SIZE;
        options_puzzle[i].dragging = 0;

        if (i == correct_index) {
            options_puzzle[i].color      = grid[missing_py][missing_px].color;
            options_puzzle[i].is_correct = 1;
            option_src_char[i]           = current_puzzle;
            option_src_x[i]              = missing_px;
            option_src_y[i]              = missing_py;
        } else {
            wrong_base = pcolors[(current_puzzle + i + 1) % PUZZLE_COUNT];
            options_puzzle[i].color.r    = wrong_base.r;
            options_puzzle[i].color.g    = wrong_base.g;
            options_puzzle[i].color.b    = wrong_base.b;
            options_puzzle[i].color.a    = 255;
            options_puzzle[i].is_correct = 0;
            option_src_char[i]           = (current_puzzle + i + 1) % PUZZLE_COUNT;
            option_src_x[i]              = missing_px;
            option_src_y[i]              = missing_py;
        }
    }
}

void menu_puzzle_init(Game *g)
{
    SDL_Color cn, ch, ct;

    (void)g;
    cn.r = 28;  cn.g = 28;  cn.b = 32;  cn.a = 220;
    ch.r = 180; ch.g = 20;  ch.b = 20;  ch.a = 240;
    ct.r = 230; ct.g = 230; ct.b = 230; ct.a = 255;

    button_init_puzzle(&btn_back_puzzle, 60, 30, 100, 34, "back", cn, ch, ct);
    generate_puzzle();
    puzzle_solved          = 0;
    puzzle_failed          = 0;
    puzzle_timed_out       = 0;
    game_over              = 0;
    consecutive_correct    = 0;
    consecutive_wrong      = 0;
    bonus_msg[0]           = '\0';
    start_time_puzzle      = SDL_GetTicks();
    result_scale_puzzle    = 1.0f;
    result_fade_puzzle     = 0;
}

void menu_puzzle_handle_event(Game *g, SDL_Event *e)
{
    int mx, my, i;
    SDL_Rect missing_rect;

    if (e->type == SDL_KEYDOWN) {
        if (e->key.keysym.sym == SDLK_ESCAPE)
            g->state = STATE_ENIGMA;
        if (e->key.keysym.sym == SDLK_SPACE && (puzzle_solved || puzzle_failed) && !game_over) {
            generate_puzzle();
            puzzle_solved      = 0;
            puzzle_failed      = 0;
            puzzle_timed_out   = 0;
            result_fade_puzzle = 0;
        }
    }

    if (puzzle_solved || puzzle_failed) {
        if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
            mx = e->button.x; my = e->button.y;
            if (button_hit_puzzle(&btn_back_puzzle, mx, my))
                g->state = STATE_ENIGMA;
        }
        if (e->type == SDL_MOUSEMOTION) {
            hover_played_puzzle = 0;
            mx = e->motion.x; my = e->motion.y;
            button_update_puzzle(&btn_back_puzzle, mx, my, &hover_played_puzzle, g->sfx_hover);
        }
        return;
    }

    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        mx = e->button.x; my = e->button.y;
        if (button_hit_puzzle(&btn_back_puzzle, mx, my)) {
            g->state = STATE_ENIGMA;
            return;
        }
        for (i = 0; i < OPTION_PIECES; i++) {
            SDL_Rect rr = options_puzzle[i].rect;
            if (mx >= rr.x && mx <= rr.x + rr.w && my >= rr.y && my <= rr.y + rr.h) {
                options_puzzle[i].dragging      = 1;
                options_puzzle[i].drag_offset_x = mx - rr.x;
                options_puzzle[i].drag_offset_y = my - rr.y;
                if (g->sfx_click) Mix_PlayChannel(-1, g->sfx_click, 0);
                break;
            }
        }
    }

    if (e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT) {
        mx = e->button.x; my = e->button.y;
        for (i = 0; i < OPTION_PIECES; i++) {
            if (options_puzzle[i].dragging) {
                missing_rect = grid[missing_py][missing_px].rect;
                if (mx >= missing_rect.x && mx <= missing_rect.x + missing_rect.w &&
                    my >= missing_rect.y && my <= missing_rect.y + missing_rect.h) {
                    if (options_puzzle[i].is_correct) {
                        grid[missing_py][missing_px].active = 1;
                        puzzle_solved      = 1;
                        result_fade_puzzle = 255;
                        if (g->sfx_click) Mix_PlayChannel(-1, g->sfx_click, 0);
                        consecutive_wrong = 0;
                        consecutive_correct++;
                        if (consecutive_correct == 3) {
                            strncpy(bonus_msg, "You won an extra life", 63);
                            bonus_msg[63]       = '\0';
                            consecutive_correct = 0;
                        } else {
                            bonus_msg[0] = '\0';
                        }
                    } else {
                        puzzle_failed      = 1;
                        result_fade_puzzle = 255;
                        consecutive_correct = 0;
                        consecutive_wrong++;
                        if (consecutive_wrong == 3) {
                            strncpy(bonus_msg, "You lose", 63);
                            bonus_msg[63]   = '\0';
                            game_over       = 1;
                        } else {
                            bonus_msg[0] = '\0';
                        }
                    }
                }
                options_puzzle[i].dragging = 0;
            }
        }
    }

    if (e->type == SDL_MOUSEMOTION) {
        hover_played_puzzle = 0;
        mx = e->motion.x; my = e->motion.y;
        button_update_puzzle(&btn_back_puzzle, mx, my, &hover_played_puzzle, g->sfx_hover);
        for (i = 0; i < OPTION_PIECES; i++) {
            if (options_puzzle[i].dragging) {
                options_puzzle[i].rect.x = mx - options_puzzle[i].drag_offset_x;
                options_puzzle[i].rect.y = my - options_puzzle[i].drag_offset_y;
            }
        }
    }
}

void menu_puzzle_draw(Game *g)
{
    SDL_Renderer *r = g->renderer;
    SDL_Color gold  = color_gold();
    SDL_Color white = color_white();
    SDL_Color red   = color_red();
    SDL_Color green, timer_color, border_color, msg_color;
    SDL_Rect  panel, title_line, empty, overlay, r_piece, shadow_rect;
    const char *msg;
    char   lbl[8];
    int    x, y, i, ww, angle, arc_angle;
    int    timer_cx, timer_cy, timer_radius;
    Uint32 elapsed;
    int    iw, ih;
    SDL_Rect src_rect;

    green.r = 80; green.g = 220; green.b = 80; green.a = 255;

    draw_bg_enigma_puzzle(r, g->frame_tick);

    panel.x = 100; panel.y = 60; panel.w = 760; panel.h = 500;
    draw_panel_puzzle(r, panel);

    draw_avengers_logo_puzzle(r, WINDOW_W / 2, 80, 28);
    draw_text_shadow_puzzle(r, g->font_large, "PUZZLE", gold, WINDOW_W / 2, 90, 1);

    draw_text_puzzle(r, g->font_medium, current_puzzle_name, gold, WINDOW_W / 2, 130, 1);

    title_line.x = WINDOW_W / 2 - 150; title_line.y = 155;
    title_line.w = 300;                 title_line.h = 2;
    SDL_SetRenderDrawColor(r, gold.r, gold.g, gold.b, gold.a);
    SDL_RenderFillRect(r, &title_line);

    if (!puzzle_solved && !puzzle_failed) {
        elapsed               = SDL_GetTicks() - start_time_puzzle;
        timer_progress_puzzle = (float)elapsed / (float)TIME_LIMIT_MS;
        if (timer_progress_puzzle >= 1.0f) {
            puzzle_failed      = 1;
            puzzle_timed_out   = 1;
            game_over          = 1;
            result_fade_puzzle = 255;
        }
    }

    timer_cx     = WINDOW_W - 150;
    timer_cy     = 120;
    timer_radius = 30;

    SDL_SetRenderDrawColor(r, 40, 40, 45, 255);
    for (ww = 0; ww < timer_radius; ww++) {
        for (angle = 0; angle < 360; angle++) {
            x = timer_cx + (int)(ww * cos(angle * M_PI / 180.0));
            y = timer_cy + (int)(ww * sin(angle * M_PI / 180.0));
            SDL_RenderDrawPoint(r, x, y);
        }
    }

    timer_color = (timer_progress_puzzle < 0.7f) ? green : red;
    SDL_SetRenderDrawColor(r, timer_color.r, timer_color.g, timer_color.b, timer_color.a);
    arc_angle = (int)(360 * timer_progress_puzzle);
    for (angle = 0; angle < arc_angle; angle++) {
        for (ww = timer_radius - 8; ww < timer_radius; ww++) {
            x = timer_cx + (int)(ww * cos((angle - 90) * M_PI / 180.0));
            y = timer_cy + (int)(ww * sin((angle - 90) * M_PI / 180.0));
            SDL_RenderDrawPoint(r, x, y);
        }
    }

    SDL_SetRenderDrawColor(r, 28, 28, 32, 255);
    for (ww = 0; ww < timer_radius - 10; ww++) {
        for (angle = 0; angle < 360; angle++) {
            x = timer_cx + (int)(ww * cos(angle * M_PI / 180.0));
            y = timer_cy + (int)(ww * sin(angle * M_PI / 180.0));
            SDL_RenderDrawPoint(r, x, y);
        }
    }

    for (y = 0; y < GRID_SIZE; y++) {
        for (x = 0; x < GRID_SIZE; x++) {
            if (grid[y][x].active) {
                if (puzzle_textures[current_puzzle]) {
                    SDL_QueryTexture(puzzle_textures[current_puzzle], NULL, NULL, &iw, &ih);
                    src_rect.x = iw / GRID_SIZE * grid[y][x].px;
                    src_rect.y = ih / GRID_SIZE * grid[y][x].py;
                    src_rect.w = iw / GRID_SIZE;
                    src_rect.h = ih / GRID_SIZE;
                    SDL_RenderCopy(r, puzzle_textures[current_puzzle], &src_rect, &grid[y][x].rect);
                } else {
                    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(r, grid[y][x].color.r, grid[y][x].color.g,
                                           grid[y][x].color.b, grid[y][x].color.a);
                    SDL_RenderFillRect(r, &grid[y][x].rect);
                }
                SDL_SetRenderDrawColor(r, 20, 20, 25, 255);
                SDL_RenderDrawRect(r, &grid[y][x].rect);
            } else {
                empty = grid[y][x].rect;
                SDL_SetRenderDrawColor(r, 60, 60, 70, 180);
                SDL_RenderFillRect(r, &empty);
                SDL_SetRenderDrawColor(r, 120, 120, 130, 255);
                for (i = 0; i < empty.w; i += 10) {
                    SDL_RenderDrawPoint(r, empty.x + i, empty.y);
                    SDL_RenderDrawPoint(r, empty.x + i, empty.y + empty.h);
                }
                for (i = 0; i < empty.h; i += 10) {
                    SDL_RenderDrawPoint(r, empty.x, empty.y + i);
                    SDL_RenderDrawPoint(r, empty.x + empty.w, empty.y + i);
                }
                draw_text_puzzle(r, g->font_small, "?", gold,
                                 empty.x + PIECE_SIZE / 2, empty.y + PIECE_SIZE / 2, 1);
            }
        }
    }

    draw_text_puzzle(r, g->font_small, "Glissez la bonne piece dans l'espace vide",
                     (SDL_Color){150, 150, 150, 200}, WINDOW_W / 2, 440, 1);

    for (i = 0; i < OPTION_PIECES; i++) {
        r_piece = options_puzzle[i].rect;
        if (options_puzzle[i].dragging) {
            shadow_rect   = r_piece;
            shadow_rect.x += 4; shadow_rect.y += 4;
            SDL_SetRenderDrawColor(r, 0, 0, 0, 100);
            SDL_RenderFillRect(r, &shadow_rect);
        }

        if (puzzle_textures[option_src_char[i]]) {
            SDL_QueryTexture(puzzle_textures[option_src_char[i]], NULL, NULL, &iw, &ih);
            src_rect.x = iw / GRID_SIZE * option_src_x[i];
            src_rect.y = ih / GRID_SIZE * option_src_y[i];
            src_rect.w = iw / GRID_SIZE;
            src_rect.h = ih / GRID_SIZE;
            SDL_RenderCopy(r, puzzle_textures[option_src_char[i]], &src_rect, &r_piece);
        } else {
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, options_puzzle[i].color.r, options_puzzle[i].color.g,
                                   options_puzzle[i].color.b, options_puzzle[i].color.a);
            SDL_RenderFillRect(r, &r_piece);
        }

        border_color = options_puzzle[i].dragging ? color_gold() : (SDL_Color){40, 40, 45, 255};
        SDL_SetRenderDrawColor(r, border_color.r, border_color.g, border_color.b, border_color.a);
        SDL_RenderDrawRect(r, &r_piece);

        snprintf(lbl, sizeof(lbl), "%c", 'A' + i);
        draw_text_puzzle(r, g->font_small, lbl, white,
                         r_piece.x + 8, r_piece.y + 4, 0);
    }

    if (puzzle_solved || puzzle_failed) {
        if (result_fade_puzzle > 10) result_fade_puzzle -= 10;
        result_scale_puzzle = 1.0f + 0.2f * sinf(
            (float)(SDL_GetTicks() % 1000) / 1000.0f * 2.0f * (float)M_PI);
        (void)result_scale_puzzle;

        if (puzzle_solved) {
            msg_color = green;
            msg       = "Success";
        } else if (puzzle_timed_out) {
            msg_color = red;
            msg       = "Time perdu";
        } else {
            msg_color = red;
            msg       = "Fail";
        }

        SDL_SetRenderDrawColor(r, 0, 0, 0, 150);
        overlay.x = 0; overlay.y = 0; overlay.w = WINDOW_W; overlay.h = WINDOW_H;
        SDL_RenderFillRect(r, &overlay);

        draw_text_shadow_puzzle(r, g->font_large, msg, msg_color,
                                WINDOW_W / 2, WINDOW_H / 2 - 50, 1);

        if (bonus_msg[0] != '\0') {
            draw_text_shadow_puzzle(r, g->font_medium, bonus_msg, gold,
                                    WINDOW_W / 2, WINDOW_H / 2 + 5, 1);
        }

        if (!game_over) {
            draw_text_puzzle(r, g->font_medium, "Appuyez sur ESPACE pour continuer",
                             white, WINDOW_W / 2, WINDOW_H / 2 + 40, 1);
        } else {
            draw_text_puzzle(r, g->font_medium, "Partie terminee - ESC pour quitter",
                             white, WINDOW_W / 2, WINDOW_H / 2 + 40, 1);
        }
    }

    button_draw_puzzle(r, g->font_small, &btn_back_puzzle);
    draw_text_puzzle(r, g->font_small, "ESC: Retour   ESPACE: Nouveau puzzle",
                     (SDL_Color){100, 100, 100, 180}, WINDOW_W / 2, WINDOW_H - 24, 1);
}
