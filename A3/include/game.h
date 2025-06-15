#ifndef GAME_H
#define GAME_H

#include "types.h"
#include "player.h"
#include <allegro5/allegro.h>

void start_player(struct Player *player);
void start_level(struct GameLevel *level);
void destroy_player(struct Player *player);
void destroy_level(struct GameLevel *level);
void handle_game_events(ALLEGRO_EVENT *event, struct Player *player, 
                       struct GameLevel *level, GameState *state);
void update_game(struct Player *player, struct GameLevel *level);
void draw_game(struct Player *player, struct GameLevel *level);
void handle_game_over_events(ALLEGRO_EVENT *event, GameState *state);
void draw_game_over(int score);
void handle_pause_events(ALLEGRO_EVENT *event, GameState *state);
void draw_pause_menu();

#endif