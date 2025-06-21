#include <stdio.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "types.h"
#include "game.h"
#include "player.h"
#include "enemy.h"
#include "utils.h"
#include "menu.h"
#include "projectiles.h"


// ---- Funções de Inicialização ----

void start_player(struct Player *player, struct GameLevel *level) {
    init_player(player);
    load_player_sprites(player);
    
    // Posição inicial 
    player->entity.x = 100.0f;
    player->entity.y = level->ground_level;
    
    update_hitbox_position(&player->entity, player->facing_right);
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
    level->game_time = 0.0; 

    // Carrega a fonte para o HUD
    level->hud_font = al_load_font("assets/fonts/DirtyWar.otf", 36, 0);
    if (!level->hud_font) {
        fprintf(stderr, "Falha ao carregar a fonte do HUD!\n");
    }
}


// ---- Funções de Limpeza ----

void destroy_player(struct Player *player) {
    unload_player_sprites(player);
}

void destroy_level(struct GameLevel *level) {
    if (level->background) {
        al_destroy_bitmap(level->background);
    }
    if (level->hud_font) {
        al_destroy_font(level->hud_font);
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
    struct Player *player, struct GameLevel *level, struct EnemySystem *enemy_system, struct ProjectileSystem *projectile_system) {

    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_ESCAPE:
            case ALLEGRO_KEY_P:
                toggle_pause(state);
                break;
                
            case ALLEGRO_KEY_M:
                break;
            
            case ALLEGRO_KEY_Q:
                reset_game(player, level, enemy_system, projectile_system);
                *state = MENU;  // Volta ao menu
                break;
        }
    }
}


// ---- Funções de Controle de Game Over ----

void handle_game_over_input(ALLEGRO_EVENT *event, GameState *state, 
    struct Player *player, struct GameLevel *level, struct EnemySystem *enemy_system, struct ProjectileSystem *projectile_system) {
    
    if (event->type != ALLEGRO_EVENT_KEY_DOWN) return;

    switch (event->keyboard.keycode) {
        case ALLEGRO_KEY_UP:
        case ALLEGRO_KEY_DOWN:
            level->game_over_selection = (level->game_over_selection + 1) % 2; // Alterna entre 0 e 1
            break;

        case ALLEGRO_KEY_ENTER:
            if (level->game_over_selection == 0) { // Primeira opção
                if (level->player_won) {
                    // MODO FREEPLAY
                    *state = PLAYING;
                    // O jogo continua de onde parou, o sistema de inimigos irá gerar um novo chefe.
                } else {
                    // REINICIAR
                    reset_game(player, level, enemy_system, projectile_system);
                    *state = PLAYING;
                }
            } else { // Segunda opção (Sair para o Menu)
                reset_game(player, level, enemy_system, projectile_system);
                *state = MENU;
            }
            break;
    }
}


// ---- Funções de Atualização e Renderização ----

void reset_game(struct Player *player, struct GameLevel *level, struct EnemySystem *enemy_system, struct ProjectileSystem *projectile_system) {
    printf("Reiniciando o jogo por completo...\n");

    // Reinicializa o jogador
    init_player(player);
    player->entity.x = 100.0f;
    
    // Reinicializa o level
    int screen_h = al_get_display_height(al_get_current_display());
    int bg_height = al_get_bitmap_height(level->background);
    
    level->ground_level = screen_h - GROUND_LEVEL; 
    player->entity.y = level->ground_level; // Posição do jogador depende do ground_level
    level->background_scale = (float)screen_h / (float)bg_height;
    level->scroll_x = 0.0f;
    level->enemy_count = 0;
    level->boss_active = false;
    level->game_time = 0.0; // Reinicia o tempo de jogo

    // Reinicializa o sistema de inimigos
    destroy_enemy_system(enemy_system);
    init_enemy_system(enemy_system);

    init_projectile_system(projectile_system);
}

