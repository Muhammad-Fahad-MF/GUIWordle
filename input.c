#include "constants.h"

void handle_keypress(GameContext* game, SDL_Keycode key) {
    if (game->game_state != GAME_IDLE && game->game_state != GAME_OVER) return;
    if (game->game_state == GAME_OVER) { if (key == SDLK_r) reset_game(game, game->game_mode); return; }

    if (key == SDLK_BACKSPACE) {
        if (game->current_col > 0) {
            game->current_col--;
            game->grid[game->current_row][game->current_col].letter = 0;
            game->grid[game->current_row][game->current_col].state = STATE_EMPTY;
        }
    } else if (key == SDLK_RETURN) {
        if (game->current_col == 5) check_word(game);
        else { set_message(game, "Not enough letters"); trigger_shake(game); }
    } else if (key >= SDLK_a && key <= SDLK_z) {
        if (game->current_col < 5) {
            Tile* t = &game->grid[game->current_row][game->current_col];
            t->letter = toupper((char)key); t->state = STATE_TYPING; t->scale_pop = 1.15f;
            game->current_col++;
        }
    }
}

void process_input(GameContext* game) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) game->running = false;

        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) game->running = false;
            if (event.key.keysym.sym == SDLK_F11 && event.key.repeat == 0) {
                Uint32 flags = SDL_GetWindowFlags(game->window);
                if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                    SDL_SetWindowFullscreen(game->window, 0);
                    SDL_SetWindowSize(game->window, DEFAULT_WIDTH, DEFAULT_HEIGHT);
                    SDL_SetWindowPosition(game->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                    SDL_SetWindowResizable(game->window, SDL_TRUE);
                } else {
                    SDL_SetWindowFullscreen(game->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                }
            } else {
                handle_keypress(game, event.key.keysym.sym);
            }
        }

        if (game->game_state == GAME_MENU || game->game_state == GAME_ABOUT) {
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int x = event.button.x, y = event.button.y;

                if (game->game_state == GAME_MENU) {
                    if (x >= game->button_daily.x && x <= game->button_daily.x + game->button_daily.w &&
                        y >= game->button_daily.y && y <= game->button_daily.y + game->button_daily.h) {

                        // --- NEW: CHECK LOCK ---
                        time_t t = time(NULL);
                        struct tm *tm = localtime(&t);
                        int today_id = (tm->tm_year + 1900) * 1000 + tm->tm_yday;
                        if (game->stats.last_daily_won_yday != today_id) {
                            reset_game(game, MODE_DAILY);
                        }
                    }
                    else if (x >= game->button_infinite.x && x <= game->button_infinite.x + game->button_infinite.w &&
                             y >= game->button_infinite.y && y <= game->button_infinite.y + game->button_infinite.h) {
                        reset_game(game, MODE_INFINITE);
                    }
                    else if (x >= game->button_about.x && x <= game->button_about.x + game->button_about.w &&
                             y >= game->button_about.y && y <= game->button_about.y + game->button_about.h) {
                        game->game_state = GAME_ABOUT;
                    }
                } else {
                    game->game_state = GAME_MENU;
                }
            }
        } else {
            // Gameplay Input
            if (event.type == SDL_MOUSEWHEEL) game->scroll_y -= event.wheel.y * 30;

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int x = event.button.x, y = event.button.y;

                // --- NEW: CHECK BACK BUTTON ---
                if (x >= game->button_back.x && x <= game->button_back.x + game->button_back.w &&
                    y >= game->button_back.y && y <= game->button_back.y + game->button_back.h) {
                    game->game_state = GAME_MENU;
                    return; // Don't process keyboard click if back clicked
                }

                int wx = event.button.x - (game->win_w - DEFAULT_WIDTH)/2;
                if(wx < 0) wx += (game->win_w - DEFAULT_WIDTH)/2;
                int wy = event.button.y + (int)game->scroll_y;

                for (int i=0; i<28; i++) {
                    SDL_Rect r = game->keyboard[i].rect;
                    if (wx >= r.x && wx <= r.x+r.w && wy >= r.y && wy <= r.y+r.h) {
                        char k = game->keyboard[i].key;
                        if(k=='\r') handle_keypress(game, SDLK_RETURN);
                        else if(k=='\b') handle_keypress(game, SDLK_BACKSPACE);
                        else handle_keypress(game, (SDL_Keycode)tolower(k));
                    }
                }
            }
        }
    }

    if (game->game_state > GAME_ABOUT) {
        int max_s = CONTENT_HEIGHT - game->win_h;
        if(max_s < 0) max_s=0;
        if(game->scroll_y < 0) game->scroll_y=0;
        if(game->scroll_y > max_s) game->scroll_y=max_s;
    }
}