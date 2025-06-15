#ifndef PROJECTILES_H
#define PROJECTILES_H

#include "types.h"
#include "game.h"
#include "player.h"
#include "enemy.h"

#define MAX_PROJECTILES 50


// ---- Estruturas de Dados Básica ----

struct Projectile {
    struct Entity entity;
    ALLEGRO_BITMAP *sprite;
    bool is_active;
    bool is_player_projectile;
};

// Estrutura de gerenciamento
struct ProjectileSystem {
    struct Projectile projectiles[MAX_PROJECTILES];
    int count;
};


// ---- Funções ----

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
                               struct Enemy *enemies[], int enemy_count);

// Renderização
void draw_projectiles(struct ProjectileSystem *system);

#endif