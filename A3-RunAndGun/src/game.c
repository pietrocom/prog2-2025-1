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

// Configura o estado inicial do jogador e o posiciona no cenário
void start_player(struct Player *player, struct GameLevel *level) {
    init_player(player);
    load_player_sprites(player);
    
    player->entity.x = 100.0f;
    player->entity.y = level->ground_level;
    
    update_hitbox_position(&player->entity, player->facing_right);
}

// Carrega os recursos do nível e define suas propriedades iniciais
void start_level(struct GameLevel *level) {
    level->background = al_load_bitmap("assets/background/Street_Background/PNG/City2/Bright/City2.png");
    if (!level->background) {
        fprintf(stderr, "Falha ao carregar o background!\n");
        exit(1);
    }
    
    int screen_h = al_get_display_height(al_get_current_display());
    int bg_height = al_get_bitmap_height(level->background);
    
    level->ground_level = screen_h - GROUND_LEVEL; 
    // Calcula a escala do background para que ele preencha a altura da tela
    level->background_scale = (float)screen_h / (float)bg_height;
    level->scroll_x = 0.0f;
    level->enemy_count = 0;
    level->boss_active = false;
    level->game_time = 0.0; 

    level->hud_font = al_load_font("assets/fonts/DirtyWar.otf", 36, 0);
    if (!level->hud_font) {
        fprintf(stderr, "Falha ao carregar a fonte do HUD!\n");
    }
}


// ---- Funções de Limpeza ----

// Libera os recursos do jogador, como suas sprites
void destroy_player(struct Player *player) {
    unload_player_sprites(player);
}

// Libera os recursos do nível, como o bitmap de fundo e a fonte
void destroy_level(struct GameLevel *level) {
    if (level->background) {
        al_destroy_bitmap(level->background);
    }
    if (level->hud_font) {
        al_destroy_font(level->hud_font);
    }
}


// ---- Funções de Controle do Jogo ----

// Processa eventos de teclado globais durante o jogo, como pausar ou sair
void handle_game_events(ALLEGRO_EVENT *event, GameState *state) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_ESCAPE:
            case ALLEGRO_KEY_P:
                toggle_pause(state);
                break;
                
            // A captura do evento de fechar a janela é feita no loop principal
        }
    }
}


// ---- Funções de Controle de Pausa ----

// Alterna o estado do jogo entre 'PLAYING' e 'PAUSED'
void toggle_pause(GameState *current_state) {
    if (*current_state == PLAYING) {
        *current_state = PAUSED;
        // Futuramente, pode-se adicionar lógica para pausar a música aqui
    } 
    else if (*current_state == PAUSED) {
        *current_state = PLAYING;
    }
}

// Processa os comandos do jogador enquanto o jogo está pausado
void handle_pause_input(ALLEGRO_EVENT *event, GameState *state, 
    struct Player *player, struct GameLevel *level, struct EnemySystem *enemy_system, struct ProjectileSystem *projectile_system) {

    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_ESCAPE:
            case ALLEGRO_KEY_P:
                toggle_pause(state);
                break;
            
            // Tecla 'Q' para sair da partida atual e voltar ao menu principal
            case ALLEGRO_KEY_Q:
                reset_game(player, level, enemy_system, projectile_system);
                *state = MENU;
                break;
        }
    }
}


// ---- Funções de Controle de Game Over ----

// Processa os comandos do jogador na tela de Fim de Jogo
void handle_game_over_input(ALLEGRO_EVENT *event, GameState *state, 
    struct Player *player, struct GameLevel *level, struct EnemySystem *enemy_system, struct ProjectileSystem *projectile_system) {
    
    if (event->type != ALLEGRO_EVENT_KEY_DOWN) return;

    switch (event->keyboard.keycode) {
        case ALLEGRO_KEY_UP:
        case ALLEGRO_KEY_DOWN:
            // Alterna a seleção entre as duas opções (0 e 1)
            level->game_over_selection = (level->game_over_selection + 1) % 2;
            break;

        case ALLEGRO_KEY_ENTER:
            // Ação da primeira opção
            if (level->game_over_selection == 0) {
                // Se o jogador venceu, a opção é "Freeplay" e o jogo continua
                if (level->player_won) {
                    *state = PLAYING;
                } else { // Se perdeu, a opção é "Reiniciar"
                    reset_game(player, level, enemy_system, projectile_system);
                    *state = PLAYING;
                }
            } else { // Ação da segunda opção: "Voltar ao Menu"
                reset_game(player, level, enemy_system, projectile_system);
                *state = MENU;
            }
            break;
    }
}


