#include "constants.h"

void render_rounded_box(SDL_Renderer* r, SDL_Rect rect, int radius, SDL_Color c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_Rect center = {rect.x, rect.y + radius, rect.w, rect.h - 2 * radius};
    SDL_Rect top = {rect.x + radius, rect.y, rect.w - 2 * radius, radius};
    SDL_Rect bot = {rect.x + radius, rect.y + rect.h - radius, rect.w - 2 * radius, radius};
    SDL_RenderFillRect(r, &center); SDL_RenderFillRect(r, &top); SDL_RenderFillRect(r, &bot);

    int x = 0, y = radius, d = 3 - 2 * radius;
    while (y >= x) {
        SDL_RenderDrawLine(r, rect.x + radius - x, rect.y + radius - y, rect.x + radius - 1, rect.y + radius - y);
        SDL_RenderDrawLine(r, rect.x + radius - y, rect.y + radius - x, rect.x + radius - 1, rect.y + radius - x);
        SDL_RenderDrawLine(r, rect.x + rect.w - radius, rect.y + radius - y, rect.x + rect.w - radius + x - 1, rect.y + radius - y);
        SDL_RenderDrawLine(r, rect.x + rect.w - radius, rect.y + radius - x, rect.x + rect.w - radius + y - 1, rect.y + radius - x);
        SDL_RenderDrawLine(r, rect.x + radius - x, rect.y + rect.h - radius + y - 1, rect.x + radius - 1, rect.y + rect.h - radius + y - 1);
        SDL_RenderDrawLine(r, rect.x + radius - y, rect.y + rect.h - radius + x - 1, rect.x + radius - 1, rect.y + rect.h - radius + x - 1);
        SDL_RenderDrawLine(r, rect.x + rect.w - radius, rect.y + rect.h - radius + y - 1, rect.x + rect.w - radius + x - 1, rect.y + rect.h - radius + y - 1);
        SDL_RenderDrawLine(r, rect.x + rect.w - radius, rect.y + rect.h - radius + x - 1, rect.x + rect.w - radius + y - 1, rect.y + rect.h - radius + x - 1);
        if (d < 0) d += 4 * x + 6; else { d += 4 * (x - y) + 10; y--; } x++;
    }
}

void render_text_simple(SDL_Renderer* r, TTF_Font* f, const char* text, int x, int y, SDL_Color color) {
    if (!f || !text[0]) return;
    SDL_Surface* surf = TTF_RenderText_Blended(f, text, color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);

    // Handle Alpha
    if (color.a < 255) {
        SDL_SetTextureAlphaMod(tex, color.a);
    }

    SDL_Rect dest = {x - surf->w/2, y - surf->h/2, surf->w, surf->h};
    SDL_RenderCopy(r, tex, NULL, &dest);
    SDL_DestroyTexture(tex); SDL_FreeSurface(surf);
}

void render_text_in_rect(SDL_Renderer* r, TTF_Font* f, const char* text, SDL_Rect* target_rect, SDL_Color color) {
    if (!f || !text[0]) return;
    SDL_Surface* surf = TTF_RenderText_Blended(f, text, color);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(r, surf);
    float scale = (float)target_rect->h / TILE_SIZE;
    if (scale > 1.2f) scale = 1.2f;
    SDL_Rect dest = *target_rect;
    dest.w = surf->w; dest.h = (int)(surf->h * scale);
    dest.x = target_rect->x + target_rect->w/2 - dest.w/2;
    dest.y = target_rect->y + target_rect->h/2 - dest.h/2;
    if (dest.h > 2) SDL_RenderCopy(r, tex, NULL, &dest);
    SDL_DestroyTexture(tex); SDL_FreeSurface(surf);
}

void render_loading_screen(GameContext* game) {
    SDL_SetRenderDrawColor(game->renderer, COLOR_WHITE.r, COLOR_WHITE.g, COLOR_WHITE.b, 255);
    SDL_RenderClear(game->renderer);
    int cx = game->win_w / 2, cy = game->win_h / 2;
    render_text_simple(game->renderer, game->font_large, "Loading Wordle...", cx, cy - 50, COLOR_BLACK);
    SDL_Rect bar_bg = {cx - 150, cy, 300, 20};
    render_rounded_box(game->renderer, bar_bg, 10, COLOR_EMPTY_BORDER);
    float pct = (float)(SDL_GetTicks() - game->start_time) / LOADING_TIME_MS;
    if (pct > 1.0f) pct = 1.0f;
    SDL_Rect bar_fill = {cx - 150, cy, (int)(300 * pct), 20};
    if (bar_fill.w > 10) render_rounded_box(game->renderer, bar_fill, 10, COLOR_BLACK);
    SDL_RenderPresent(game->renderer);
}

