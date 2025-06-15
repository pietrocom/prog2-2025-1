#include <stdio.h>
#include "types.h"
#include "player.h"

// ---- Funções auxiliares ----

void split_spritesheet(ALLEGRO_BITMAP *sheet, int frame_width, int frame_height, 
    ALLEGRO_BITMAP **frames, int *frame_count) 
{
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

bool soldier_supports_crouch(SoldierType type) {
    return type != SOLDIER_1; // Apenas Soldier 1 não agacha
}


// ---- Inicialização ----

void init_player (struct Player * player) {
    // Entidade
    player->entity.x = 0;
    player->entity.y = 0;
    player->entity.width = PLAYER_WIDTH;
    player->entity.height = PLAYER_HEIGHT;
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
        "Idle.png",        // 0: Idle
        "Walk.png",        // 1: Walk
        "Run.png",         // 2: Run 
        "Jump.png",        // 3: Jump
        "Shot_1.png",      // 4: Shoot
        "Crouch.png",      // 5: Crouch (Soldier_1 não tem)
        "Crouch_shot.png"  // 6: Crouch Shot (Soldier_1 não tem)
    };

    SoldierType type = player->soldier_type;
    char path[256];

    for (int i = 0; i < 5; i++) { // Idle, Walk, Run, Jump
        snprintf(path, sizeof(path), "%s%s", soldier_folders[type], sprite_filenames[i]);
        ALLEGRO_BITMAP *sheet = al_load_bitmap(path);
        
        if (!sheet) {
            fprintf(stderr, "Failed to load spritesheet: %s\n", path);
            exit(-1);
        }

        struct Animation *anim = NULL;
        switch (i) {
            case 0: anim = &player->idle; break;
            case 1: anim = &player->walking; break;
            case 2: anim = &player->running; break; 
            case 3: anim = &player->jumping; break;
            case 4: anim = &player->shooting; break; 
        }

        split_spritesheet(sheet, SPRITE_SIZE, SPRITE_SIZE, anim->frames, &anim->frame_count);
        al_destroy_bitmap(sheet);
    }

    // Carrega sprites de crouch apenas se o soldado suportar
    if (soldier_supports_crouch(type)) {
        // Crouch
        snprintf(path, sizeof(path), "%s%s", soldier_folders[type], sprite_filenames[5]);
        ALLEGRO_BITMAP *crouch_sheet = al_load_bitmap(path);
        if (crouch_sheet) {
            split_spritesheet(crouch_sheet, SPRITE_SIZE, SPRITE_SIZE, player->crouching.frames, &player->crouching.frame_count);
            al_destroy_bitmap(crouch_sheet);
        }

        // Crouch Shot (opcional)
        snprintf(path, sizeof(path), "%s%s", soldier_folders[type], sprite_filenames[6]);
        ALLEGRO_BITMAP *crouch_shot_sheet = al_load_bitmap(path);
        if (crouch_shot_sheet) {
            // Implemente lógica para crouch_shot se necessário
        }
    } 
    else {
        // Soldier 1: Define crouch como idle (ou vazio)
        player->crouching = player->idle;       // Usa animação idle 
        player->crouch_shot = player->shooting; // Usa animação shooting
    }
}

void unload_player_sprites(struct Player *player) {
    for (int i = 0; i < 5; i++) {
        al_destroy_bitmap(player->sprites[i]);
    }

    // Libera as animações
    for (int i = 0; i < player->idle.frame_count; i++) {
        if (player->idle.frames[i]) al_destroy_bitmap(player->idle.frames[i]);
    }
    for (int i = 0; i < player->walking.frame_count; i++) {
        if (player->walking.frames[i]) al_destroy_bitmap(player->walking.frames[i]);
    }
    for (int i = 0; i < player->running.frame_count; i++) {
        if (player->running.frames[i]) al_destroy_bitmap(player->running.frames[i]);
    }
    for (int i = 0; i < player->jumping.frame_count; i++) {
        if (player->jumping.frames[i]) al_destroy_bitmap(player->jumping.frames[i]);
    }
    for (int i = 0; i < player->shooting.frame_count; i++) {
        if (player->shooting.frames[i]) al_destroy_bitmap(player->shooting.frames[i]);
    }
    
    if (soldier_supports_crouch(player->soldier_type)) {
        for (int i = 0; i < player->crouching.frame_count; i++) {
            if (player->crouching.frames[i]) al_destroy_bitmap(player->crouching.frames[i]);
        }
        for (int i = 0; i < player->crouch_shot.frame_count; i++) {
            if (player->crouch_shot.frames[i]) al_destroy_bitmap(player->crouch_shot.frames[i]);
        }
    }
}


