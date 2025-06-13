#include <stdio.h>
#include "menu.h"

void init_menu (struct Menu *menu) {
    menu->background = NULL;
    menu->title_font = NULL;
    menu->text_font = NULL;
    menu->selected_option = 0;
    menu->current_state = MENU_MAIN; 
}

void load_menu_resources (struct Menu *menu) {
    // Carrega imagem de fundo
    menu->background = al_load_bitmap("assets/background/menu_background.JPEG");
    if (!menu->background) {
        fprintf(stderr, "Falha ao carregar o background do menu!\n");
    }

    menu->title_font = al_load_font("assets/fonts/DirtyWar.otf", MENU_TITLE_FONT_SIZE, 0);
    menu->text_font = al_load_font("assets/fonts/DirtyWar.otf", MENU_TEXT_FONT_SIZE, 0);
    if (!menu->title_font || !menu->text_font) {
        fprintf(stderr, "Falha ao carregar a fonte do menu!\n");
        // Usa fonte built-in se a customizada falhar
        menu->text_font = al_create_builtin_font();
        menu->title_font = al_create_builtin_font();
    }
}


void unload_menu_resources (struct Menu *menu) {
    if (menu->background) {
        al_destroy_bitmap(menu->background);
    }
    if (menu->text_font) {
        al_destroy_font(menu->text_font);
    }
    if (menu->title_font) {
        al_destroy_font(menu->title_font);
    }
}

void handle_menu_input(struct Menu *menu, GameState *game_state, ALLEGRO_EVENT *event) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_UP:
                menu->selected_option--;
                if (menu->selected_option < 0) menu->selected_option = 2;
                break;
                
            case ALLEGRO_KEY_DOWN:
                menu->selected_option = (menu->selected_option + 1) % 3;
                break;
                
            case ALLEGRO_KEY_ENTER:
                switch(menu->selected_option) {
                    case 0: // Iniciar Jogo
                        *game_state = PLAYING;
                        break;
                    case 1: // Opções
                        menu->current_state = MENU_OPTIONS;
                        break;
                    case 2: // Sair
                        *game_state = GAME_OVER; // Isto deve fechar o jogo
                        break;
                }
                break;
                
            case ALLEGRO_KEY_ESCAPE:
                if (menu->current_state == MENU_OPTIONS) {
                    menu->current_state = MENU_MAIN; // Volta ao menu principal
                } else {
                    *game_state = GAME_OVER; // Sai do jogo
                }
                break;
        }
    }
}

void draw_main_menu (struct Menu *menu) {
    // Limpa a tela
    al_clear_to_color(al_map_rgb(0, 0, 0));
    
    // Desenha o background se existir
    if (menu->background) {
        al_draw_bitmap(menu->background, 0, 0, 0);
    }

    // Configurações de cor
    ALLEGRO_COLOR white = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR yellow = al_map_rgb(255, 255, 0);
    ALLEGRO_COLOR shadow = al_map_rgb(50, 50, 50);
    int screen_w = al_get_display_width(al_get_current_display());
    int screen_h = al_get_display_height(al_get_current_display());

    int title_x = screen_w / 2;
    int title_y = screen_h / 5;
    int shadow_offset = 5;
    const char *title = "LISBOA DE ALMEIDA REBORN";

    // Sombra do título do jogo
    al_draw_text(menu->title_font, shadow, 
                title_x + shadow_offset, title_y + shadow_offset, 
                ALLEGRO_ALIGN_CENTER, title);
    
    // Título do jogo
    al_draw_text(menu->title_font, white, 
                title_x, title_y, 
                ALLEGRO_ALIGN_CENTER, title);

    // Opções do menu
    const char *options[] = {"Iniciar Jogo", "Opções", "Sair"};
    
    for (int i = 0; i < 3; i++) {
        ALLEGRO_COLOR color = (i == menu->selected_option) ? yellow : white;
        al_draw_text(menu->text_font, color, screen_w/2, 
                    screen_h/2 + i * 50, 
                    ALLEGRO_ALIGN_CENTER, options[i]);
    }
}

void draw_options_menu (struct Menu *menu) {
    al_clear_to_color(al_map_rgb(0, 0, 30));
    
    int screen_w = al_get_display_width(al_get_current_display());
    int screen_h = al_get_display_height(al_get_current_display());

    al_draw_text(menu->text_font, al_map_rgb(255, 255, 255), 
                screen_w/2, screen_h/4, 
                ALLEGRO_ALIGN_CENTER, "OPÇÕES");

    // Exemplo de opções ajustáveis
    al_draw_text(menu->text_font, al_map_rgb(255, 255, 255),
                screen_w/2, screen_h/2,
                ALLEGRO_ALIGN_CENTER, "Volume: [implementar]");

    al_draw_text(menu->text_font, al_map_rgb(255, 255, 0),
                screen_w/2, screen_h - 100,
                ALLEGRO_ALIGN_CENTER, "Voltar (ESC)");
}