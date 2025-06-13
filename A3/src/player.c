#include <stdio.h>
#include "player.h"

void init_player (struct Player * player) {
    // Entidade
    player->entity.x = 0;
    player->entity.y = 0;
    player->entity.width = PLAYER_W;
    player->entity.height = PLAYER_H;
    player->entity.vel_x = 0;
    player->entity.vel_y = 0;

    // Bitmap sera carregado em load_player_sprites

    // Vai carregar o soldado correto (padrão é o 1)
    player->soldier_type = SOLDIER_1;

    // Status
    player->health = 100;
    player->score = 0;

    // Estado
    player->is_jumping = false;
    player->is_crouching = false;
    player->is_shooting = false;
    player->facing_right = true;
}

void load_player_sprites(struct Player *player) {
    const char *soldier_folders[] = {
        "assets/soldier_sprites/Soldier_1/",
        "assets/soldier_sprites/Soldier_2/",
        "assets/soldier_sprites/Soldier_3/"
    };

    const char *sprite_filenames[] = {
        "Idle.png",        // Parado (contém vários frames?)
        "Walk.png",        // Andando
        "Jump.png",        // Pulo
        "Crouch.png",      // TEM QUE SER FEITO!
        "Shot_1.png"       // Ataque (3 frames: parado, facada iniciando, facada completa)
    };

    SoldierType type = player->soldier_type;
    char path[256];

    // Carrega cada spritesheet e divide em frames
    for (int i = 0; i < 5; i++) {
        snprintf(path, sizeof(path), "%s%s", soldier_folders[type], sprite_filenames[i]);
        ALLEGRO_BITMAP *sheet = al_load_bitmap(path);
        
        if (!sheet) {
            fprintf(stderr, "Failed to load spritesheet: %s\n", path);
            exit(-1);
        }

        // Define a animação correspondente
        struct Animation *anim = NULL;
        switch (i) {
            case 0: anim = &player->idle; break;
            case 1: anim = &player->walking; break;
            case 2: anim = &player->jumping; break;
            case 3: anim = &player->crouching; break;
            case 4: anim = &player->attacking; break;
        }

        // Corta a spritesheet (assumindo que cada frame tem 32x32 pixels, ajuste conforme seus arquivos!)
        split_spritesheet(sheet, 32, 32, anim->frames, &anim->frame_count);
        anim->current_frame = 0;
        anim->frame_delay = 0.1f; // Troca de frame a cada 0.1 segundos (ajuste conforme necessário)
        anim->elapsed_time = 0.0f;

        al_destroy_bitmap(sheet); // A spritesheet original não é mais necessária
    }
}

void split_spritesheet(ALLEGRO_BITMAP *sheet, int frame_width, int frame_height, ALLEGRO_BITMAP **frames, int *frame_count) {
    int sheet_width = al_get_bitmap_width(sheet);
    int sheet_height = al_get_bitmap_height(sheet);
    *frame_count = sheet_width / frame_width; // Calcula quantos frames há na horizontal
    
    for (int i = 0; i < *frame_count; i++) {
        // Cria um bitmap para cada frame
        frames[i] = al_create_sub_bitmap(sheet, i * frame_width, 0, frame_width, frame_height);
        if (!frames[i]) {
            fprintf(stderr, "Failed to create sub-bitmap for frame %d\n", i);
            exit(-1);
        }
    }
}