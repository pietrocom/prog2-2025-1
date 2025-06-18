#ifndef GAME_H
#define GAME_H

#define PAUSE_MENU_TITLE_FONT_SIZE 54
#define PAUSE_MENU_TEXT_FONT_SIZE 24

#define BACKGROUND_THRESHOLD 0.4f

#include "types.h"
#include "player.h"
#include <allegro5/allegro.h>

// ---- Inicialização ----
void start_player(struct Player *player, struct GameLevel *level);
void start_level(struct GameLevel *level);

// ---- Limpeza ----
void destroy_player(struct Player *player);
void destroy_level(struct GameLevel *level);

// ---- Controle de Jogo ----
void handle_game_events(ALLEGRO_EVENT *event, GameState *state);

// ---- Controle de Pausa ----
void toggle_pause(GameState *current_state);
void handle_pause_input(ALLEGRO_EVENT *event, GameState *state, struct Player *player, struct GameLevel *level);

// ---- Controle de Game Over ----
void handle_game_over_events(ALLEGRO_EVENT *event, GameState *state);

// ---- Renderização ----
void reset_game(struct Player *player, struct GameLevel *level);
void update_game(struct Player *player, struct GameLevel *level);
void draw_game(struct Player *player, struct GameLevel *level);
void draw_game_over(int score);
void draw_pause_menu(struct GameLevel *level);

#endif