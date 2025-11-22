#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

// --- Settings ---
#define DEFAULT_WIDTH 600
#define DEFAULT_HEIGHT 800
#define CONTENT_HEIGHT 950
#define GRID_ROWS 6
#define GRID_COLS 5
#define TILE_SIZE 62
#define TILE_PADDING 6
#define PI 3.14159265f
#define MAX_PARTICLES 200  // New Constant

// --- Timing ---
#define FLIP_SPEED 0.07f
#define SHAKE_DURATION 12.0f
#define REVEAL_DELAY 11
#define LOADING_TIME_MS 1000

// --- Colors ---
static const SDL_Color COLOR_BG            = {250, 250, 250, 255};
static const SDL_Color COLOR_BLACK         = {36, 36, 36, 255};
static const SDL_Color COLOR_WHITE         = {255, 255, 255, 255};
static const SDL_Color COLOR_EMPTY_BORDER  = {211, 214, 218, 255};
static const SDL_Color COLOR_FILLED_BORDER = {135, 138, 140, 255};
static const SDL_Color COLOR_ABSENT        = {120, 124, 126, 255};
static const SDL_Color COLOR_PRESENT       = {201, 180, 88, 255};
static const SDL_Color COLOR_CORRECT       = {106, 170, 100, 255};
static const SDL_Color COLOR_LOADING_BAR   = {106, 170, 100, 255};

// --- Enums & Structs ---
typedef enum { STATE_EMPTY, STATE_TYPING, STATE_ABSENT, STATE_PRESENT, STATE_CORRECT } TileState;
typedef enum { GAME_LOADING, GAME_MENU, GAME_ABOUT, GAME_IDLE, GAME_REVEALING, GAME_OVER } GameState;
typedef enum { MODE_DAILY, MODE_INFINITE } GameMode;

typedef struct {
    int games_played;
    int games_won;
    int current_streak;
    int max_streak;
    int last_daily_won_yday;
} GameStats;

// NEW: Particle Struct
typedef struct {
    float x, y;
    float vx, vy;
    float angle;
    float v_angle;
    SDL_Color color;
    int life;
    bool active;
} Particle;

typedef struct {
    char letter;
    TileState state;
    TileState target_state;
    float scale_pop;
    float flip_progress;
    bool flipping;
} Tile;

typedef struct {
    char key;
    SDL_Rect rect;
    TileState state;
} VirtualKey;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    int win_w, win_h;
    TTF_Font *font_large, *font_small, *font_ui, *font_menu;
    Tile grid[GRID_ROWS][GRID_COLS];
    VirtualKey keyboard[28];

    char target_word[6];
    int current_row, current_col;
    GameState game_state;
    GameMode game_mode;
    GameStats stats;

    // NEW: Particles & Toast Message
    Particle particles[MAX_PARTICLES];
    char message[64];
    Uint32 message_timer;
    float message_y;     // For float animation
    int message_alpha;   // For fade animation

    bool running, won;
    Uint32 start_time;
    float shake_timer, scroll_y;
    int reveal_col_index, reveal_timer;
    char** dictionary;
    int dict_count;

    SDL_Rect button_daily;
    SDL_Rect button_infinite;
    SDL_Rect button_about;
    SDL_Rect button_back;
} GameContext;

// --- Prototypes ---

// Logic
bool init_game(GameContext* game);
void close_game(GameContext* game);
void load_dictionary(GameContext* game);
void pick_word(GameContext* game, GameMode mode);
void init_keyboard(GameContext* game);
void reset_game(GameContext* game, GameMode mode);
void update_game(GameContext* game);
void check_word(GameContext* game);
void set_message(GameContext* game, const char* msg);
void trigger_shake(GameContext* game);
void load_stats(GameContext* game);
void save_stats(GameContext* game);
void spawn_confetti(GameContext* game); // New prototype

// Rendering
void render_game(GameContext* game);
void render_menu_screen(GameContext* game);
void render_loading_screen(GameContext* game);
void render_about_screen(GameContext* game);
void render_rounded_box(SDL_Renderer* r, SDL_Rect rect, int radius, SDL_Color c);
void render_text_simple(SDL_Renderer* r, TTF_Font* f, const char* text, int x, int y, SDL_Color color);
void render_text_in_rect(SDL_Renderer* r, TTF_Font* f, const char* text, SDL_Rect* target_rect, SDL_Color color);
void render_particles(GameContext* game); // New prototype
void render_toast(GameContext* game);     // New prototype

// Input
void process_input(GameContext* game);

#endif