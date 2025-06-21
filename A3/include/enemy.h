#ifndef ENEMY_H
#define ENEMY_H

#include "types.h"
#include "boss.h"

#define MAX_ENEMIES 20
#define MAX_ENEMY_TYPES 3
#define ENEMY_SPAWN_COOLDOWN 2.0f

#define ENEMY_WIDTH 40
#define ENEMY_HEIGHT 110

#define ENEMY_HITBOX_OFFSET_X 0.0f
#define ENEMY_HITBOX_OFFSET_Y 0.0f 
#define ENEMY_SPRITE_OFFSET_X 0.0f
#define ENEMY_SPRITE_OFFSET_Y 0.0f

#define ENEMY_SCALE 1.7f

#define SPRITE_SIZE 128

#define DEATH_FADEOUT_DELAY 2.0f

#define DETECTION_RANGE 950.0f


// ---- Forward declarations ----
struct Player;
struct Projectile;
struct ProjectileSystem;
struct GameLevel;


// ---- Estruturas de dados básica ----

// Tipos de inimigos
typedef enum {
    ENEMY_MELEE,   // Gangster 2
    ENEMY_RANGED   // Gangster 1
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
    bool is_dead;
    bool has_been_aggroed;
    float death_timer;

    // Sistema de projéteis
    float shoot_cooldown;
    float current_shoot_cooldown;
};

// Sistema de inimigos
struct EnemySystem {
    struct Enemy enemies[MAX_ENEMIES];
    struct Boss boss; 
    float spawn_timer;
    int active_count;
    int wave_number;
};

// ---- Funções ----

// Sistema
void init_enemy_system(struct EnemySystem *system);
void update_enemy_system(struct EnemySystem *system, struct Player *player, struct GameLevel *level, struct ProjectileSystem *projectile_system, float delta_time);
void spawn_enemy_wave(struct EnemySystem *system, struct GameLevel *level);
void destroy_enemy_system(struct EnemySystem *system);

// Inimigos individuais
void init_enemy(struct Enemy *enemy, EnemyType type, float x, float y);
void load_enemy_sprites(struct Enemy *enemy, EnemyType type);
void unload_enemy_sprites(struct Enemy *enemy);

// Controle
void update_enemy(struct Enemy *enemy, struct Player *player, struct GameLevel *level, struct ProjectileSystem *projectile_system, float delta_time);
void enemy_ai(struct Enemy *enemy, struct Player *player, struct GameLevel *level, struct ProjectileSystem *projectile_system, float delta_time);
void enemy_move(struct Enemy *enemy, float dx, float dy);
void enemy_attack(struct Enemy *enemy, struct Player *player, struct GameLevel *level, struct ProjectileSystem *projectile_system);
void enemy_ranged_attack(struct Enemy *enemy, struct ProjectileSystem *projectile_system);

// Estado
void damage_enemy(struct Enemy *enemy, int amount, struct Player *player);
bool is_enemy_dead(struct Enemy *enemy);
void kill_enemy(struct Enemy *enemy, struct Player *player);

// Renderização
void draw_enemy(struct Enemy *enemy, float scroll_x);
void draw_enemies(struct EnemySystem *system, struct GameLevel *level, struct Player *player);

#endif