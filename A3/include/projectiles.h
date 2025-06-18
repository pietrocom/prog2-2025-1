#ifndef PROJECTILES_H
#define PROJECTILES_H

#include "types.h"

#define MAX_PROJECTILES 50
#define PROJECTILE_SPEED 300.0f
#define PROJECTILE_WIDTH 10.0f
#define PROJECTILE_HEIGHT 5.0f
#define PLAYER_PROJECTILE_COOLDOWN 0.5f
#define ENEMY_PROJECTILE_COOLDOWN 2.0f

struct Projectile {
    struct Entity entity;
    ProjectileType type;
    ProjectileBehavior behavior;
    bool is_active;
    int damage;
    float lifetime;
    float max_lifetime;
    ALLEGRO_BITMAP *sprite;
    ALLEGRO_COLOR color;
};

struct ProjectileSystem {
    struct Projectile projectiles[MAX_PROJECTILES];
    int active_count;
};

// ---- Forward declarations ----
struct Player;
struct Enemy;
struct EnemySystem;


// ---- Funções do Sistema de Projéteis ----

void init_projectile_system(struct ProjectileSystem *system);
void update_projectile_system(struct ProjectileSystem *system, float delta_time, struct Player *player, struct EnemySystem *enemy_system);
void draw_projectiles(struct ProjectileSystem *system);

// ---- Funções de Controle de Projéteis ----

void spawn_projectile(struct ProjectileSystem *system, float x, float y, bool facing_right, ProjectileType type, ProjectileBehavior behavior, int damage);
void spawn_player_projectile(struct ProjectileSystem *system, struct Player *player);
void spawn_enemy_projectile(struct ProjectileSystem *system, struct Enemy *enemy);

// ---- Funções de Colisão ----

void check_projectile_collisions(struct ProjectileSystem *system, struct Player *player, struct EnemySystem *enemy_system);
bool check_projectile_hit(struct Projectile *projectile, struct Entity *target);

// ---- Funções de Limpeza ----

void clear_projectiles(struct ProjectileSystem *system);
void destroy_projectile_system(struct ProjectileSystem *system);

#endif