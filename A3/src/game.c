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
    
    // Se quiser parallax (opcional):
    /*
    level->parallax_layers[0] = al_load_bitmap("assets/background/parallax_far.png");
    level->parallax_layers[1] = al_load_bitmap("assets/background/parallax_mid.png");
    level->parallax_layers[2] = al_load_bitmap("assets/background/parallax_near.png");
    level->parallax_scroll[0] = 0;
    level->parallax_scroll[1] = 0;
    level->parallax_scroll[2] = 0;
    */
    
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
    
    // Se estiver usando parallax:
    /*
    for (int i = 0; i < 3; i++) {
        if (level->parallax_layers[i]) {
            al_destroy_bitmap(level->parallax_layers[i]);
        }
    }
    */
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
        
        // Se estiver usando parallax:
        /*
        level->parallax_scroll[0] += player->entity.vel_x * 0.2f; // Layer mais distante (mais lento)
        level->parallax_scroll[1] += player->entity.vel_x * 0.5f;
        level->parallax_scroll[2] += player->entity.vel_x * 0.8f; // Layer mais próximo (mais rápido)
        */
    }
}

void draw_game(struct Player *player, struct GameLevel *level) {
    int screen_w = al_get_display_width(al_get_current_display());
    int screen_h = al_get_display_height(al_get_current_display());
    
    // Desenha o background (com rolagem)
    if (level->background) {
        // Calcula quantas vezes precisamos repetir o background
        int bg_width = al_get_bitmap_width(level->background);
        int repeat_times = (screen_w / bg_width) + 2;
        
        // Desenha o background repetido
        for (int i = 0; i < repeat_times; i++) {
            al_draw_bitmap(level->background, 
                          (i * bg_width) - (int)level->scroll_x % bg_width, 
                          0, 0);
        }
        
        // Se estiver usando parallax:
        /*
        for (int layer = 0; layer < 3; layer++) {
            if (level->parallax_layers[layer]) {
                int layer_width = al_get_bitmap_width(level->parallax_layers[layer]);
                int layer_repeat = (screen_w / layer_width) + 2;
                
                for (int i = 0; i < layer_repeat; i++) {
                    al_draw_bitmap(level->parallax_layers[layer],
                                 (i * layer_width) - (int)level->parallax_scroll[layer] % layer_width,
                                 0, 0);
                }
            }
        }
        */
    }
    
    // Desenha o jogador (sobre o background)
    draw_player(player);
}

void handle_game_over_events (ALLEGRO_EVENT *event, GameState *state) {}

void draw_game_over (int score) {}

void handle_pause_events (ALLEGRO_EVENT *event, GameState *state) {}

void draw_pause_menu () {}
