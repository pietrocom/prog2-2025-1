#ifndef TYPES_H
#define TYPES_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <stdbool.h>


// Estruturas principais

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

struct Player {
    struct Entity entity;
    // 0: parado, 1: andando, 2: pulando, 3: abaixado, 4: atirando
    ALLEGRO_BITMAP *sprites[5];
    int health;
    int score;
    bool is_jumping;
    bool is_crouching;
    bool is_shooting;
    bool facing_right;
};

struct Menu {
    ALLEGRO_BITMAP *background; // Fundo do menu
    ALLEGRO_FONT *title_font;   // Fonte para o título
    ALLEGRO_FONT *text_font;    // Fonte para o resto
    int selected_option;        // Opção selecionada
    MenuState current_state; 
};

struct Enemy {
    struct Entity entity;
    ALLEGRO_BITMAP *sprite;
    int health;
    int damage;
    bool is_active;
};

struct Projectile {
    struct Entity entity;
    ALLEGRO_BITMAP *sprite;
    bool is_active;
    bool is_player_projectile;
};

struct GameLevel {
    ALLEGRO_BITMAP *background;
    float scroll_x;
    int enemy_count;
    bool boss_active;
};

#endif