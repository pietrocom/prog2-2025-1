#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "enemy.h"
#include "game.h"
#include "menu.h"
#include "player.h"
#include "projectiles.h"
#include "types.h"


// ---- Funções de Inicialização ----

void start_player(struct Player *player, struct GameLevel *level) {
    init_player(player);  // Primeiro inicializa
    load_player_sprites(player);  // Depois carrega sprites
    
    // Posição inicial
    player->entity.x = 100.0f;
    player->entity.y = level->ground_level;
}

void start_level(struct GameLevel *level) {
    level->background = al_load_bitmap("assets/background/Street_Background/PNG/City2/Bright/City2.png");
    if (!level->background) {
        fprintf(stderr, "Falha ao carregar o background!\n");
        exit(1);
    }
    
    int screen_h = al_get_display_height(al_get_current_display());
    int bg_height = al_get_bitmap_height(level->background);
    
    level->ground_level = screen_h - GROUND_LEVEL; 
    level->background_scale = (float)screen_h / (float)bg_height;
    level->scroll_x = 0.0f;
    level->enemy_count = 0;
    level->boss_active = false;
}


// ---- Funções de Limpeza ----

void destroy_player(struct Player *player) {
    unload_player_sprites(player);
}

void destroy_level(struct GameLevel *level) {
    if (level->background) {
        al_destroy_bitmap(level->background);
    }
}


// ---- Funções de Controle do Jogo ----

void handle_game_events(ALLEGRO_EVENT *event, GameState *state) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_ESCAPE:   // Entrará na condição abaixo
            case ALLEGRO_KEY_P:
                toggle_pause(state);
                break;
                
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                *state = QUIT;
                break;
        }
    }
}


// ---- Funções de Controle de Pausa ----

void toggle_pause(GameState *current_state) {
    if (*current_state == PLAYING) {
        *current_state = PAUSED;
        // adicionar lógica adicional (pausar música)
    } 
    else if (*current_state == PAUSED) {
        *current_state = PLAYING;
    }
}

void handle_pause_input(ALLEGRO_EVENT *event, GameState *state, 
    struct Player *player, struct GameLevel *level) {

    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_ESCAPE:
            case ALLEGRO_KEY_P:
                toggle_pause(state);
                break;
                
            case ALLEGRO_KEY_M:
                // Exemplo: Mute durante a pausa
                // toggle_audio_mute();
                break;
                
            case ALLEGRO_KEY_Q:
                reset_game(player, level);
                *state = MENU;  // Volta ao menu
                break;
        }
    }
}


// ---- Funções de Controle de Game Over ----

void handle_game_over_events (ALLEGRO_EVENT *event, GameState *state) {}


// ---- Funções de Atualização e Renderização ----

void reset_game(struct Player *player, struct GameLevel *level) {
    init_player(player);  // Reinicializa o jogador

    player->entity.x = 100.0f;
    player->entity.y = level->ground_level;

    int screen_h = al_get_display_height(al_get_current_display());
    int bg_height = al_get_bitmap_height(level->background);
    
    level->ground_level = screen_h - GROUND_LEVEL; 
    level->background_scale = (float)screen_h / (float)bg_height;
    level->scroll_x = 0.0f;
    level->enemy_count = 0;
    level->boss_active = false;
}

// Tanto o jogador quanto o background vao se mover
// dependendo da posição do jogador na tela
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
    
    // Mostra coordenadas X do jogador e do scroll
    // printf("ScrollX: %.1f, PlayerX: %.1f\n", level->scroll_x, player->entity.x);
}

// Relativiza a posição do jogador na tela
void draw_game(struct Player *player, struct GameLevel *level) {
    int screen_w = al_get_display_width(al_get_current_display());
    int screen_h = al_get_display_height(al_get_current_display());
    int bg_width = al_get_bitmap_width(level->background);
    float scaled_width = bg_width * level->background_scale;
    
    // Calcula a posição de desenho do background
    float bg_offset = fmod(level->scroll_x, scaled_width);  // Resto da divisao de floats
    
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
    
    draw_player_at_position(player, player_screen_x, player->entity.y, player->hitbox_show);
    
    draw_ground_line(level);  
}

void draw_game_over (int score) {}

void draw_pause_menu(struct GameLevel *level) {
    if (!al_is_primitives_addon_initialized()) {
        fprintf(stderr, "Primitives addon not initialized!\n");
        exit(1);
    }

    // Configurações
    ALLEGRO_COLOR bg_color = al_map_rgba(0, 0, 0, 200);  // Fundo semi-transparente
    ALLEGRO_COLOR box_color = al_map_rgba(70, 70, 90, 220);  // Cor das caixas de texto
    ALLEGRO_COLOR text_color = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR highlight_color = al_map_rgb(180, 180, 255);  // Cor de destaque
    
    // Use sua fonte customizada ou a built-in
    ALLEGRO_FONT *font = al_load_font("assets/fonts/DirtyWar.otf", MENU_TEXT_FONT_SIZE, 0);
    ALLEGRO_FONT *title_font = al_load_font("assets/fonts/DirtyWar.otf", MENU_TITLE_FONT_SIZE, 0);
    
    // Dimensões
    int display_w = al_get_display_width(al_get_current_display());
    int display_h = al_get_display_height(al_get_current_display());
    int box_width = 600;
    int box_height = 400;
    int box_padding = 20;
    int text_height = al_get_font_line_height(font);

    if (!al_is_primitives_addon_initialized()) {
        fprintf(stderr, "ERRO: Primitives não inicializado no draw!\n");
        exit(1);
    }

    // 1. Desenha overlay escuro
    al_draw_filled_rectangle(0, 0, display_w, display_h, bg_color);

    // 2. Caixa principal centralizada
    al_draw_filled_rounded_rectangle(
        display_w/2 - box_width/2, 
        display_h/2 - box_height/2,
        display_w/2 + box_width/2,
        display_h/2 + box_height/2,
        10, 10, box_color);
    
    // Borda da caixa principal
    al_draw_rounded_rectangle(
        display_w/2 - box_width/2, 
        display_h/2 - box_height/2,
        display_w/2 + box_width/2,
        display_h/2 + box_height/2,
        10, 10, highlight_color, 3);

    // 3. Título
    al_draw_text(
        title_font, 
        highlight_color, 
        display_w/2, 
        display_h/2 - 160, 
        ALLEGRO_ALIGN_CENTRE, 
        "JOGO PAUSADO");

    // 4. Opções
    int y_offset = display_h/2 - box_height/2 + box_padding * 3;
    
    // Opção 1 - Continuar
    al_draw_text(
        font, 
        text_color, 
        display_w/2, 
        display_h/2 + 35, 
        ALLEGRO_ALIGN_CENTRE, 
        "''P'' Continuar");
    
    // Opção 2 - Voltar ao menu
    al_draw_text(
        font, 
        text_color, 
        display_w/2, 
        display_h/2, 
        ALLEGRO_ALIGN_CENTRE, 
        "''Q'' Voltar ao menu");

    // 5. Limpeza
    al_destroy_font(font);
}
