#ifndef ENEMY_H
#define ENEMY_H

#include "types.h"
#include "player.h"
#include "projectiles.h"

#define MAX_ENEMIES 20
#define MAX_ENEMY_TYPES 3
#define ENEMY_SPAWN_COOLDOWN 2.0f
#define MAX_FRAMES 10

#define ENEMY_WIDTH 40
#define ENEMY_HEIGHT 105
#define ENEMY_HITBOX_OFFSET_X 00.0f
#define ENEMY_HITBOX_OFFSET_Y 00.0f

#define ENEMY_SCALE 1.5f

#define SPRITE_SIZE 128

// ---- Estruturas de dados básica ----

// Tipos de inimigos
typedef enum {
    ENEMY_MELEE,   // Gangster 2
    ENEMY_RANGED,  // Gangster 1
    ENEMY_BOSS 
} EnemyType;

// Estados de animação do inimigo
typedef enum {
    ENEMY_ANIM_IDLE,
    ENEMY_ANIM_WALK,
    ENEMY_ANIM_ATTACK,
    ENEMY_ANIM_HURT,
    ENEMY_ANIM_DEATH,
    ENEMY_ANIM_COUNT    // Adicionado para automação
} EnemyAnimState;

struct Enemy {
    struct Entity entity;
    EnemyType type;
    
    struct Animation *current_animation;
    struct Animation animations[ENEMY_ANIM_COUNT];
    
    // Atributos
    int health;
    int max_health;
    int damage;
    float speed;
    float attack_range;
    float detection_range;
    float attack_cooldown;
    float current_cooldown;
    
    // Controle
    bool is_active;
    bool facing_right;
    bool is_attacking;
    bool hitbox_show;
};

// Sistema de inimigos
struct EnemySystem {
    struct Enemy enemies[MAX_ENEMIES];
    struct Enemy boss;
    float spawn_timer;
    int active_count;
    int wave_number;
};

// ---- Funções ----

// Sistema
void init_enemy_system(struct EnemySystem *system);
void update_enemy_system(struct EnemySystem *system, struct Player *player, struct GameLevel *level, float delta_time);
void spawn_enemy_wave(struct EnemySystem *system, struct GameLevel *level);

// Inimigos individuais
void init_enemy(struct Enemy *enemy, EnemyType type, float x, float y);
void load_enemy_sprites(struct Enemy *enemy, EnemyType type);
void unload_enemy_sprites(struct Enemy *enemy);

// Controle
void update_enemy(struct Enemy *enemy, struct Player *player, float delta_time);
void enemy_ai(struct Enemy *enemy, struct Player *player, float delta_time);
void enemy_move(struct Enemy *enemy, float dx, float dy);
void enemy_attack(struct Enemy *enemy, struct Player *player);
void enemy_ranged_attack(struct Enemy *enemy, struct Projectile *projectiles[], int *projectile_count);

// Estado
void damage_enemy(struct Enemy *enemy, int amount);
bool is_enemy_dead(struct Enemy *enemy);
void kill_enemy(struct Enemy *enemy);

// Renderização
void draw_enemy(struct Enemy *enemy);
void draw_enemy_health_bar(struct Enemy *enemy);

// Chefe
void init_boss(struct Enemy *boss);
void update_boss(struct Enemy *boss, struct Player *player, float delta_time);
void boss_attack_pattern(struct Enemy *boss, struct Player *player, struct Projectile *projectiles[], int *projectile_count);

#endif