// ---- Funções de Atualização e Renderização ----

// Reseta completamente o estado do jogo para seus valores iniciais
void reset_game(struct Player *player, struct GameLevel *level, struct EnemySystem *enemy_system, struct ProjectileSystem *projectile_system) {
    printf("Reiniciando o jogo por completo...\n");

    // Reinicializa o jogador
    init_player(player);
    player->entity.x = 100.0f;
    
    // Reinicializa o nível
    level->ground_level = al_get_display_height(al_get_current_display()) - GROUND_LEVEL; 
    player->entity.y = level->ground_level;
    level->scroll_x = 0.0f;
    level->boss_active = false;
    level->game_time = 0.0;

    // Reinicializa os sistemas de inimigos e projéteis
    destroy_enemy_system(enemy_system);
    init_enemy_system(enemy_system);
    clear_projectiles(projectile_system);
}

// Atualiza a lógica de câmera (scrolling) com base na posição do jogador
void update_game(struct Player *player, struct GameLevel *level, float delta_time) {
    level->game_time += delta_time;

    float screen_width = al_get_display_width(al_get_current_display());
    
    // Define as "bordas invisíveis" que ativam o scroll da câmera
    float left_threshold = screen_width * (1 - BACKGROUND_THRESHOLD); 
    float right_threshold = screen_width * BACKGROUND_THRESHOLD;
    
    float move_delta = player->entity.vel_x * delta_time;

    // Movimento para a direita
    if (player->entity.vel_x > 0) {
        // Se o jogador ultrapassa a borda direita, a câmera se move em vez do jogador
        if (player->entity.x > right_threshold) {
            level->scroll_x += move_delta;
        } else { // Caso contrário, o jogador se move na tela
            player->entity.x += move_delta;
        }
    }
    // Movimento para a esquerda
    else if (player->entity.vel_x < 0) {
        // Se o jogador ultrapassa a borda esquerda e a câmera não está no início, a câmera se move
        if (player->entity.x < left_threshold && level->scroll_x > 0) {
            level->scroll_x += move_delta;
            // Garante que o scroll não fique negativo
            if (level->scroll_x < 0) {
                level->scroll_x = 0;
            }
        } else { // Caso contrário, o jogador se move na tela
            player->entity.x += move_delta;
        }
    }

    // Impede que o jogador saia dos limites da tela
    if (player->entity.x < 0) player->entity.x = 0;
    if (player->entity.x + player->entity.width > screen_width) player->entity.x = screen_width - player->entity.width;
}


// Desenha o cenário de fundo com efeito de rolagem e o jogador
void draw_game(struct Player *player, struct GameLevel *level) {
    int screen_w = al_get_display_width(al_get_current_display());
    int screen_h = al_get_display_height(al_get_current_display());
    int bg_width = al_get_bitmap_width(level->background);
    float scaled_bg_width = bg_width * level->background_scale;
    
    // Usa o operador de módulo (fmod) para criar um efeito de loop infinito no background
    float bg_offset = fmod(level->scroll_x, scaled_bg_width);
    
    // Desenha cópias do background lado a lado para preencher a tela e dar a impressão de continuidade
    for (int i = -1; i <= (screen_w / scaled_bg_width) + 1; i++) {
        al_draw_scaled_bitmap(level->background,
            0, 0, bg_width, al_get_bitmap_height(level->background),
            (i * scaled_bg_width) - bg_offset, 0, scaled_bg_width, screen_h,
            0);
    }
    
    // Como a função update_game já lida com o movimento do jogador na tela,
    // basta desenhá-lo em sua posição atual.
    draw_player(player);
    
    if (level->draw_ground_line) {
        draw_ground_line(level);
    }
}

