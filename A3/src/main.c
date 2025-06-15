//Compilação: gcc src/main.c -o AS $(pkg-config allegro_ttf-5 allegro_image-5 allegro-5 allegro_main-5 allegro_font-5 --libs --cflags)

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

    start_level(&level);
	start_player(&player, &level);

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

		// Verifica se quer sair do jogo
		if (game_state == GAME_OVER) {
			break; 
		}

		// Trata eventos gerais (fechar janela)
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			break;
		}

		// Chama o handler de eventos específico para cada estado
		switch (game_state) {
			case MENU:
				handle_menu_input(&game_menu, &game_state, &event);
				break;
				
			case PLAYING:
			case PAUSED:
				handle_game_events(&event, &game_state); 
				break;
		}

		// Atualizações de estado
		switch (game_state) {
			case MENU:
				if (redraw) {
					if (game_menu.current_state == MENU_MAIN) {
						draw_main_menu(&game_menu);
					} 
					else if (game_menu.current_state == MENU_OPTIONS) {
						draw_options_menu(&game_menu);
					}
					al_flip_display();
					redraw = false;
				}
				break;
				
			case PLAYING:
				handle_player_input(&player, &event);
				update_player(&player, 1.0/FPS, &level);
				update_game(&player, &level);
				
				if (redraw) {
					draw_game(&player, &level);
					al_flip_display();
					redraw = false;
				}
				break;

			case PAUSED:
				if (redraw) {
					draw_game(&player, &level);
					draw_pause_menu(&level);
					al_flip_display();
					redraw = false;
				}
				break;
		}

		if (event.type == ALLEGRO_EVENT_TIMER) {
			redraw = true;
		}
	}

    // Limpeza
    destroy_player(&player);
    destroy_level(&level);
    unload_menu_resources(&game_menu);
    al_destroy_timer(timer);
    al_destroy_event_queue(event_queue);
    al_destroy_display(display);

    return 0;
}