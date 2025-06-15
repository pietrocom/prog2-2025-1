#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>
#include "enemy.h"
#include "game.h"
#include "menu.h"
#include "player.h"
#include "projectiles.h"
#include "types.h"


// ---- Funções Auxiliares ----

void draw_player_at_position(struct Player *player, float x, float y) {
    if (!player || !player->current_animation || 
        player->current_animation->current_frame < 0 ||
        player->current_animation->current_frame >= player->current_animation->frame_count) {
        return;
    }

    ALLEGRO_BITMAP *frame = player->current_animation->frames[player->current_animation->current_frame];
    if (!frame) return;

    int flags = player->facing_right ? 0 : ALLEGRO_FLIP_HORIZONTAL;
    al_draw_bitmap(frame, 
                  x, 
                  y - (SPRITE_SIZE - player->entity.height),
                  flags);
}


// ---- Funções de Inicialização ----

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
    
    level->ground_level = screen_h - GROUND_LEVEL;  // Chão 200px acima da base
    level->background_scale = (float)screen_h / (float)bg_height;
    level->scroll_x = 0.0f;
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


// ---- Funções de Manipulação de Eventos ----

void handle_game_events (ALLEGRO_EVENT *event, struct Player *player, struct GameLevel *level, GameState *state) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        if (event->keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            *state = MENU; 
        }
    }
}

void handle_game_over_events (ALLEGRO_EVENT *event, GameState *state) {}

void handle_pause_events (ALLEGRO_EVENT *event, GameState *state) {}


// ---- Funções de Atualização e Renderização ----

void update_game(struct Player *player, struct GameLevel *level) {
    float screen_width = al_get_display_width(al_get_current_display());
    float threshold = screen_width * 0.6f; // 60% da tela
    
    // Movimento para a direita
    if (player->entity.vel_x > 0) {
        if (player->entity.x > threshold) {
            level->scroll_x += player->entity.vel_x;
        } else {
            player->entity.x += player->entity.vel_x;
        }
    }
    // Movimento para a esquerda
    else if (player->entity.vel_x < 0) {
        if (level->scroll_x > 0) {
            level->scroll_x += player->entity.vel_x;
            if (level->scroll_x < 0) level->scroll_x = 0;
        } else {
            player->entity.x += player->entity.vel_x;
            if (player->entity.x < 50) player->entity.x = 50; // Limite esquerdo
        }
    }
    
    // Debug: mostra os valores de scroll e posição do jogador
    printf("ScrollX: %.1f, PlayerX: %.1f\n", level->scroll_x, player->entity.x);
}

void draw_game(struct Player *player, struct GameLevel *level) {
    int screen_w = al_get_display_width(al_get_current_display());
    int screen_h = al_get_display_height(al_get_current_display());
    int bg_width = al_get_bitmap_width(level->background);
    float scaled_width = bg_width * level->background_scale;
    
    // Calcula a posição de desenho do background
    float bg_offset = fmod(level->scroll_x, scaled_width);
    
    // Desenha o background repetido
    for (int i = -1; i <= (screen_w / scaled_width) + 1; i++) {
        al_draw_scaled_bitmap(level->background,
            0, 0, bg_width, al_get_bitmap_height(level->background),
            (i * scaled_width) - bg_offset, 0, scaled_width, screen_h,
            0);
    }
    
    // Desenha o jogador (posição relativa à tela)
    float player_screen_x = (player->entity.x > screen_w * 0.6f) 
                          ? screen_w * 0.6f 
                          : player->entity.x;
    
    draw_player_at_position(player, player_screen_x, player->entity.y);
}

void draw_game_over (int score) {}

void draw_pause_menu () {}
