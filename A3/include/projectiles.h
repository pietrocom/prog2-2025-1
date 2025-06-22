#ifndef PROJECTILES_H
#define PROJECTILES_H

#include "types.h"

// =================================================
// === CONSTANTES DE CONFIGURAÇÃO DO PROJÉTIL ======
// =================================================

// --- Atributos ---
#define MAX_PROJECTILES 50
#define PROJECTILE_SPEED 500.0f
#define PROJECTILE_LIFETIME 3.0f
#define PLAYER_PROJECTILE_DAMAGE 120.0f

// --- Ajustes finos ---
#define PLAYER_PROJECTILE_HITBOX_W 22.0f  
#define PLAYER_PROJECTILE_HITBOX_H 9.0f 
#define ENEMY_PROJECTILE_HITBOX_W 12.0f 
#define ENEMY_PROJECTILE_HITBOX_H 12.0f  

#define PROJECTILE_HITBOX_OFFSET_X 0.0f
#define PROJECTILE_HITBOX_OFFSET_Y 6.0f 

// --- Cooldowns ---
#define PLAYER_PROJECTILE_COOLDOWN 0.5f
#define ENEMY_PROJECTILE_COOLDOWN 2.0f

// --- Controle de escala ---
#define PLAYER_BULLET_SCALE (1.0f / 6.0f) 
#define ENEMY_BULLET_SCALE  (1.0f / 9.0f)


// =================================================
// === ESTRUTURAS E ENUMS DO PROJÉTIL ==============
// =================================================

// ---- Forward declarations ----
struct Player;
struct Enemy;
struct Boss;
struct EnemySystem;

// ---- Estruturas de dados ----
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
    ALLEGRO_BITMAP *player_bullet_sprite;
    ALLEGRO_BITMAP *enemy_bullet_sprite;
};


// =================================================
// === PROTÓTIPOS DAS FUNÇÕES ======================
// =================================================

// ---- Funções do Sistema de Projéteis ----
void init_projectile_system(struct ProjectileSystem *system);
void update_projectile_system(struct ProjectileSystem *system, float delta_time, struct Player *player, struct EnemySystem *enemy_system, struct GameLevel *level);
void draw_projectiles(struct ProjectileSystem *system, struct GameLevel *level, struct Player *player);

// ---- Funções de Controle de Projéteis ----
void spawn_projectile(struct ProjectileSystem *system, float x, float y, bool facing_right, ProjectileType type, ProjectileBehavior behavior, int damage);
void spawn_player_projectile(struct ProjectileSystem *system, struct Player *player, struct GameLevel *level);
void spawn_enemy_projectile(struct ProjectileSystem *system, struct Enemy *enemy);
void spawn_boss_projectile(struct ProjectileSystem *system, struct Boss *boss);

// ---- Funções de Colisão ----
void check_projectile_collisions(struct ProjectileSystem *system, struct Player *player, struct EnemySystem *enemy_system, struct GameLevel *level);
bool check_projectile_hit(struct Projectile *projectile, struct Entity *target);

// ---- Funções de Limpeza ----
void clear_projectiles(struct ProjectileSystem *system);
void destroy_projectile_system(struct ProjectileSystem *system);

#endif