#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"

#define PLAYER_WIDTH 40
#define PLAYER_HEIGHT 105

#define PLAYER_SCALE 1.7f

#define PLAYER_RUN_SPEED 380.0f

#define MAX_AMMO 20
#define RELOAD_TIME 2.25f

#define PLAYER_MAX_HEALTH 100

// ---- Constantes de estamina

#define MAX_STAMINA 100.0f
#define STAMINA_DEPLETION_RATE 25.0f // Pontos de estamina por segundo ao correr
#define STAMINA_REGEN_RATE 15.0f

// ---- Constantes de ajuste ----

#define PLAYER_HITBOX_OFFSET_X 0.0f  
#define PLAYER_HITBOX_OFFSET_Y 0.0f  

#define PLAYER_SPRITE_OFFSET_X 0.0f
#define PLAYER_SPRITE_OFFSET_Y 0.0f

#define PLAYER_CROUCH_HEIGHT (PLAYER_HEIGHT * 0.8f) 

#define PLAYER_MUZZLE_OFFSET_X 30.0f
#define PLAYER_MUZZLE_OFFSET_Y_STANDING 67.0f
#define PLAYER_MUZZLE_OFFSET_Y_CROUCH   46.0f


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
    int max_health;
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
    bool is_running;

    // Sistema de projetéis
    float shoot_cooldown;
    float current_shoot_cooldown;

    // Sistema de recharge
    int current_ammo; 
    int max_ammo; 
    float current_reload_time;

    // Variáveis de Estamina
    float stamina;
    float max_stamina;
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