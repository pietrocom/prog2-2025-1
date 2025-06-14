#ifndef TYPES_H
#define TYPES_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <stdbool.h>


// Definições de constantes físicas
#define PLAYER_JUMP_VEL -8.0f
#define PLAYER_MOVE_SPEED 5.0f
#define GRAVITY 9.8f
#define GROUND_LEVEL 300.0f 



// Estruturas gerais principais

typedef enum {
    MENU_MAIN,
    MENU_OPTIONS,
    MENU_PAUSE
} MenuState;

typedef enum {
    MENU,      // Estado do menu principal
    PLAYING,   // Estado de jogo ativo
    GAME_OVER, // Estado de fim de jogo
    PAUSED     // Estado de pausa
} GameState;

struct Entity {
    float x, y;
    float width, height;
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
};

#endif