#include <stdio.h>
#include <allegro5/allegro.h>
#include "enemy.h"
#include "game.h"
#include "menu.h"
#include "player.h"
#include "projectiles.h"
#include "types.h"

void start_player(struct Player *player, struct GameLevel *level) {
    init_player(player);  // Primeiro inicializa
    load_player_sprites(player);  // Depois carrega sprites
    
    // Posição inicial
    player->entity.x = 100.0f;
    player->entity.y = level->ground_level - player->entity.height;
}

void start_level(struct GameLevel *level) {
    level->background = al_load_bitmap("assets/background/Street_Background/PNG/City2/Bright/City2.png");
    if (!level->background) {
        fprintf(stderr, "Falha ao carregar o background!\n");
        exit(1);
    }
    
    int screen_h = al_get_display_height(al_get_current_display());
    int bg_height = al_get_bitmap_height(level->background);
    
    level->ground_level = screen_h - 200;  // Chão 200px acima da base
    level->background_scale = (float)screen_h / (float)bg_height;
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
    // Movimento horizontal com rolagem de câmera
    float threshold = al_get_display_width(al_get_current_display()) / 3;
    
    if (player->entity.x > threshold && player->entity.vel_x > 0) {
        level->scroll_x += player->entity.vel_x;
        player->entity.x = threshold;  // Mantém player na posição relativa
    }
    else if (player->entity.x < 100 && player->entity.vel_x < 0) {
        level->scroll_x += player->entity.vel_x;
        if (level->scroll_x < 0) level->scroll_x = 0;
        player->entity.x = 100;
    }
}

void draw_game(struct Player *player, struct GameLevel *level) {
    int screen_w = al_get_display_width(al_get_current_display());
    int screen_h = al_get_display_height(al_get_current_display());
    int bg_width = al_get_bitmap_width(level->background);
    int scaled_width = bg_width * level->background_scale;
    
    // Desenha o background repetido
    int first_bg = (int)level->scroll_x % scaled_width;
    if (first_bg > 0) first_bg -= scaled_width;
    
    for (int offset = first_bg; offset < screen_w; offset += scaled_width) {
        al_draw_scaled_bitmap(level->background,
            0, 0, bg_width, al_get_bitmap_height(level->background),
            offset, 0, scaled_width, screen_h,
            0);
    }
    
    // Desenha o jogador (posição relativa à tela)
    draw_player(player);
    
    // Opcional: desenha uma linha do chão para debug
    al_draw_line(0, level->ground_level, screen_w, level->ground_level, al_map_rgb(255,0,0), 2);
}

void handle_game_over_events (ALLEGRO_EVENT *event, GameState *state) {}

void draw_game_over (int score) {}

void handle_pause_events (ALLEGRO_EVENT *event, GameState *state) {}

void draw_pause_menu () {}