void render_menu_screen(GameContext* game) {
    SDL_SetRenderDrawColor(game->renderer, COLOR_BG.r, COLOR_BG.g, COLOR_BG.b, 255);
    SDL_RenderClear(game->renderer);
    int cx = game->win_w / 2, cy = game->win_h / 2;
    int btn_w = 220, btn_h = 50;

    game->button_daily    = (SDL_Rect){cx - btn_w/2, cy - 60, btn_w, btn_h};
    game->button_infinite = (SDL_Rect){cx - btn_w/2, cy,      btn_w, btn_h};
    game->button_about    = (SDL_Rect){cx - btn_w/2, cy + 60, btn_w, btn_h};

    render_text_simple(game->renderer, game->font_ui, "Wordle Clone", cx, cy - 150, COLOR_BLACK);

    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    int today_id = (tm->tm_year + 1900) * 1000 + tm->tm_yday;
    bool solved_today = (game->stats.last_daily_won_yday == today_id);

    if (solved_today) {
        render_rounded_box(game->renderer, game->button_daily, 10, COLOR_ABSENT);
        render_text_simple(game->renderer, game->font_menu, "SOLVED TODAY", cx, cy - 60 + 25, COLOR_WHITE);
    } else {
        render_rounded_box(game->renderer, game->button_daily, 10, COLOR_CORRECT);
        render_text_simple(game->renderer, game->font_menu, "DAILY WORD", cx, cy - 60 + 25, COLOR_WHITE);
    }

    render_rounded_box(game->renderer, game->button_infinite, 10, COLOR_PRESENT);
    render_text_simple(game->renderer, game->font_menu, "INFINITE", cx, cy + 25, COLOR_WHITE);

    render_rounded_box(game->renderer, game->button_about, 10, COLOR_ABSENT);
    render_text_simple(game->renderer, game->font_menu, "ABOUT", cx, cy + 60 + 25, COLOR_WHITE);

    char stat_msg[64];
    snprintf(stat_msg, 64, "Streak: %d  |  Best: %d", game->stats.current_streak, game->stats.max_streak);
    render_text_simple(game->renderer, game->font_small, stat_msg, cx, cy + 160, COLOR_BLACK);

    render_text_simple(game->renderer, game->font_small, "Press ESC to Quit | F11 Fullscreen", cx, cy + 200, COLOR_ABSENT);
    SDL_RenderPresent(game->renderer);
}

void render_about_screen(GameContext* game) {
    SDL_SetRenderDrawColor(game->renderer, COLOR_BG.r, COLOR_BG.g, COLOR_BG.b, 255);
    SDL_RenderClear(game->renderer);
    int cx = game->win_w / 2, start_y = game->win_h / 2 - 200;

    render_text_simple(game->renderer, game->font_ui, "How to Play", cx, start_y, COLOR_BLACK);
    start_y += 60; render_text_simple(game->renderer, game->font_small, "Guess the Wordle in six tries.", cx, start_y, COLOR_BLACK);
    start_y += 30; render_text_simple(game->renderer, game->font_small, "Green: Correct letter & spot.", cx, start_y, COLOR_CORRECT);
    start_y += 30; render_text_simple(game->renderer, game->font_small, "Yellow: Correct letter, wrong spot.", cx, start_y, COLOR_PRESENT);
    start_y += 80; render_text_simple(game->renderer, game->font_menu, "Click anywhere to return", cx, start_y, COLOR_BLACK);
    SDL_RenderPresent(game->renderer);
}

// --- NEW: Render Particles ---
void render_particles(GameContext* game) {
    int off_x = (game->win_w - DEFAULT_WIDTH) / 2; if(off_x<0) off_x=0;
    int off_y = -(int)game->scroll_y;

    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (game->particles[i].active) {
            SDL_Rect r = {
                (int)game->particles[i].x + off_x,
                (int)game->particles[i].y + off_y,
                8, 8
            };
            SDL_SetRenderDrawColor(game->renderer,
                game->particles[i].color.r,
                game->particles[i].color.g,
                game->particles[i].color.b, 255);
            SDL_RenderFillRect(game->renderer, &r);
        }
    }
}

// --- NEW: Render Toast ---
void render_toast(GameContext* game) {
    if (game->message[0] == '\0') return;

    int cx = game->win_w / 2;
    // Position toast near the top, moving slightly based on animation if you added y-offset logic
    int cy = 115 - (int)game->scroll_y;

    int w = 260, h = 46;
    SDL_Rect rect = {cx - w/2, cy - h/2, w, h};

    SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_BLEND);
    SDL_Color bg = {20, 20, 20, (Uint8)game->message_alpha};

    render_rounded_box(game->renderer, rect, 10, bg);

    SDL_Color txt = {255, 255, 255, (Uint8)game->message_alpha};
    render_text_simple(game->renderer, game->font_small, game->message, cx, cy, txt);
}