// Desenha a tela de Fim de Jogo, mostrando estatísticas e opções
void draw_game_over_screen(struct Player *player, struct GameLevel *level) {
    ALLEGRO_COLOR bg_color = al_map_rgba(0, 0, 0, 210);
    ALLEGRO_COLOR title_color, text_color, highlight_color;
    const char *title_text;
    const char *option1_text;
    
    ALLEGRO_FONT *title_font = al_load_font("assets/fonts/DirtyWar.otf", GAMEOVER_TITLE_FONT_SIZE, 0);
    ALLEGRO_FONT *text_font = al_load_font("assets/fonts/DirtyWar.otf", GAMEOVER_TEXT_FONT_SIZE, 0);
    
    int display_w = al_get_display_width(al_get_current_display());
    int display_h = al_get_display_height(al_get_current_display());

    // Configura textos e cores com base no resultado do jogo (vitória ou derrota)
    if (level->player_won) {
        title_text = "YOU WON!";
        option1_text = "Freeplay";
        title_color = al_map_rgb(255, 215, 0);
        highlight_color = al_map_rgb(173, 255, 47);
    } else {
        title_text = "GAME OVER";
        option1_text = "Reiniciar";
        title_color = al_map_rgb(200, 0, 0);
        highlight_color = al_map_rgb(255, 100, 100);
    }
    text_color = al_map_rgb(220, 220, 220);

    // Desenha um fundo escuro semi-transparente para focar nos textos
    al_draw_filled_rectangle(0, 0, display_w, display_h, bg_color);
    
    al_draw_text(title_font, title_color, display_w / 2, display_h * 0.2, ALLEGRO_ALIGN_CENTER, title_text);

    // Apresenta as estatísticas finais da partida
    int minutes = (int)level->game_time / 60;
    int seconds = (int)level->game_time % 60;
    al_draw_textf(text_font, text_color, display_w / 2, display_h * 0.45, ALLEGRO_ALIGN_CENTER, "Pontuacao final: %06d", player->score);
    al_draw_textf(text_font, text_color, display_w / 2, display_h * 0.55, ALLEGRO_ALIGN_CENTER, "Tempo de jogo: %02d:%02d", minutes, seconds);

    // Desenha as opções, destacando a que está selecionada
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

// Desenha a interface do menu de pausa sobre o jogo
void draw_pause_menu() {
    // Configurações de cores e fontes para o menu de pausa
    ALLEGRO_COLOR bg_color = al_map_rgba(0, 0, 0, 200);
    ALLEGRO_COLOR box_color = al_map_rgba(70, 70, 90, 220);
    ALLEGRO_COLOR text_color = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR highlight_color = al_map_rgb(180, 180, 255);
    
    ALLEGRO_FONT *font = al_load_font("assets/fonts/DirtyWar.otf", PAUSE_MENU_TEXT_FONT_SIZE, 0);
    ALLEGRO_FONT *title_font = al_load_font("assets/fonts/DirtyWar.otf", PAUSE_MENU_TITLE_FONT_SIZE, 0);
    
    int display_w = al_get_display_width(al_get_current_display());
    int display_h = al_get_display_height(al_get_current_display());
    int box_width = 600;
    int box_height = 400;

    // 1. Desenha um overlay escuro para diminuir o brilho do fundo
    al_draw_filled_rectangle(0, 0, display_w, display_h, bg_color);

    // 2. Desenha a caixa principal do menu no centro da tela
    al_draw_filled_rounded_rectangle(
        display_w/2 - box_width/2, display_h/2 - box_height/2,
        display_w/2 + box_width/2, display_h/2 + box_height/2,
        10, 10, box_color);
    
    al_draw_rounded_rectangle(
        display_w/2 - box_width/2, display_h/2 - box_height/2,
        display_w/2 + box_width/2, display_h/2 + box_height/2,
        10, 10, highlight_color, 3);

    // 3. Título e opções do menu
    al_draw_text(title_font, highlight_color, display_w/2, display_h/2 - 160, ALLEGRO_ALIGN_CENTRE, "JOGO PAUSADO");
    al_draw_text(font, text_color, display_w/2, display_h/2 + 35, ALLEGRO_ALIGN_CENTRE, "''P'' Continuar");
    al_draw_text(font, text_color, display_w/2, display_h/2, ALLEGRO_ALIGN_CENTRE, "''Q'' Voltar ao menu");

    // Libera as fontes carregadas apenas para esta função
    al_destroy_font(font);
    al_destroy_font(title_font);
}

// Desenha a interface de informações (HUD) na tela
void draw_hud(struct Player *player, struct GameLevel *level) {
    if (!level->hud_font) return;

    // Converte o tempo de jogo para minutos e segundos
    int total_seconds = (int)level->game_time;
    int minutes = total_seconds / 60;
    int seconds = total_seconds % 60;

    ALLEGRO_COLOR text_color = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR ammo_color = al_map_rgb(255, 255, 0);
    ALLEGRO_COLOR stamina_color = al_map_rgb(0, 255, 100);

    int screen_w = al_get_display_width(al_get_current_display());

    // Elementos do HUD: Pontuação e Tempo
    al_draw_textf(level->hud_font, text_color, 20, 10, ALLEGRO_ALIGN_LEFT, "SCORE: %06d", player->score);
    al_draw_textf(level->hud_font, text_color, screen_w - 20, 10, ALLEGRO_ALIGN_RIGHT, "TIME: %02d:%02d", minutes, seconds);

    // Elemento do HUD: Munição
    if(player->is_reloading) {
         al_draw_text(level->hud_font, ammo_color, 20, 50, ALLEGRO_ALIGN_LEFT, "RECARREGANDO...");
    } else {
        al_draw_textf(level->hud_font, ammo_color, 20, 50, ALLEGRO_ALIGN_LEFT, "MUNICAO: %02d / %d", player->current_ammo, player->max_ammo);
    }
    
    // Elemento do HUD: Barra de Estamina
    float stamina_bar_w = 200;
    float stamina_bar_h = 20;
    float stamina_bar_x = 20;
    float stamina_bar_y = 90;
    float stamina_percent = player->stamina / player->max_stamina;
    al_draw_filled_rectangle(stamina_bar_x, stamina_bar_y, stamina_bar_x + stamina_bar_w, stamina_bar_y + stamina_bar_h, al_map_rgba(50, 50, 50, 150));
    al_draw_filled_rectangle(stamina_bar_x, stamina_bar_y, stamina_bar_x + (stamina_bar_w * stamina_percent), stamina_bar_y + stamina_bar_h, stamina_color);
    al_draw_rectangle(stamina_bar_x, stamina_bar_y, stamina_bar_x + stamina_bar_w, stamina_bar_y + stamina_bar_h, al_map_rgb(200, 200, 200), 2);

    // Elemento do HUD: Barra de Vida
    float health_bar_w = 200;
    float health_bar_h = 20;
    float health_bar_x = 20;
    float health_bar_y = stamina_bar_y + stamina_bar_h + 10;
    float health_percent = (float)player->health / player->max_health;
    if (health_percent < 0) health_percent = 0;
    
    // A cor da barra de vida muda de acordo com a vida restante do jogador
    ALLEGRO_COLOR health_color;
    if (health_percent > 0.6f) {
        health_color = al_map_rgb(46, 204, 113); // Verde
    } else if (health_percent > 0.3f) {
        health_color = al_map_rgb(241, 196, 15); // Amarelo
    } else {
        health_color = al_map_rgb(231, 76, 60);  // Vermelho
    }

    al_draw_filled_rectangle(health_bar_x, health_bar_y, health_bar_x + health_bar_w, health_bar_y + health_bar_h, al_map_rgba(50, 50, 50, 150));
    al_draw_filled_rectangle(health_bar_x, health_bar_y, health_bar_x + (health_bar_w * health_percent), health_bar_y + health_bar_h, health_color);
    al_draw_rectangle(health_bar_x, health_bar_y, health_bar_x + health_bar_w, health_bar_y + health_bar_h, al_map_rgb(200, 200, 200), 2);
}