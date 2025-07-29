
// Módulo que contém estruturas de dados gerais utilizadas pelos outros módulos.

#ifndef TYPES_H
#define TYPES_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <stdbool.h>

// =================================================
// === CONSTANTES DE CONFIGURAÇÃO GERAIS ===========
// =================================================

// Definições de constantes físicas
#define PLAYER_JUMP_VEL -1200.0f
#define PLAYER_MOVE_SPEED 240.0f
#define GRAVITY 2600.0f
#define GROUND_LEVEL 150 // Distância do teto até a base da tela

#define MAX_FRAMES 14 

#define SPRITE_SIZE 128


// =================================================
// === ESTRUTURAS E ENUMS GERAIS ===================
// =================================================

// ---- Estados do Menu ----
typedef enum {
    MENU_MAIN,
    MENU_OPTIONS,
    MENU_PAUSE
} MenuState;

// ---- Estados do Jogo ----
typedef enum {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER,
    QUIT
} GameState;

// ---- Estruturas de Dados ----
struct Entity {
    float x, y;
    float width, height; 
    
    // Sistema de hitbox 
    struct {
        float x, y;
        float width, height;
        float offset_x, offset_y; // Deslocamento relativo à entidade
    } hitbox;
    
    float vel_x, vel_y;
};

struct Animation {
    ALLEGRO_BITMAP *frames[MAX_FRAMES];
    int frame_count;      // Quantos frames tem a animação
    int current_frame;    // Frame atual sendo exibido
    float frame_delay;    // Tempo entre frames (em segundos)
    float elapsed_time;   // Tempo acumulado desde a última troca de frame
};

struct Menu {
    ALLEGRO_BITMAP *background;
    ALLEGRO_FONT *title_font;
    ALLEGRO_FONT *text_font;
    int main_menu_selection;    // Seleção no menu principal (0-2)
    int options_menu_selection; // Seleção no menu de opções (0-X)
    MenuState current_state;
};

struct GameLevel {
    ALLEGRO_BITMAP *background;
    float scroll_x;
    int enemy_count;
    bool boss_active;
    float ground_level;
    float background_scale;
    bool draw_ground_line; 

    double game_time; 
    ALLEGRO_FONT *hud_font;

    bool player_won;
    int game_over_selection;
};

typedef enum {
    PROJECTILE_PLAYER,
    PROJECTILE_ENEMY
} ProjectileType;

typedef enum {
    PROJECTILE_NORMAL,
    PROJECTILE_EXPLOSIVE,
    PROJECTILE_PIERCING
} ProjectileBehavior;

#endif