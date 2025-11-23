#include "constants.h"

// Helper for random float
float random_float(float min, float max) {
    return min + (float)rand() / (float)(RAND_MAX / (max - min));
}

// Helper for sorting
int compare_strings(const void* a, const void* b) {
    const char* strA = *(const char**)a;
    const char* strB = *(const char**)b;
    return strcmp(strA, strB);
}

void set_message(GameContext* game, const char* msg) {
    strncpy(game->message, msg, 63);
    game->message_timer = SDL_GetTicks();
    game->message_y = 0.0f; // Reset float offset
    game->message_alpha = 255;
}

void trigger_shake(GameContext* game) {
    game->shake_timer = SHAKE_DURATION;
}

void spawn_confetti(GameContext* game) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        game->particles[i].active = true;
        game->particles[i].x = random_float(0, DEFAULT_WIDTH);
        game->particles[i].y = random_float(-200, -50);
        game->particles[i].vx = random_float(-2, 2);
        game->particles[i].vy = random_float(2, 8);
        game->particles[i].life = (int)random_float(100, 300);
        game->particles[i].angle = random_float(0, 360);
        game->particles[i].v_angle = random_float(-5, 5);

        int r = rand() % 5;
        if (r==0) game->particles[i].color = COLOR_CORRECT;
        else if (r==1) game->particles[i].color = COLOR_PRESENT;
        else if (r==2) game->particles[i].color = (SDL_Color){255, 100, 100, 255};
        else if (r==3) game->particles[i].color = (SDL_Color){100, 100, 255, 255};
        else game->particles[i].color = COLOR_ABSENT;
    }
}

bool init_game(GameContext* game) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;
    if (TTF_Init() == -1) return false;

    game->window = SDL_CreateWindow(" Wordle", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  DEFAULT_WIDTH, DEFAULT_HEIGHT,
                                  SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_RESIZABLE);
    if (!game->window) return false;
    SDL_GetWindowSize(game->window, &game->win_w, &game->win_h);

    game->renderer = SDL_CreateRenderer(game->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    game->font_large = TTF_OpenFont("arial.ttf", 32);
    game->font_small = TTF_OpenFont("arialbd.ttf", 20);
    game->font_ui    = TTF_OpenFont("arialbd.ttf", 36);
    game->font_menu  = TTF_OpenFont("arialbd.ttf", 24);

    if (!game->font_large) SDL_Log("Error opening fonts.");

    game->running = true;
    game->game_state = GAME_LOADING;
    game->scroll_y = 0;

    // Initialize Particles and Toast
    memset(game->particles, 0, sizeof(game->particles));
    game->message_alpha = 0;

    load_stats(game);
    SDL_SetRenderDrawBlendMode(game->renderer, SDL_BLENDMODE_BLEND);

    int cx = game->win_w / 2;
    int cy = game->win_h / 2;
    int btn_w = 220, btn_h = 50;

    game->button_daily    = (SDL_Rect){cx - btn_w/2, cy - 60, btn_w, btn_h};
    game->button_infinite = (SDL_Rect){cx - btn_w/2, cy,      btn_w, btn_h};
    game->button_about    = (SDL_Rect){cx - btn_w/2, cy + 60, btn_w, btn_h};
    game->button_back     = (SDL_Rect){20, 20, 100, 40};

    return true;
}

