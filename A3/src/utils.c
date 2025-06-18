#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h> 
#include <allegro5/allegro5.h>											
#include <allegro5/allegro_font.h>	
#include <allegro5/allegro_image.h>		
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#include "types.h"
#include "utils.h"
#include "player.h"

void init_allegro () {
    if (!al_init()) {
        fprintf(stderr, "Failed to initialize Allegro.\n");
        exit(EXIT_FAILURE);
    }

    if (!al_init_primitives_addon()) {
        fprintf(stderr, "Falha ao inicializar primitives addon.\n");
        exit(EXIT_FAILURE);
    }

    if (!al_install_keyboard()) {
        fprintf(stderr, "Failed to install keyboard.\n");
        exit(EXIT_FAILURE);
    }

    if (!al_init_image_addon()) {
        fprintf(stderr, "Failed to initialize image addon.\n");
        exit(EXIT_FAILURE);
    }

    if (!al_init_font_addon()) {
        fprintf(stderr, "Failed to initialize font addon.\n");
        exit(EXIT_FAILURE);
    }

    if (!al_init_ttf_addon()) {
        fprintf(stderr, "Failed to initialize TTF addon.\n");
        exit(EXIT_FAILURE);
    }
}

ALLEGRO_DISPLAY * create_display (int width, int height) {
    ALLEGRO_DISPLAY *display = al_create_display(width, height);
    if (!display) {
        fprintf(stderr, "Failed to create display.\n");
        exit(EXIT_FAILURE);
    }

    return display;
}

void split_spritesheet(const char *filename, int frame_width, int frame_height,
                      ALLEGRO_BITMAP **frames, int *frame_count) 
{
    // Carrega a spritesheet completa
    ALLEGRO_BITMAP *sheet = al_load_bitmap(filename);
    if (!sheet) {
        fprintf(stderr, "Failed to load spritesheet: %s\n", filename);
        *frame_count = 0;
        return;
    }

    // Calcula quantos frames existem na imagem
    int sheet_width = al_get_bitmap_width(sheet);
    *frame_count = sheet_width / frame_width;

    // Salva o alvo de desenho atual para restaurá-lo depois
    ALLEGRO_BITMAP *old_target = al_get_target_bitmap();

    for (int i = 0; i < *frame_count; i++) {
        // --> CORREÇÃO 1: Evita o buffer overflow se o spritesheet for muito grande
        if (i >= MAX_FRAMES) {
            fprintf(stderr, "Warning: Spritesheet %s has more frames than MAX_FRAMES (%d). Truncating.\n", filename, MAX_FRAMES);
            *frame_count = MAX_FRAMES; // Corrige a contagem de frames para o valor máximo permitido
            break; // Sai do loop para não escrever fora da memória do array
        }

        // Cria um bitmap novo e independente para cada frame
        frames[i] = al_create_bitmap(frame_width, frame_height);
        if (!frames[i]) {
            fprintf(stderr, "Failed to create frame %d for %s\n", i, filename);
            continue;
        }

        // Define o novo bitmap como alvo do desenho
        al_set_target_bitmap(frames[i]);
        
        // --> CORREÇÃO 2: Limpa o novo bitmap com transparência para evitar "lixo" gráfico (chiado)
        al_clear_to_color(al_map_rgba(0, 0, 0, 0)); 

        // Copia a região correta da spritesheet para o novo frame
        al_draw_bitmap_region(sheet, i * frame_width, 0, 
                             frame_width, frame_height,
                             0, 0, 0);
    }

    // Restaura o alvo de desenho original
    al_set_target_bitmap(old_target);
    // Libera a memória da spritesheet original, que não é mais necessária
    al_destroy_bitmap(sheet);
}

bool soldier_supports_crouch(SoldierType type) {
    return type != SOLDIER_1; // Apenas Soldier 1 não agacha
}

bool file_exists(const char* filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

void draw_ground_line(struct GameLevel *level) {
    if (!level || !level->draw_ground_line) return;

    // Configurações da linha
    ALLEGRO_COLOR line_color = al_map_rgb(255, 0, 0); // Vermelho
    float line_thickness = 2.0f;
    int screen_width = al_get_display_width(al_get_current_display());
    
    // Desenha a linha horizontal no ground level
    al_draw_line(
        0,                          // X inicial (borda esquerda)
        level->ground_level,        // Y (ground level)
        screen_width,               // X final (borda direita)
        level->ground_level,        // Y (ground level)
        line_color, 
        line_thickness
    );
}

void update_hitbox_position(struct Entity *entity, bool facing_right) {
    entity->hitbox.x = entity->x - (entity->hitbox.width / 2) + entity->hitbox.offset_x;
    
    entity->hitbox.y = entity->y - entity->hitbox.height + entity->hitbox.offset_y;
}

bool check_collision(struct Entity *a, struct Entity *b) {
    // A lógica de colisão original está um pouco estranha. Uma verificação AABB (Axis-Aligned Bounding Box)
    // padrão é mais confiável. Vamos assumir que a.hitbox.y é o TOPO e a.hitbox.y + height é a BASE.
    
    // A hitbox de A
    float a_top = a->hitbox.y;
    float a_bottom = a->hitbox.y + a->hitbox.height;
    float a_left = a->hitbox.x;
    float a_right = a->hitbox.x + a->hitbox.width;

    // A hitbox de B
    float b_top = b->hitbox.y;
    float b_bottom = b->hitbox.y + b->hitbox.height;
    float b_left = b->hitbox.x;
    float b_right = b->hitbox.x + b->hitbox.width;

    // Verifica se não há colisão
    if (a_right < b_left || a_left > b_right || a_bottom < b_top || a_top > b_bottom) {
        return false; // Sem colisão
    }

    // Se nenhuma das condições acima for verdadeira, há uma colisão
    return true;
}