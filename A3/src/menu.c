#include <stdio.h>
#include "menu.h"

// Inicializa a estrutura do menu com valores padrão
void init_menu(struct Menu *menu) {
    menu->background = NULL;
    menu->title_font = NULL;
    menu->text_font = NULL;
    menu->main_menu_selection = 0;
    menu->options_menu_selection = 0;
    menu->current_state = MENU_MAIN; 
}

// Carrega os recursos gráficos e fontes usados nos menus
void load_menu_resources (struct Menu *menu) {
    menu->background = al_load_bitmap("assets/background/menu_background.JPEG");
    if (!menu->background) {
        fprintf(stderr, "Falha ao carregar o background do menu!\n");
    }

    menu->title_font = al_load_font("assets/fonts/DirtyWar.otf", MAIN_MENU_TITLE_FONT_SIZE, 0);
    menu->text_font = al_load_font("assets/fonts/DirtyWar.otf", MAIN_MENU_TEXT_FONT_SIZE, 0);
    
    // Caso as fontes customizadas não sejam encontradas, carrega uma fonte padrão do Allegro
    if (!menu->title_font || !menu->text_font) {
        fprintf(stderr, "Falha ao carregar a fonte do menu! Usando fonte built-in\n");
        menu->text_font = al_create_builtin_font();
        menu->title_font = al_create_builtin_font();
    }
}

// Libera da memória os recursos do menu
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

// Direciona os eventos de input para o handler do submenu correspondente
void handle_menu_input (struct Menu *menu, GameState *game_state, ALLEGRO_EVENT *event) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        switch (menu->current_state) {
            case MENU_MAIN:
                handle_main_menu_input(menu, game_state, event);
                break;
            case MENU_OPTIONS:
                handle_options_menu_input(menu, event);
                break;
            case MENU_PAUSE:
                // O menu de pausa é gerenciado em game.c
                break;
        }
    }
}

// Processa os comandos do jogador no menu principal
void handle_main_menu_input (struct Menu *menu, GameState *game_state, ALLEGRO_EVENT *event) {
    switch (event->keyboard.keycode) {
        case ALLEGRO_KEY_UP:
            menu->main_menu_selection--;
            // Garante que a seleção "dê a volta" ao chegar no topo
            if (menu->main_menu_selection < 0) menu->main_menu_selection = 2;
            break;
            
        case ALLEGRO_KEY_DOWN:
            // O operador de módulo (%) faz a seleção "dar a volta" ao chegar no final
            menu->main_menu_selection = (menu->main_menu_selection + 1) % 3;
            break;
            
        case ALLEGRO_KEY_ENTER:
            // Executa uma ação com base na opção selecionada
            switch(menu->main_menu_selection) {
                case 0: *game_state = PLAYING; break;      // Iniciar Jogo
                case 1: menu->current_state = MENU_OPTIONS; break; // Opções
                case 2: *game_state = QUIT; break;         // Sair
            }
            break;
            
        case ALLEGRO_KEY_ESCAPE:
            *game_state = QUIT;
            break;
    }
}

// Processa os comandos do jogador no menu de opções
void handle_options_menu_input (struct Menu *menu, ALLEGRO_EVENT *event) {
    switch (event->keyboard.keycode) {
        case ALLEGRO_KEY_UP:
            menu->options_menu_selection--;
            if (menu->options_menu_selection < 0) 
                menu->options_menu_selection = 1; // Ajustar para o número de opções
            break;
            
        case ALLEGRO_KEY_DOWN:
            menu->options_menu_selection = (menu->options_menu_selection + 1) % 2;
            break;
            
        case ALLEGRO_KEY_ENTER:
            if (menu->options_menu_selection == 0) {
                // Futuramente, implementar a lógica da primeira opção aqui
            } else {
                // A segunda opção é "Voltar"
                menu->current_state = MENU_MAIN;
            }
            break;
            
        case ALLEGRO_KEY_ESCAPE:
            // A tecla ESC sempre volta para o menu principal
            menu->current_state = MENU_MAIN;
            break;
    }
}

// Desenha a tela do menu principal
void draw_main_menu(struct Menu *menu) {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    
    if (menu->background) {
        al_draw_bitmap(menu->background, 0, 0, 0);
    }

    // Configurações de cores e dimensões
    ALLEGRO_COLOR white = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR yellow = al_map_rgb(255, 255, 0);
    ALLEGRO_COLOR shadow = al_map_rgb(50, 50, 50);
    int screen_w = al_get_display_width(al_get_current_display());
    int screen_h = al_get_display_height(al_get_current_display());
    int title_x = screen_w / 2;
    int title_y = screen_h / 5;
    int shadow_offset = 5;
    const char *title = "SHOOTER BORN IN HEAVEN";

    // Desenha o título do jogo com um efeito de sombra para destaque
    al_draw_text(menu->title_font, shadow, title_x + shadow_offset, title_y + shadow_offset, ALLEGRO_ALIGN_CENTER, title);
    al_draw_text(menu->title_font, white, title_x, title_y, ALLEGRO_ALIGN_CENTER, title);

    // Lista de opções do menu
    const char *options[] = {"Iniciar Jogo", "Opções", "Sair"};
    
    // Desenha cada opção na tela
    for (int i = 0; i < 3; i++) {
        // Usa uma cor diferente (amarelo) para destacar a opção selecionada
        ALLEGRO_COLOR color = (i == menu->main_menu_selection) ? yellow : white;
        al_draw_text(menu->text_font, color, screen_w/2, 
                    4 * screen_h / 7 + i * 50, 
                    ALLEGRO_ALIGN_CENTER, options[i]);
    }
}

// Desenha a tela do menu de opções
void draw_options_menu (struct Menu *menu) {
    al_clear_to_color(al_map_rgb(0, 0, 30));
    
    int screen_w = al_get_display_width(al_get_current_display());
    int screen_h = al_get_display_height(al_get_current_display());

    // Título da tela
    al_draw_text(menu->title_font, al_map_rgb(255, 255, 255), 
                screen_w/2, screen_h/4, 
                ALLEGRO_ALIGN_CENTER, "OPÇÕES");

    // Configuração de cores para as opções
    ALLEGRO_COLOR white = al_map_rgb(255, 255, 255);
    ALLEGRO_COLOR yellow = al_map_rgb(255, 255, 0);
    
    // Opção 1 
    al_draw_text(menu->text_font, 
                (menu->options_menu_selection == 0) ? yellow : white,
                screen_w/2, screen_h/2,
                ALLEGRO_ALIGN_CENTER, "Volume: [implementar]");

    // Opção 2
    al_draw_text(menu->text_font, 
                (menu->options_menu_selection == 1) ? yellow : white,
                screen_w/2, screen_h/2 + 50,
                ALLEGRO_ALIGN_CENTER, "Voltar (ESC)");
}