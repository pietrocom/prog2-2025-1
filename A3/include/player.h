#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"

// Inicialização
void init_player(struct Player *player);
void load_player_sprites(struct Player *player);
void unload_player_sprites(struct Player *player);

// Controles
void handle_player_input(struct Player *player, ALLEGRO_EVENT *event);
void update_player(struct Player *player);

// Ações
void player_jump(struct Player *player);
void player_shoot(struct Player *player, struct Projectile projectiles[], int *projectile_count);
void player_crouch(struct Player *player, bool crouch);

// Estado
void damage_player(struct Player *player, int amount);
void heal_player(struct Player *player, int amount);
bool is_player_dead(struct Player *player);

// Renderização
void draw_player(struct Player *player);

#endif