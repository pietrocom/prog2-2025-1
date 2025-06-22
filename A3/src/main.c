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

int main() {
    
    init_allegro();

    ALLEGRO_DISPLAY *display = create_display(SCREEN_W, SCREEN_H);
    ALLEGRO_TIMER *timer = al_create_timer(1.0 / FPS);
    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

    // Inicializações do jogo
    GameState game_state = MENU;
    struct Player player;
    struct GameLevel level;
    struct EnemySystem enemy_system;
    struct ProjectileSystem projectile_system;

    // Inicializa sistemas
    start_level(&level);
    start_player(&player, &level);
    init_enemy_system(&enemy_system);
    init_projectile_system(&projectile_system);

    // Inicializações do menu
    struct Menu game_menu;
    init_menu(&game_menu);
    load_menu_resources(&game_menu);

    // Controla refresh da tela
    bool redraw = true;
    al_start_timer(timer);

    // Loop principal do jogo
    while (true) {
        ALLEGRO_EVENT event;
        al_wait_for_event(event_queue, &event);

        if (game_state == QUIT || event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break; 
        }

        // --- MANIPULAÇÃO DE INPUTS POR ESTADO ---
        // Este switch cuida apenas da entrada do usuário (teclado, etc.)
        switch (game_state) {
            case MENU:
                handle_menu_input(&game_menu, &game_state, &event);
                break;
            case PLAYING:
                handle_game_events(&event, &game_state);
                // O input do jogador é tratado dentro da lógica de atualização do PLAYING
                handle_player_input(&player, &event, &level);
                break;
            case PAUSED:
                handle_pause_input(&event, &game_state, &player, &level, &enemy_system, &projectile_system);
                break;
            case GAME_OVER:
                // Chama a nova função que lida com os inputs da tela de Game Over
                handle_game_over_input(&event, &game_state, &player, &level, &enemy_system, &projectile_system);
                break;
            case QUIT:
                break; // Já foi lidado
        }

        // --- ATUALIZAÇÃO DE ESTADOS E LÓGICA DO JOGO ---
        // Este switch cuida da lógica de atualização (movimento, IA) e do desenho.
        // Ele só executa quando o timer dispara um "redraw".
        if (event.type == ALLEGRO_EVENT_TIMER) {
            redraw = true;

            if (game_state == PLAYING) {
                // Atualiza todas as entidades do jogo
                update_player(&player, 1.0/FPS, &level, &projectile_system);
                update_enemy_system(&enemy_system, &player, &level, &projectile_system, 1.0/FPS);
                update_projectile_system(&projectile_system, 1.0/FPS, &player, &enemy_system, &level);
                update_game(&player, &level, 1.0/FPS);
                
                // NOVO: Verifica as condições de fim de jogo APÓS as atualizações
                // GATILHO DE DERROTA
                if (is_player_dead(&player)) {
                    game_state = GAME_OVER;
                    level.player_won = false;
                    level.game_over_selection = 0;
                }
                
                // GATILHO DE VITÓRIA: Ocorre se o chefe está morto e sua animação de morte terminou
                // A flag level.player_won impede que a tela de vitória seja re-acionada no modo freeplay
                if (enemy_system.boss.is_dead && !enemy_system.boss.is_active && !level.player_won) {
                    game_state = GAME_OVER;
                    level.player_won = true;
                    level.game_over_selection = 0;
                }
            }
        }
        
        // --- DESENHO NA TELA ---
        if (redraw) {
            redraw = false;
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
                    draw_game(&player, &level);
                    draw_enemies(&enemy_system, &level, &player);
                    draw_projectiles(&projectile_system, &level, &player);
                    draw_hud(&player, &level);
                    draw_pause_menu(&level);
                    break;
                    
                case GAME_OVER:
                    // Mostra as estatísticas e opções
                    draw_game_over_screen(&player, &level);
                    break;

                case QUIT:
                    break;
            }
            al_flip_display();
        }
    }

    // Limpeza
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
