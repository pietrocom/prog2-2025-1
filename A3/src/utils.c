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

// Inicializa a biblioteca Allegro e todos os seus addons necessários
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

// Cria e retorna a janela principal do jogo
ALLEGRO_DISPLAY * create_display (int width, int height) {
    ALLEGRO_DISPLAY *display = al_create_display(width, height);
    if (!display) {
        fprintf(stderr, "Failed to create display.\n");
        exit(EXIT_FAILURE);
    }
    return display;
}

// Divide uma folha de sprites horizontal em frames individuais
void split_spritesheet(const char *filename, int frame_width, int frame_height,
                      ALLEGRO_BITMAP **frames, int *frame_count) 
{
    ALLEGRO_BITMAP *sheet = al_load_bitmap(filename);
    if (!sheet) {
        fprintf(stderr, "Failed to load spritesheet: %s\n", filename);
        *frame_count = 0;
        return;
    }

    int sheet_width = al_get_bitmap_width(sheet);
    *frame_count = sheet_width / frame_width;

    // Salva o alvo de desenho atual para restaurá-lo depois
    ALLEGRO_BITMAP *old_target = al_get_target_bitmap();

    for (int i = 0; i < *frame_count; i++) {
        // Impede que o número de frames ultrapasse o limite definido em types.h
        if (i >= MAX_FRAMES) {
            fprintf(stderr, "Warning: Spritesheet %s has more frames than MAX_FRAMES. Truncating.\n", filename);
            *frame_count = MAX_FRAMES;
            break;
        }

        frames[i] = al_create_bitmap(frame_width, frame_height);
        if (!frames[i]) {
            fprintf(stderr, "Failed to create frame %d for %s\n", i, filename);
            continue;
        }

        // Redireciona as operações de desenho para o novo bitmap de frame
        al_set_target_bitmap(frames[i]);
        
        // Limpa o bitmap com transparência para evitar "lixo" gráfico de frames anteriores
        al_clear_to_color(al_map_rgba(0, 0, 0, 0)); 

        // Desenha a região específica da spritesheet no bitmap do frame atual
        al_draw_bitmap_region(sheet, i * frame_width, 0, 
                             frame_width, frame_height,
                             0, 0, 0);
    }

    // Restaura o alvo de desenho original (geralmente a tela)
    al_set_target_bitmap(old_target);
    
    al_destroy_bitmap(sheet);
}

// Verifica se um determinado tipo de soldado tem a habilidade de se agachar
bool soldier_supports_crouch(SoldierType type) {
    // Apenas o Soldier 1 não tem a animação de agachar
    return type != SOLDIER_1;
}

// Verifica de forma segura se um arquivo existe no disco
bool file_exists(const char* filename) {
    struct stat buffer;
    // A função stat retorna 0 em caso de sucesso (arquivo encontrado)
    return (stat(filename, &buffer) == 0);
}

// Desenha uma linha vermelha no nível do chão para fins de debug
void draw_ground_line(struct GameLevel *level) {
    if (!level || !level->draw_ground_line) return;

    ALLEGRO_COLOR line_color = al_map_rgb(255, 0, 0);
    float line_thickness = 2.0f;
    int screen_width = al_get_display_width(al_get_current_display());
    
    al_draw_line(0, level->ground_level, screen_width, level->ground_level, line_color, line_thickness);
}

// Atualiza a posição da hitbox de uma entidade com base em sua âncora e direção
void update_hitbox_position(struct Entity *entity, bool facing_right) {
    // Aplica o offset horizontal na direção correta
    float directional_offset_x = facing_right ? entity->hitbox.offset_x : -entity->hitbox.offset_x;

    // Calcula o canto superior esquerdo da hitbox
    // A posição X é o centro da entidade, mais o offset, menos metade da largura da hitbox
    entity->hitbox.x = entity->x + directional_offset_x - (entity->hitbox.width / 2);
    // A posição Y é a base da entidade, menos a altura da hitbox, menos o offset vertical
    entity->hitbox.y = entity->y - entity->hitbox.height - entity->hitbox.offset_y;
}

// Verifica se as hitboxes de duas entidades estão se sobrepondo (AABB Collision)
bool check_collision(struct Entity *a, struct Entity *b) {
    // Coordenadas da hitbox A
    float a_left = a->hitbox.x;
    float a_right = a->hitbox.x + a->hitbox.width;
    float a_top = a->hitbox.y;
    float a_bottom = a->hitbox.y + a->hitbox.height;

    // Coordenadas da hitbox B
    float b_left = b->hitbox.x;
    float b_right = b->hitbox.x + b->hitbox.width;
    float b_top = b->hitbox.y;
    float b_bottom = b->hitbox.y + b->hitbox.height;

    // Se for possível encontrar qualquer eixo em que as caixas não se sobrepõem,
    // então não há colisão
    if (a_right < b_left ||  // A está totalmente à esquerda de B
        a_left > b_right ||  // A está totalmente à direita de B
        a_bottom < b_top ||  // A está totalmente acima de B
        a_top > b_bottom)    // A está totalmente abaixo de B
    {
        return false;
    }

    // Se nenhuma condição de separação foi encontrada, houve colisão
    return true;
}