#ifndef GAME_H
#define GAME_H

#define PAUSE_MENU_TITLE_FONT_SIZE 54
#define PAUSE_MENU_TEXT_FONT_SIZE 24

#define GAMEOVER_TITLE_FONT_SIZE 72
#define GAMEOVER_TEXT_FONT_SIZE 32

#define BACKGROUND_THRESHOLD 0.4f

#include "types.h"
#include "player.h"
#include "enemy.h"
#include "projectiles.h"
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
void handle_pause_input(ALLEGRO_EVENT *event, GameState *state, struct Player *player, struct GameLevel *level, struct EnemySystem *enemy_system, struct ProjectileSystem *projectile_system);

// ---- Controle de Game Over ----
void handle_game_over_input(ALLEGRO_EVENT *event, GameState *state, struct Player *player, struct GameLevel *level, struct EnemySystem *enemy_system, struct ProjectileSystem *projectile_system);

// ---- Renderização ----
void reset_game(struct Player *player, struct GameLevel *level, struct EnemySystem *enemy_system, struct ProjectileSystem *projectile_system);
void update_game(struct Player *player, struct GameLevel *level, float delta_time);
void draw_game(struct Player *player, struct GameLevel *level);
void draw_game_over_screen(struct Player *player, struct GameLevel *level);
void draw_pause_menu();
void draw_hud(struct Player *player, struct GameLevel *level);

#endif