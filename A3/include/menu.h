#ifndef MENU_H
#define MENU_H

#include <allegro5/allegro5.h>
#include "types.h"

#define MENU_FONT_SIZE 24

void init_menu(struct Menu *menu);
void load_menu_resources(struct Menu *menu);
void unload_menu_resources(struct Menu *menu);
void handle_menu_input(struct Menu *menu, GameState *game_state, ALLEGRO_EVENT *event);
void draw_main_menu(struct Menu *menu);
void draw_options_menu(struct Menu *menu);

#endif