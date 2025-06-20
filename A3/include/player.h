#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"

#define PLAYER_WIDTH 40
#define PLAYER_HEIGHT 105

#define PLAYER_HITBOX_OFFSET_X 5.0f  
#define PLAYER_HITBOX_OFFSET_Y 0.0f  

#define PLAYER_SPRITE_OFFSET_X 0.0f
#define PLAYER_SPRITE_OFFSET_Y 0.0f

#define PLAYER_SCALE 1.7f

#define MAX_FRAMES 10 

#define RUN_THRESHOLD 3.0f

#define MAX_AMMO 20
#define RELOAD_TIME 1.5f

// ---- Forward declarations ----
struct ProjectileSystem;
struct GameLevel;


// ---- Estruturas de dados básica ----

typedef enum {
    SOLDIER_1,   // Não tem crouch
    SOLDIER_2,
    SOLDIER_3
} SoldierType;

struct Player {
    struct Entity entity;
    ALLEGRO_BITMAP *sprites[5];
    SoldierType soldier_type;

    // Tamanho da sprite
    float scale;

    // Estatísticas
    int health;
    int score;

    // Fluxo de animações do player
    struct Animation *current_animation; // Aponta para a animação atual
    struct Animation idle;
    struct Animation walking;
    struct Animation running;
    struct Animation jumping;
    struct Animation shooting;
    struct Animation reloading;
    struct Animation crouching; 
    struct Animation crouch_shot;

    // Estado
    bool is_moving;
    bool is_jumping;
    bool is_crouching;
    bool is_shooting;
    bool is_reloading;
    bool facing_right;
    bool hitbox_show; 

    // Sistema de projetéis
    float shoot_cooldown;
    float current_shoot_cooldown;

    // Sistema de recharge
    int current_ammo; 
    int max_ammo; 
    float current_reload_time;
};


// ---- Funções ----

// Inicialização
void init_player(struct Player *player);
void load_player_sprites(struct Player *player);
void unload_player_sprites(struct Player *player);

// Controles
void handle_player_input(struct Player *player, ALLEGRO_EVENT *event, struct GameLevel *level);
void update_player(struct Player *player, float delta_time, struct GameLevel *level, struct ProjectileSystem *projectile_system);
void handle_player_ground_collision(struct Player *player, struct GameLevel *level);

// Estado
void damage_player(struct Player *player, int amount);
bool is_player_dead(struct Player *player);
void start_reload(struct Player *player);

// Renderização
void draw_player(struct Player *player);
void show_player_hitbox(struct Player *player);

#endif