void init_keyboard(GameContext* game) {
    const char* rows[] = {"QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};
    int start_y = CONTENT_HEIGHT - 350;
    int key_w = 43, key_h = 58, padding = 7, k_idx = 0;
    int center_x = DEFAULT_WIDTH / 2;

    int row_w = 10 * (key_w + padding) - padding;
    int start_x = center_x - (row_w / 2);
    for (int i = 0; i < 10; i++)
        game->keyboard[k_idx++] = (VirtualKey){rows[0][i], {start_x + i*(key_w+padding), start_y, key_w, key_h}, STATE_EMPTY};

    row_w = 9 * (key_w + padding) - padding;
    start_x = center_x - (row_w / 2);
    for (int i = 0; i < 9; i++)
        game->keyboard[k_idx++] = (VirtualKey){rows[1][i], {start_x + i*(key_w+padding), start_y + key_h + padding, key_w, key_h}, STATE_EMPTY};

    row_w = 7 * (key_w + padding) + 2 * (key_w * 1.7 + padding) - padding;
    start_x = center_x - (row_w / 2);
    game->keyboard[k_idx++] = (VirtualKey){'\r', {start_x, start_y + (key_h + padding)*2, (int)(key_w * 1.7), key_h}, STATE_EMPTY};
    int current_x = start_x + (int)(key_w * 1.7) + padding;
    for (int i = 0; i < 7; i++) {
        game->keyboard[k_idx++] = (VirtualKey){rows[2][i], {current_x, start_y + (key_h + padding)*2, key_w, key_h}, STATE_EMPTY};
        current_x += key_w + padding;
    }
    game->keyboard[k_idx++] = (VirtualKey){'\b', {current_x, start_y + (key_h + padding)*2, (int)(key_w * 1.7), key_h}, STATE_EMPTY};
}

void load_dictionary(GameContext* game) {
    FILE* f = fopen("words.txt", "r");
    if (!f) {
        game->dict_count = 1;
        game->dictionary = malloc(sizeof(char*));
        game->dictionary[0] = strdup("HELLO");
        return;
    }
    char buffer[128];
    int count = 0;
    while(fgets(buffer, sizeof(buffer), f)) count++;
    rewind(f);
    game->dictionary = malloc(count * sizeof(char*));
    game->dict_count = 0;
    while(fgets(buffer, sizeof(buffer), f)) {
        buffer[strcspn(buffer, "\r\n")] = 0;
        if (strlen(buffer) == 5) {
            for(int i=0; buffer[i]; i++) buffer[i] = toupper(buffer[i]);
            game->dictionary[game->dict_count++] = strdup(buffer);
        }
    }
    fclose(f);
    qsort(game->dictionary, game->dict_count, sizeof(char*), compare_strings);
}

void pick_word(GameContext* game, GameMode mode) {
    if (game->dict_count == 0) return;
    unsigned int seed;
    if (mode == MODE_DAILY) {
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        seed = (tm->tm_year + 1900) * 1000 + tm->tm_yday;
        set_message(game, "Daily Challenge");
    } else {
        seed = (unsigned int)time(NULL);
    }
    srand(seed);
    strcpy(game->target_word, game->dictionary[rand() % game->dict_count]);
    SDL_Log("Target [%s] Mode: %d", game->target_word, mode);
}

void save_stats(GameContext* game) {
    FILE* f = fopen("save.dat", "wb");
    if (f) {
        fwrite(&game->stats, sizeof(GameStats), 1, f);
        fclose(f);
    }
}

void load_stats(GameContext* game) {
    FILE* f = fopen("save.dat", "rb");
    if (f) {
        fread(&game->stats, sizeof(GameStats), 1, f);
        fclose(f);
    } else {
        memset(&game->stats, 0, sizeof(GameStats));
    }
}

void check_word(GameContext* game) {
    char guess[6];
    for(int i=0; i<5; i++) guess[i] = game->grid[game->current_row][i].letter;
    guess[5] = '\0';

    char* key = guess;
    bool valid = (bsearch(&key, game->dictionary, game->dict_count, sizeof(char*), compare_strings) != NULL);

    if (!valid) {
        set_message(game, "Not in word list");
        trigger_shake(game);
        return;
    }

    int letter_counts[26] = {0};
    for(int i=0; i<5; i++) letter_counts[game->target_word[i] - 'A']++;
    bool guess_matched[5] = {0};

    for (int i = 0; i < 5; i++) {
        if (guess[i] == game->target_word[i]) {
            game->grid[game->current_row][i].target_state = STATE_CORRECT;
            letter_counts[guess[i] - 'A']--;
            guess_matched[i] = true;
        }
    }
    for (int i = 0; i < 5; i++) {
        if (!guess_matched[i]) {
            if (letter_counts[guess[i] - 'A'] > 0) {
                game->grid[game->current_row][i].target_state = STATE_PRESENT;
                letter_counts[guess[i] - 'A']--;
            } else {
                game->grid[game->current_row][i].target_state = STATE_ABSENT;
            }
        }
    }
    game->game_state = GAME_REVEALING;
    game->reveal_col_index = 0;
    game->reveal_timer = 0;
    game->grid[game->current_row][0].flipping = true;
}

void update_game(GameContext* game) {
    if (game->game_state == GAME_LOADING) {
        if (SDL_GetTicks() - game->start_time >= LOADING_TIME_MS) game->game_state = GAME_MENU;
        return;
    }

    // 1. Update Particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (game->particles[i].active) {
            game->particles[i].x += game->particles[i].vx;
            game->particles[i].y += game->particles[i].vy;
            game->particles[i].angle += game->particles[i].v_angle;
            game->particles[i].vy += 0.1f;
            game->particles[i].vx *= 0.99f;
            if (game->particles[i].y > CONTENT_HEIGHT) game->particles[i].active = false;
            if (--game->particles[i].life <= 0) game->particles[i].active = false;
        }
    }

    // 2. Update Toast
    if (game->message[0] != '\0') {
        Uint32 elapsed = SDL_GetTicks() - game->message_timer;
        if (elapsed < 200) game->message_alpha = (elapsed * 255) / 200;
        else if (elapsed > 1500) {
            int fade = 255 - ((elapsed - 1500) * 255 / 500);
            game->message_alpha = (fade < 0) ? 0 : fade;
        } else {
            game->message_alpha = 255;
        }
        if (game->message_alpha == 0 && elapsed > 2000) game->message[0] = '\0';
    }

    if (game->game_state == GAME_IDLE || game->game_state == GAME_REVEALING || game->game_state == GAME_OVER) {
        for(int r=0; r<GRID_ROWS; r++) for(int c=0; c<GRID_COLS; c++)
            if (game->grid[r][c].scale_pop > 1.0f) game->grid[r][c].scale_pop -= 0.05f;

        if (game->shake_timer > 0) if ((game->shake_timer -= 1.0f) < 0) game->shake_timer = 0;

        if (game->game_state == GAME_REVEALING) {
            if (game->reveal_col_index < GRID_COLS - 1) {
                if (++game->reveal_timer >= REVEAL_DELAY) {
                    game->reveal_timer = 0;
                    game->grid[game->current_row][++game->reveal_col_index].flipping = true;
                }
            }
            bool all_done = true;
            for (int c = 0; c < GRID_COLS; c++) {
                Tile* t = &game->grid[game->current_row][c];
                if (t->flipping) {
                    all_done = false;
                    t->flip_progress += FLIP_SPEED;
                    if (t->flip_progress >= 0.5f && t->state != t->target_state) {
                        t->state = t->target_state;
                        for (int k = 0; k < 28; k++) {
                            if (game->keyboard[k].key == t->letter) {
                                if (t->state == STATE_CORRECT) game->keyboard[k].state = STATE_CORRECT;
                                else if (t->state == STATE_PRESENT && game->keyboard[k].state != STATE_CORRECT) game->keyboard[k].state = STATE_PRESENT;
                                else if (t->state == STATE_ABSENT && game->keyboard[k].state == STATE_EMPTY) game->keyboard[k].state = STATE_ABSENT;
                            }
                        }
                    }
                    if (t->flip_progress >= 1.0f) { t->flip_progress = 0.0f; t->flipping = false; }
                }
            }
            if (all_done && game->reveal_col_index == GRID_COLS - 1) {

                // Reconstruct guess
                char guess[6];
                for(int i=0; i<5; i++) guess[i] = game->grid[game->current_row][i].letter;
                guess[5] = '\0';

                if (strcmp(guess, game->target_word) == 0) {
                    game->won = true;
                    game->game_state = GAME_OVER;
                    set_message(game, "Splendid! Press 'R'");
                    spawn_confetti(game);

                    game->stats.games_played++;
                    game->stats.games_won++;
                    game->stats.current_streak++;
                    if (game->stats.current_streak > game->stats.max_streak) game->stats.max_streak = game->stats.current_streak;

                    if (game->game_mode == MODE_DAILY) {
                        time_t t = time(NULL);
                        struct tm *tm = localtime(&t);
                        int date_id = (tm->tm_year + 1900) * 1000 + tm->tm_yday;
                        game->stats.last_daily_won_yday = date_id;
                    }

                    save_stats(game);

                } else if (game->current_row == GRID_ROWS - 1) {
                    game->game_state = GAME_OVER;
                    char msg[32]; snprintf(msg, 32, "%s (Press 'R')", game->target_word);
                    set_message(game, msg);

                    game->stats.games_played++;
                    game->stats.current_streak = 0;
                    save_stats(game);

                } else {
                    game->current_row++; game->current_col = 0; game->game_state = GAME_IDLE;
                }
            }
        }
    }
}

void reset_game(GameContext* game, GameMode mode) {
    memset(game->grid, 0, sizeof(game->grid));
    for(int r=0; r<GRID_ROWS; r++) for(int c=0; c<GRID_COLS; c++) game->grid[r][c].scale_pop = 1.0f;
    init_keyboard(game);
    game->current_row = 0; game->current_col = 0;
    game->game_state = GAME_IDLE; game->won = false;
    game->message[0] = '\0'; game->shake_timer = 0;
    game->game_mode = mode;

    pick_word(game, mode);
}

void close_game(GameContext* game) {
    for(int i=0; i<game->dict_count; i++) free(game->dictionary[i]);
    free(game->dictionary);
    if(game->font_large) TTF_CloseFont(game->font_large);
    if(game->font_small) TTF_CloseFont(game->font_small);
    if(game->font_ui) TTF_CloseFont(game->font_ui);
    if(game->font_menu) TTF_CloseFont(game->font_menu);
    SDL_DestroyRenderer(game->renderer);
    SDL_DestroyWindow(game->window);
    TTF_Quit(); SDL_Quit();
}