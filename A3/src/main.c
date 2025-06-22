#include <stdio.h>
#include <allegro5/allegro5.h>                                          
#include <allegro5/allegro_font.h>  
#include <allegro5/allegro_image.h>     
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#include "types.h"
#include "game.h"
#include "menu.h"
#include "player.h"
#include "projectiles.h"
#include "enemy.h"
#include "utils.h"

#define SCREEN_W 1600
#define SCREEN_H 900
#define FPS 60

// Função principal que inicializa o jogo, executa o loop principal e limpa os recursos ao final
int main() {
    
    // --- INICIALIZAÇÃO DO ALLEGRO E DA JANELA ---
    init_allegro();
    ALLEGRO_DISPLAY *display = create_display(SCREEN_W, SCREEN_H);
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / FPS);
    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    
    // Registra as fontes de eventos que o jogo irá escutar
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

    // --- INICIALIZAÇÃO DOS SISTEMAS DO JOGO ---
    GameState game_state = MENU;
    struct Player player;
    struct GameLevel level;
    struct EnemySystem enemy_system;
    struct ProjectileSystem projectile_system;

    // Chama as funções que preparam cada sistema para o início do jogo
    start_level(&level);
    start_player(&player, &level);
    init_enemy_system(&enemy_system);
    init_projectile_system(&projectile_system);

    // Inicializa a estrutura e os recursos do menu
    struct Menu game_menu;
    init_menu(&game_menu);
    load_menu_resources(&game_menu);

    // --- CONTROLE DO LOOP PRINCIPAL ---
    bool redraw = true; // Flag para controlar quando a tela deve ser redesenhada
    al_start_timer(timer);

    // O Game Loop executa continuamente até que o estado seja 'QUIT'
    while (true) {
        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event);

        // Condição de saída do loop (fechar janela ou estado QUIT)
        if (game_state == QUIT || event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break; 
        }

        // --- SEÇÃO DE PROCESSAMENTO DE INPUTS ---
        // Este switch direciona os eventos de input para a função de tratamento correta,
        // com base no estado atual do jogo (MENU, PLAYING, etc)
        switch (game_state) {
            case MENU:
                handle_menu_input(&game_menu, &game_state, &event);
                break;
            case PLAYING:
                // Inputs globais do jogo (como pausar) são tratados aqui
                handle_game_events(&event, &game_state);
                // Inputs específicos do jogador são tratados aqui
                handle_player_input(&player, &event, &level);
                break;
            case PAUSED:
                handle_pause_input(&event, &game_state, &player, &level, &enemy_system, &projectile_system);
                break;
            case GAME_OVER:
                handle_game_over_input(&event, &game_state, &player, &level, &enemy_system, &projectile_system);
                break;
            case QUIT:
                // A verificação no início do loop cuidará disso
                break;
        }

        // --- SEÇÃO DE ATUALIZAÇÃO DA LÓGICA DO JOGO ---
        // Este bloco só executa em intervalos fixos definidos pelo FPS do timer
        if (event.type == ALLEGRO_EVENT_TIMER) {
            redraw = true; // Sinaliza que a tela precisa ser redesenhada

            if (game_state == PLAYING) {
                // Atualiza a lógica de todos os sistemas do jogo
                update_player(&player, 1.0/FPS, &level, &projectile_system);
                update_enemy_system(&enemy_system, &player, &level, &projectile_system, 1.0/FPS);
                update_projectile_system(&projectile_system, 1.0/FPS, &player, &enemy_system, &level);
                update_game(&player, &level, 1.0/FPS);
                
                // Verifica as condições de fim de jogo após todas as atualizações
                if (is_player_dead(&player)) {
                    game_state = GAME_OVER;
                    level.player_won = false;
                    level.game_over_selection = 0;
                }
                
                // Condição de vitória: chefe está morto e sua animação de morte terminou
                if (enemy_system.boss.is_dead && !enemy_system.boss.is_active && !level.player_won) {
                    game_state = GAME_OVER;
                    level.player_won = true;
                    level.game_over_selection = 0;
                }
            }
        }
        
        // --- SEÇÃO DE RENDERIZAÇÃO ---
        // Este bloco só executa se a lógica do jogo foi atualizada, evitando redesenhos desnecessários
        if (redraw) {
            redraw = false;
            // O switch decide o que desenhar com base no estado do jogo
            switch (game_state) {
                case MENU:
                    if (game_menu.current_state == MENU_MAIN) {
                        draw_main_menu(&game_menu);
                    } else if (game_menu.current_state == MENU_OPTIONS) {
                        draw_options_menu(&game_menu);
                    }
                    break;
                    
                case PLAYING:
                    draw_game(&player, &level);
                    draw_enemies(&enemy_system, &level, &player); 
                    draw_projectiles(&projectile_system, &level, &player);
                    draw_hud(&player, &level);
                    break;

                case PAUSED:
                    // Desenha o jogo congelado no fundo e o menu de pausa por cima
                    draw_game(&player, &level);
                    draw_enemies(&enemy_system, &level, &player);
                    draw_projectiles(&projectile_system, &level, &player);
                    draw_hud(&player, &level);
                    draw_pause_menu(); // A função foi alterada para não precisar de 'level'
                    break;
                    
                case GAME_OVER:
                    draw_game_over_screen(&player, &level);
                    break;

                case QUIT:
                    break;
            }
            // Exibe na tela tudo o que foi desenhado no buffer
            al_flip_display();
        }
    }

    // --- SEÇÃO DE LIMPEZA DE MEMÓRIA ---
    // Libera todos os recursos alocados ao final do programa para evitar vazamentos de memória
    destroy_player(&player);
    destroy_level(&level);
    destroy_enemy_system(&enemy_system);
    destroy_projectile_system(&projectile_system);
    unload_menu_resources(&game_menu);
    al_destroy_timer(timer);
    al_destroy_event_queue(event_queue);
    al_destroy_display(display);

    return 0;
}