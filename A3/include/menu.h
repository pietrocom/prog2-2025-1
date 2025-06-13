#ifndef MENU_H
#define MENU_H

#include <allegro5/allegro5.h>
#include "types.h"

#define MENU_TITLE_FONT_SIZE 54
#define MENU_TEXT_FONT_SIZE 24

// Inicialização
void init_menu(struct Menu *menu);
void load_menu_resources(struct Menu *menu);
void unload_menu_resources(struct Menu *menu);

// Controle de input
void handle_menu_input(struct Menu *menu, GameState *game_state, ALLEGRO_EVENT *event);
void handle_main_menu_input(struct Menu *menu, GameState *game_state, ALLEGRO_EVENT *event);
void handle_options_menu_input(struct Menu *menu, ALLEGRO_EVENT *event);

// Renderização
void draw_main_menu(struct Menu *menu);
void draw_options_menu(struct Menu *menu);

#endif