// Tanto o jogador quanto o background vao se mover
// dependendo da posição do jogador na tela
void update_game(struct Player *player, struct GameLevel *level, float delta_time) {
    level->game_time += delta_time;

    float screen_width = al_get_display_width(al_get_current_display());
    
    float left_threshold = screen_width * (1 - BACKGROUND_THRESHOLD); 
    float right_threshold = screen_width * BACKGROUND_THRESHOLD;

    float player_screen_pos = player->entity.x;
    
    float move_delta = player->entity.vel_x * delta_time;

    // Movimento para a direita (esta parte já estava correta)
    if (player->entity.vel_x > 0) {
        if (player_screen_pos > right_threshold) {
            level->scroll_x += move_delta;
        } else {
            player->entity.x += move_delta;
        }
    }
    // Movimento para a esquerda
    else if (player->entity.vel_x < 0) {
        if (player_screen_pos < left_threshold && level->scroll_x > 0) {
            level->scroll_x += move_delta;
            // Garante que o scroll não ultrapasse o limite 0 neste mesmo quadro
            if (level->scroll_x < 0) {
                level->scroll_x = 0;
            }
        } else {
            // Se a câmera estiver no limite (ou o jogador fora da zona de scroll),
            // movemos o próprio jogador na tela.
            player->entity.x += move_delta;
        }
    }

    // Limites para o jogador não sair da tela
    if (player->entity.x < 0) player->entity.x = 0;
    if (player->entity.x + player->entity.width > screen_width) player->entity.x = screen_width - player->entity.width;
}


// Relativiza a posição do jogador na tela
void draw_game(struct Player *player, struct GameLevel *level) {
    int screen_w = al_get_display_width(al_get_current_display());
    int screen_h = al_get_display_height(al_get_current_display());
    int bg_width = al_get_bitmap_width(level->background);
    float scaled_bg_width = bg_width * level->background_scale;
    
    float bg_offset = fmod(level->scroll_x, scaled_bg_width);
    

    // Desenha o background repetido
    for (int i = -1; i <= (screen_w / scaled_bg_width) + 1; i++) {
        al_draw_scaled_bitmap(level->background,
            0, 0, bg_width, al_get_bitmap_height(level->background),
            (i * scaled_bg_width) - bg_offset, 0, scaled_bg_width, screen_h,
            0);
    }
    
    // A posição do player (player->entity.x) já é sua posição na tela.
    // A função update_game cuida de mover o scroll ou o player.
    // Então, podemos simplesmente desenhar o player em sua posição atual.
    draw_player(player);
    
    if (level->draw_ground_line) {
        draw_ground_line(level);
    }
}