// ---- Controles ----

void handle_player_input(struct Player *player, ALLEGRO_EVENT *event) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_UP:
                if (!player->is_jumping) {  // Só permite pular se não estiver no ar
                    player->entity.vel_y = PLAYER_JUMP_VEL;
                    player->is_jumping = true;
                }
                break;
            case ALLEGRO_KEY_DOWN:
                if (soldier_supports_crouch(player->soldier_type)) {
                    player->is_crouching = true; // Agachar
                }
                break;
            case ALLEGRO_KEY_LEFT:
                player->entity.vel_x = player->is_crouching ? 
                    (-PLAYER_MOVE_SPEED * 0.2f) : // Movimento mais caso agachado
                    (-PLAYER_MOVE_SPEED);
                break;
            case ALLEGRO_KEY_RIGHT:
                player->entity.vel_x = PLAYER_MOVE_SPEED;  // Mover para a direita
                player->facing_right = true;
                break;
            case ALLEGRO_KEY_SPACE:
                player->is_shooting = true; // Atirar
                break;
        }
    } 
    else if (event->type == ALLEGRO_EVENT_KEY_UP) {
        switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_DOWN:
                player->is_crouching = false;
                break;
                
            case ALLEGRO_KEY_LEFT:
                if (player->entity.vel_x < 0) // Só zera se ainda estiver indo para esquerda
                    player->entity.vel_x = 0;
                break;
                
            case ALLEGRO_KEY_RIGHT:
                if (player->entity.vel_x > 0) // Só zera se ainda estiver indo para direita
                    player->entity.vel_x = 0;
                break;
                
            case ALLEGRO_KEY_SPACE:
                player->is_shooting = false;
                break;
        }
    }
}

void update_player(struct Player *player, float delta_time) {

    player->entity.vel_y += GRAVITY * delta_time;

    // Atualiza a posição do jogador
    player->entity.x += player->entity.vel_x * delta_time;
    player->entity.y += player->entity.vel_y * delta_time;

    // Colisão com o chão
    if (player->entity.y >= GROUND_LEVEL) {
        player->entity.y = GROUND_LEVEL;
        player->entity.vel_y = 0;
        player->is_jumping = false;
    }

    // Máquina de estados para animações
    if (player->is_crouching) {
        player->current_animation = player->is_shooting ? &player->crouch_shot : &player->crouching;
    } 
    else if (player->is_jumping) {
        player->current_animation = &player->jumping;
    } 
    // Vai definir uma velocidade mínima para estar correndo
    else if (player->entity.vel_x > 0.1f || player->entity.vel_x < -0.1f) { 
        player->current_animation = (player->entity.vel_x > RUN_THRESHOLD || player->entity.vel_x < -RUN_THRESHOLD) 
                                 ? &player->running 
                                 : &player->walking;
    } 
    else {
        player->current_animation = &player->idle;
    }

    // Atualiza frame da animação
    player->current_animation->elapsed_time += delta_time;
    if (player->current_animation->elapsed_time >= player->current_animation->frame_delay) {
        player->current_animation->current_frame = 
            (player->current_animation->current_frame + 1) % 
            player->current_animation->frame_count;
        player->current_animation->elapsed_time = 0;
    }
}


// ---- Renderização ----

