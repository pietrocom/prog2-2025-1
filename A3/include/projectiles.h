#ifndef PROJECTILES_H
#define PROJECTILES_H

#include "types.h"

#define MAX_PROJECTILES 50

// Estrutura de gerenciamento
struct ProjectileSystem {
    struct Projectile projectiles[MAX_PROJECTILES];
    int count;
};

// Inicialização
void init_projectile_system(struct ProjectileSystem *system);

// Controle
void add_projectile(struct ProjectileSystem *system, float x, float y, 
                   float vel_x, float vel_y, bool is_player_projectile);
void update_projectiles(struct ProjectileSystem *system);
void clear_projectiles(struct ProjectileSystem *system);

// Colisão
void check_projectile_collisions(struct ProjectileSystem *projectiles, 
                               struct Player *player, 
                               struct Enemy enemies[], int enemy_count);

// Renderização
void draw_projectiles(struct ProjectileSystem *system);

#endif