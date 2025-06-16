#ifndef TYPES_H
#define TYPES_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <stdbool.h>


// Definições de constantes físicas
#define PLAYER_JUMP_VEL -100.0f
#define PLAYER_MOVE_SPEED 4.0f
#define GRAVITY 70.0f
#define GROUND_LEVEL 200 // Distância do teto até a base da tela



// Estruturas gerais principais

typedef enum {
    MENU_MAIN,
    MENU_OPTIONS,
    MENU_PAUSE
} MenuState;

typedef enum {
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER,
    QUIT
} GameState;

struct Entity {
    float x, y;          // Posição do canto inferior esquerdo (base do personagem)
    float width, height; 
    
    // Sistema de hitbox 
    struct {
        float x, y;               // Posição do canto inferior esquerdo da hitbox
        float width, height;
        float offset_x, offset_y; // Deslocamento relativo à entidade
    } hitbox;
    
    float vel_x, vel_y;
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
};

#endif