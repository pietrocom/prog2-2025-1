#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"
#include "projectiles.h"

#define PLAYER_WIDTH 10
#define PLAYER_HEIGHT 10
#define MAX_FRAMES 10 
#define SPRITE_SIZE 128
#define RUN_THRESHOLD 3.0f

// ---- Estruturas de dados básica ----

typedef enum {
    SOLDIER_1,   // Não tem crouch
    SOLDIER_2,
    SOLDIER_3
} SoldierType;

struct Animation {
    ALLEGRO_BITMAP *frames[MAX_FRAMES];
    int frame_count;      // Quantos frames tem a animação
    int current_frame;    // Frame atual sendo exibido
    float frame_delay;    // Tempo entre frames (em segundos)
    float elapsed_time;   // Tempo acumulado desde a última troca de frame
};

struct Player {
    struct Entity entity;
    ALLEGRO_BITMAP *sprites[5];
    SoldierType soldier_type;

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
};


// ---- Funções ----

// Inicialização
void init_player(struct Player *player);
void load_player_sprites(struct Player *player);
void unload_player_sprites(struct Player *player);

// Controles
void handle_player_input(struct Player *player, ALLEGRO_EVENT *event);
void update_player(struct Player *player, float delta_time, struct GameLevel *level);

// Estado
void damage_player(struct Player *player, int amount);
bool is_player_dead(struct Player *player);

// Renderização
void draw_player(struct Player *player);

#endif