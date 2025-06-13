#include <stdio.h>
#include "menu.h"

void init_menu (struct Menu *menu) {
    menu->background = NULL;
    menu->font = NULL;
    menu->selected_option = 0;
    menu->current_state = MENU_MAIN; 
}

void load_menu_resources (struct Menu *menu) {
    // Carrega imagem de fundo
    menu->background = al_load_bitmap("assets/background/menu_background.JPEG");
    if (!menu->background) {
        fprintf(stderr, "Falha ao carregar o background do menu!\n");
    }

    // Carrega fonte (tamanho 36)
    menu->font = al_load_font("assets/fonts/DirtyWar.otf", MENU_FONT_SIZE, 0);
    if (!menu->font) {
        fprintf(stderr, "Falha ao carregar a fonte do menu!\n");
        // Usa fonte built-in se a customizada falhar
        menu->font = al_create_builtin_font();
    }
}


void unload_menu_resources (struct Menu *menu) {
    if (menu->background) {
        al_destroy_bitmap(menu->background);
    }
    if (menu->font) {
        al_destroy_font(menu->font);
    }
}

void handle_menu_input (struct Menu *menu, GameState *game_state, ALLEGRO_EVENT *event) {
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
                *game_state = PLAYING;
                break;
                
            case ALLEGRO_KEY_ESCAPE:
                *game_state = GAME_OVER;
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
    int screen_w = al_get_display_width(al_get_current_display());
    int screen_h = al_get_display_height(al_get_current_display());

    // Título do jogo
    al_draw_text(menu->font, white, screen_w/2, screen_h/4, 
                ALLEGRO_ALIGN_CENTER, "LISBOA DE ALMEIDA - REBORN");

    // Opções do menu
    const char *options[] = {"Iniciar Jogo", "Opções", "Sair"};
    
    for (int i = 0; i < 3; i++) {
        ALLEGRO_COLOR color = (i == menu->selected_option) ? yellow : white;
        al_draw_text(menu->font, color, screen_w/2, 
                    screen_h/2 + i * 50, 
                    ALLEGRO_ALIGN_CENTER, options[i]);
    }
}

void draw_options_menu (struct Menu *menu) {
    al_clear_to_color(al_map_rgb(0, 0, 30));
    
    int screen_w = al_get_display_width(al_get_current_display());
    int screen_h = al_get_display_height(al_get_current_display());

    al_draw_text(menu->font, al_map_rgb(255, 255, 255), 
                screen_w/2, screen_h/4, 
                ALLEGRO_ALIGN_CENTER, "OPÇÕES");

    // Exemplo de opções ajustáveis
    al_draw_text(menu->font, al_map_rgb(255, 255, 255),
                screen_w/2, screen_h/2,
                ALLEGRO_ALIGN_CENTER, "Volume: [implementar]");

    al_draw_text(menu->font, al_map_rgb(255, 255, 0),
                screen_w/2, screen_h - 100,
                ALLEGRO_ALIGN_CENTER, "Voltar (ESC)");
}