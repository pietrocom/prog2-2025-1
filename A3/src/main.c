//Compilação: gcc src/main.c -o AS $(pkg-config allegro_ttf-5 allegro_image-5 allegro-5 allegro_main-5 allegro_font-5 --libs --cflags)

#include <stdio.h>
#include <allegro5/allegro5.h>											
#include <allegro5/allegro_font.h>	
#include <allegro5/allegro_image.h>		
#include <allegro5/allegro_ttf.h>

#include "types.h"
#include "game.h"
#include "menu.h"

#define SCREEN_W 1600
#define SCREEN_H 900
#define FPS 60

int main() {
    if (!al_init()) {
        fprintf(stderr, "Falha ao inicializar Allegro.\n");
        return -1;
    }

    if (!al_install_keyboard()) {
        fprintf(stderr, "Falha ao inicializar o teclado.\n");
        return -1;
    }

    al_init_image_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    ALLEGRO_DISPLAY *display = al_create_display(SCREEN_W, SCREEN_H);
    if (!display) {
        fprintf(stderr, "Falha ao criar display.\n");
        return -1;
    }

    ALLEGRO_TIMER *timer = al_create_timer(1.0 / FPS);
    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

    // Inicializações do jogo
    GameState game_state = MENU;
    struct Player player;
    struct GameLevel level;

	init_player(&player);
    init_level(&level);

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
			break; // Sai do loop principal
		}

		switch (game_state) {
			case MENU:
				handle_menu_input(&game_menu, &game_state, &event);
				if (redraw) {
					if (game_menu.current_state == MENU_MAIN) {
						draw_main_menu(&game_menu);
					} else if (game_menu.current_state == MENU_OPTIONS) {
						draw_options_menu(&game_menu);
					}
					al_flip_display();
					redraw = false;
				}
				break;
				
			case PLAYING:
				if (redraw) {
					al_clear_to_color(al_map_rgb(0, 0, 50)); // Fundo azul escuro
					al_draw_text(game_menu.text_font, al_map_rgb(255, 255, 255),
								SCREEN_W/2, SCREEN_H/2,
								ALLEGRO_ALIGN_CENTER, "MODO JOGO - PRESSIONE ESC PARA VOLTAR");
					al_flip_display();
					redraw = false;
				}
				
				// Verifica se quer voltar ao menu
				if (event.type == ALLEGRO_EVENT_KEY_DOWN && 
					event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
					game_state = MENU;
				}
				break;
		}

		if (event.type == ALLEGRO_EVENT_TIMER) {
			redraw = true;
		} else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			break;
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