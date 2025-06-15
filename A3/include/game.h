#ifndef GAME_H
#define GAME_H

#define MENU_TITLE_FONT_SIZE 25
#define MENU_TEXT_FONT_SIZE 15

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
void handle_pause_input(ALLEGRO_EVENT *event, GameState *state);

// ---- Controle de Game Over ----
void handle_game_over_events(ALLEGRO_EVENT *event, GameState *state);

// ---- Renderização ----
void update_game(struct Player *player, struct GameLevel *level);
void draw_game(struct Player *player, struct GameLevel *level);
void draw_game_over(int score);
void draw_pause_menu(struct GameLevel *level);

#endif