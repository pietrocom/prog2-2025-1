#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"
#include "projectiles.h"

#define PLAYER_WIDTH 40
#define PLAYER_HEIGHT 105

#define PLAYER_RSPRITE_OFFSET_X 73.0f
#define PLAYER_LSPRITE_OFFSET_X 85.0f
#define PLAYER_HITBOX_OFFSET_X 00.0f
#define PLAYER_HITBOX_OFFSET_Y 00.0f

#define PLAYER_SCALE 1.5f

#define MAX_FRAMES 10 

#define RUN_THRESHOLD 3.0f

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
    struct Animation crouching; 
    struct Animation crouch_shot;

    // Estado
    bool is_moving;
    bool is_jumping;
    bool is_crouching;
    bool is_shooting;
    bool facing_right;
    bool hitbox_show; 
};


// ---- Funções ----

// Inicialização
void init_player(struct Player *player);
void load_player_sprites(struct Player *player);
void unload_player_sprites(struct Player *player);

// Controles
void handle_player_input(struct Player *player, ALLEGRO_EVENT *event, struct GameLevel *level);
void update_player(struct Player *player, float delta_time, struct GameLevel *level);

// Estado
void damage_player(struct Player *player, int amount);
bool is_player_dead(struct Player *player);

// Renderização
void draw_player(struct Player *player);
void draw_player_at_position(struct Player *player, float x, float y, bool hitbox_show);
void show_player_hitbox(struct Player *player);

#endif