#include "constants.h"

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    GameContext game;
    memset(&game, 0, sizeof(GameContext));

    if (!init_game(&game)) return 1;

    load_dictionary(&game);

    // FIX: Use the new function name 'pick_word' and pass a default mode
    pick_word(&game, MODE_DAILY);

    init_keyboard(&game);

    // Init grid scales
    for(int r=0; r<GRID_ROWS; r++)
        for(int c=0; c<GRID_COLS; c++)
            game.grid[r][c].scale_pop = 1.0f;

    game.start_time = SDL_GetTicks();

    const int FPS = 60;
    const int frameDelay = 1000 / FPS;
    Uint32 frameStart, frameTime;

    while (game.running) {
        frameStart = SDL_GetTicks();

        // FIX: Use dot operator (.) because 'game' is a struct, not a pointer
        SDL_GetWindowSize(game.window, &game.win_w, &game.win_h);

        process_input(&game);
        update_game(&game);

        if (game.game_state == GAME_LOADING) render_loading_screen(&game);
        else if (game.game_state == GAME_MENU) render_menu_screen(&game);
        else if (game.game_state == GAME_ABOUT) render_about_screen(&game);
        else render_game(&game);

        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) SDL_Delay(frameDelay - frameTime);
    }

    close_game(&game);
    return 0;
}