void draw_game_over_screen(struct Player *player, struct GameLevel *level) {
    ALLEGRO_COLOR bg_color = al_map_rgba(0, 0, 0, 210);
    ALLEGRO_COLOR title_color, text_color, highlight_color;
    const char *title_text;
    const char *option1_text;
    
    ALLEGRO_FONT *title_font = al_load_font("assets/fonts/DirtyWar.otf", GAMEOVER_TITLE_FONT_SIZE, 0);
    ALLEGRO_FONT *text_font = al_load_font("assets/fonts/DirtyWar.otf", GAMEOVER_TEXT_FONT_SIZE, 0);
    
    int display_w = al_get_display_width(al_get_current_display());
    int display_h = al_get_display_height(al_get_current_display());

    // Configura textos e cores com base na vitória ou derrota
    if (level->player_won) {
        title_text = "YOU WON!";
        option1_text = "Freeplay";
        title_color = al_map_rgb(255, 215, 0); // Dourado
        highlight_color = al_map_rgb(173, 255, 47); // Verde claro
    } else {
        title_text = "GAME OVER";
        option1_text = "Reiniciar";
        title_color = al_map_rgb(200, 0, 0); // Vermelho escuro
        highlight_color = al_map_rgb(255, 100, 100); // Vermelho claro
    }
    text_color = al_map_rgb(220, 220, 220); // Branco acinzentado

    // Desenha o fundo semi-transparente
    al_draw_filled_rectangle(0, 0, display_w, display_h, bg_color);
    
    // Título (VOCÊ VENCEU ou GAME OVER)
    al_draw_text(title_font, title_color, display_w / 2, display_h * 0.2, ALLEGRO_ALIGN_CENTER, title_text);

    // Estatísticas Finais
    int minutes = (int)level->game_time / 60;
    int seconds = (int)level->game_time % 60;
    al_draw_textf(text_font, text_color, display_w / 2, display_h * 0.45, ALLEGRO_ALIGN_CENTER, "Pontuacao final: %06d", player->score);
    al_draw_textf(text_font, text_color, display_w / 2, display_h * 0.55, ALLEGRO_ALIGN_CENTER, "Tempo de jogo: %02d:%02d", minutes, seconds);

    // Opções
    al_draw_text(text_font, 
                 level->game_over_selection == 0 ? highlight_color : text_color, 
                 display_w / 2, display_h * 0.75, ALLEGRO_ALIGN_CENTER, 
                 option1_text);
                 
    al_draw_text(text_font, 
                 level->game_over_selection == 1 ? highlight_color : text_color, 
                 display_w / 2, display_h * 0.85, ALLEGRO_ALIGN_CENTER, 
                 "Voltar ao Menu");

    al_destroy_font(title_font);
    al_destroy_font(text_font);
}

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
    
    ALLEGRO_FONT *font = al_load_font("assets/fonts/DirtyWar.otf", PAUSE_MENU_TEXT_FONT_SIZE, 0);
    ALLEGRO_FONT *title_font = al_load_font("assets/fonts/DirtyWar.otf", PAUSE_MENU_TITLE_FONT_SIZE, 0);
    
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

void draw_hud(struct Player *player, struct GameLevel *level) {
    if (!level->hud_font) return;

    int total_seconds = (int)level->game_time;
    int minutes = total_seconds / 60;
    int seconds = total_seconds % 60;

    ALLEGRO_COLOR text_color = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR ammo_color = al_map_rgb(255, 255, 0);
    ALLEGRO_COLOR stamina_color = al_map_rgb(0, 255, 100);

    int screen_w = al_get_display_width(al_get_current_display());

    // Score
    al_draw_textf(level->hud_font, text_color, 20, 10, ALLEGRO_ALIGN_LEFT, "SCORE: %06d", player->score);
                  
    // Tempo
    al_draw_textf(level->hud_font, text_color, screen_w - 20, 10, ALLEGRO_ALIGN_RIGHT, "TIME: %02d:%02d", minutes, seconds);

    // Munição
    if(player->is_reloading) {
         al_draw_text(level->hud_font, ammo_color, 20, 50, ALLEGRO_ALIGN_LEFT, "RECARREGANDO...");
    } else {
        al_draw_textf(level->hud_font, ammo_color, 20, 50, ALLEGRO_ALIGN_LEFT, "MUNICAO: %02d / %d", player->current_ammo, player->max_ammo);
    }
    
    // Barra de Estamina
    float stamina_bar_w = 200;
    float stamina_bar_h = 20;
    float stamina_bar_x = 20;
    float stamina_bar_y = 90; // abaixo da munição
    
    float stamina_percent = player->stamina / player->max_stamina;
    float current_stamina_w = stamina_bar_w * stamina_percent;

    // Fundo da barra
    al_draw_filled_rectangle(stamina_bar_x, stamina_bar_y, stamina_bar_x + stamina_bar_w, stamina_bar_y + stamina_bar_h, al_map_rgba(50, 50, 50, 150));
    // Preenchimento da barra
    al_draw_filled_rectangle(stamina_bar_x, stamina_bar_y, stamina_bar_x + current_stamina_w, stamina_bar_y + stamina_bar_h, stamina_color);
    // Borda da barra
    al_draw_rectangle(stamina_bar_x, stamina_bar_y, stamina_bar_x + stamina_bar_w, stamina_bar_y + stamina_bar_h, al_map_rgb(200, 200, 200), 2);
}