void render_game(GameContext* game) {
    SDL_SetRenderDrawColor(game->renderer, COLOR_BG.r, COLOR_BG.g, COLOR_BG.b, 255);
    SDL_RenderClear(game->renderer);
    int offset_x = (game->win_w - DEFAULT_WIDTH) / 2; if(offset_x<0) offset_x=0;
    int offset_y = -(int)game->scroll_y;

    render_rounded_box(game->renderer, game->button_back, 5, COLOR_ABSENT);
    render_text_simple(game->renderer, game->font_small, "MENU", game->button_back.x + 50, game->button_back.y + 20, COLOR_WHITE);

    render_text_simple(game->renderer, game->font_ui, "Wordle", offset_x + DEFAULT_WIDTH/2, offset_y + 40, COLOR_BLACK);
    SDL_SetRenderDrawColor(game->renderer, COLOR_EMPTY_BORDER.r, COLOR_EMPTY_BORDER.g, COLOR_EMPTY_BORDER.b, 255);
    SDL_RenderDrawLine(game->renderer, offset_x, offset_y + 80, offset_x + DEFAULT_WIDTH, offset_y + 80);

    int grid_start_x = offset_x + (DEFAULT_WIDTH - (GRID_COLS * TILE_SIZE + (GRID_COLS-1)*TILE_PADDING)) / 2;
    int grid_start_y = offset_y + 120;

    for (int r = 0; r < GRID_ROWS; r++) {
        int shake_x = (r == game->current_row && game->shake_timer > 0) ? (int)(sinf(game->shake_timer * 0.8f) * 6.0f) : 0;
        for (int c = 0; c < GRID_COLS; c++) {
            Tile* t = &game->grid[r][c];
            int x = grid_start_x + c * (TILE_SIZE + TILE_PADDING) + shake_x;
            int y = grid_start_y + r * (TILE_SIZE + TILE_PADDING);
            SDL_Color fill = COLOR_WHITE, border = COLOR_EMPTY_BORDER, txt = COLOR_BLACK;
            if (t->state == STATE_TYPING) border = COLOR_FILLED_BORDER;
            else if (t->state == STATE_CORRECT) { fill=border=COLOR_CORRECT; txt=COLOR_WHITE; }
            else if (t->state == STATE_PRESENT) { fill=border=COLOR_PRESENT; txt=COLOR_WHITE; }
            else if (t->state == STATE_ABSENT) { fill=border=COLOR_ABSENT; txt=COLOR_WHITE; }

            float scale_y = t->flipping ? fabsf(cosf(t->flip_progress * PI)) : 1.0f;
            int w = (int)(TILE_SIZE * t->scale_pop); int h = (int)(TILE_SIZE * t->scale_pop * scale_y);
            SDL_Rect rect = {x - (w - TILE_SIZE)/2, y - (h - TILE_SIZE)/2, w, h};
            SDL_SetRenderDrawColor(game->renderer, fill.r, fill.g, fill.b, 255);
            SDL_RenderFillRect(game->renderer, &rect);
            SDL_SetRenderDrawColor(game->renderer, border.r, border.g, border.b, 255);
            SDL_RenderDrawRect(game->renderer, &rect);
            if (t->letter) { char s[2]={t->letter,0}; render_text_in_rect(game->renderer, game->font_large, s, &rect, txt); }
        }
    }
    for (int i = 0; i < 28; i++) {
        VirtualKey* vk = &game->keyboard[i];
        SDL_Color fill = {211, 214, 218, 255}, txt = COLOR_BLACK;
        if (vk->state==STATE_CORRECT) {fill=COLOR_CORRECT; txt=COLOR_WHITE;}
        else if (vk->state==STATE_PRESENT) {fill=COLOR_PRESENT; txt=COLOR_WHITE;}
        else if (vk->state==STATE_ABSENT) {fill=COLOR_ABSENT; txt=COLOR_WHITE;}
        SDL_Rect krect = vk->rect; krect.x += offset_x; krect.y += offset_y;
        render_rounded_box(game->renderer, krect, 5, fill);
        char s[6];
        if(vk->key=='\r') strcpy(s,"ENTER"); else if(vk->key=='\b') strcpy(s,"<"); else {s[0]=vk->key; s[1]=0;}
        render_text_simple(game->renderer, game->font_small, s, krect.x+krect.w/2, krect.y+krect.h/2, txt);
    }

    // NEW: Draw Particles and Toast
    render_particles(game);
    render_toast(game);

    if (CONTENT_HEIGHT > game->win_h) {
        int sb_h = (int)((float)game->win_h / CONTENT_HEIGHT * game->win_h);
        int sb_y = (int)((game->scroll_y / (CONTENT_HEIGHT - game->win_h)) * (game->win_h - sb_h));
        SDL_Rect sb = {game->win_w - 10, sb_y, 8, sb_h};
        SDL_SetRenderDrawColor(game->renderer, 180,180,180,255); SDL_RenderFillRect(game->renderer, &sb);
    }
    SDL_RenderPresent(game->renderer);
}