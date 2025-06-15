#include <stdio.h>
#include <allegro5/allegro.h>
#include "enemy.h"
#include "game.h"
#include "menu.h"
#include "player.h"
#include "projectiles.h"
#include "types.h"

void start_player(struct Player *player) {
    init_player(player);  // Primeiro inicializa
    load_player_sprites(player);  // Depois carrega sprites
    
    // Posição inicial
    player->entity.x = 100.0f;
    player->entity.y = GROUND_LEVEL - player->entity.height;
}

void start_level (struct GameLevel *level) {
    // Carrega o background principal
    level->background = al_load_bitmap("assets/background/Background_1.png");
    if (!level->background) {
        fprintf(stderr, "Falha ao carregar o background!\n");
        exit(1);
    }

    // Inicializa rolagem
    level->scroll_x = 0;
    
    level->enemy_count = 0;
    level->boss_active = false;
}

void destroy_player(struct Player *player) {
    unload_player_sprites(player);
}

void destroy_level(struct GameLevel *level) {
    if (level->background) {
        al_destroy_bitmap(level->background);
    }
}

void handle_game_events (ALLEGRO_EVENT *event, struct Player *player, struct GameLevel *level, GameState *state) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        if (event->keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            *state = MENU; // Volta ao menu
        }
    }
}

void update_game(struct Player *player, struct GameLevel *level) {
    // Atualiza a posição do background baseado no movimento do jogador
    if (player->entity.vel_x != 0) {
        level->scroll_x += player->entity.vel_x * 0.5f; // Ajuste a velocidade da rolagem
    }
}

void draw_game(struct Player *player, struct GameLevel *level) {
    int screen_w = al_get_display_width(al_get_current_display());
    int screen_h = al_get_display_height(al_get_current_display());
    
    if (level->background) {
        // Obtém as dimensões do background
        int bg_width = al_get_bitmap_width(level->background);
        int bg_height = al_get_bitmap_height(level->background);
        
        // Calcula a escala necessária para cobrir a altura da tela
        float scale = (float)screen_h / (float)bg_height;
        
        // Desenha o background escalado
        al_draw_scaled_bitmap(level->background,
                            0, 0, bg_width, bg_height,  // Retângulo de origem
                            0, 0, bg_width * scale, screen_h,  // Retângulo de destino
                            0);
    }
    
    // Desenha o jogador
    draw_player(player);
}

void handle_game_over_events (ALLEGRO_EVENT *event, GameState *state) {}

void draw_game_over (int score) {}

void handle_pause_events (ALLEGRO_EVENT *event, GameState *state) {}

void draw_pause_menu () {}
