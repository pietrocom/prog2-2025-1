#include "game.h"
#include "player.h"

void init_player (struct Player *player) {}

void init_level (struct GameLevel *level) {}

void destroy_player (struct Player *player) {}

void destroy_level (struct GameLevel *level) {}

void handle_game_events (ALLEGRO_EVENT *event, struct Player *player, 
                      struct GameLevel *level, GameState *state) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        if (event->keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            *state = MENU; // Volta ao menu
        }
    }
}

void update_game (struct Player *player, struct GameLevel *level) {
    // Lógica básica de atualização
}

void draw_game (struct Player *player, struct GameLevel *level) {
    // Desenho básico
    al_clear_to_color(al_map_rgb(0, 0, 50));
    al_draw_textf(al_create_builtin_font(), al_map_rgb(255, 255, 255),
                 400, 300, 0, "Jogo em desenvolvimento");
}
void handle_game_over_events (ALLEGRO_EVENT *event, GameState *state) {}

void draw_game_over (int score) {}

void handle_pause_events (ALLEGRO_EVENT *event, GameState *state) {}

void draw_pause_menu () {}
