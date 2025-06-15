#include <stdio.h>
#include <sys/stat.h> 
#include "types.h"
#include "player.h"

// ---- Funções auxiliares ----

void split_spritesheet(const char *filename, int frame_width, int frame_height,
                      ALLEGRO_BITMAP **frames, int *frame_count) 
{
    // Carrega a spritesheet uma vez
    ALLEGRO_BITMAP *sheet = al_load_bitmap(filename);
    if (!sheet) {
        fprintf(stderr, "Failed to load spritesheet: %s\n", filename);
        *frame_count = 0;
        return;
    }

    int sheet_width = al_get_bitmap_width(sheet);
    *frame_count = sheet_width / frame_width;

    ALLEGRO_BITMAP *old_target = al_get_target_bitmap();

    for (int i = 0; i < *frame_count; i++) {
        // Cria bitmap independente para cada frame
        frames[i] = al_create_bitmap(frame_width, frame_height);
        if (!frames[i]) {
            fprintf(stderr, "Failed to create frame %d\n", i);
            continue;
        }

        // Copia a região da spritesheet para o frame independente
        al_set_target_bitmap(frames[i]);
        al_draw_bitmap_region(sheet, i * frame_width, 0, 
                             frame_width, frame_height,
                             0, 0, 0);
    }

    // Restaura o target e limpa
    al_set_target_bitmap(old_target);
    al_destroy_bitmap(sheet); // Já podemos destruir a spritesheet
}

bool soldier_supports_crouch(SoldierType type) {
    return type != SOLDIER_1; // Apenas Soldier 1 não agacha
}

bool file_exists(const char* filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
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

    // Vai carregar o soldado correto (padrão é o 2)
    player->soldier_type = SOLDIER_1;

    // Status
    player->health = 100;
    player->score = 0;

    // Estado
    player->is_jumping = false;
    player->is_crouching = false;
    player->is_shooting = false;
    player->facing_right = true;

    // Inicializa animações
    player->idle.frame_delay = 0.1f;
    player->idle.elapsed_time = 0;
    player->idle.current_frame = 0;
    
    player->walking.frame_delay = 0.1f;
    player->walking.elapsed_time = 0;
    player->walking.current_frame = 0;
    
    player->running.frame_delay = 0.08f;
    player->running.elapsed_time = 0;
    player->running.current_frame = 0;
    
    player->jumping.frame_delay = 0.1f;
    player->jumping.elapsed_time = 0;
    player->jumping.current_frame = 0;
    
    player->shooting.frame_delay = 0.1f;
    player->shooting.elapsed_time = 0;
    player->shooting.current_frame = 0;
    
    player->crouching.frame_delay = 0.1f;
    player->crouching.elapsed_time = 0;
    player->crouching.current_frame = 0;
    
    player->crouch_shot.frame_delay = 0.1f;
    player->crouch_shot.elapsed_time = 0;
    player->crouch_shot.current_frame = 0;

    player->current_animation = &player->idle;
}

void load_player_sprites(struct Player *player) {
    const char* soldier_folders[] = {
        "assets/soldier_sprites/Soldier_1/",
        "assets/soldier_sprites/Soldier_2/",
        "assets/soldier_sprites/Soldier_3/"
    };

    const char* animation_files[] = {
        "Idle.png",    // 0
        "Walk.png",    // 1
        "Run.png",     // 2
        "Jump.png",    // 3
        "Shot_1.png",  // 4
        "Crouch.png",  // 5 (opcional)
        "Crouch_Shot.png" // 6 (opcional)
    };

    SoldierType type = player->soldier_type;
    char full_path[256];

    // Carrega cada animação
    for (int i = 0; i < 5; i++) { // Idle, Walk, Run, Jump, Shoot
        snprintf(full_path, sizeof(full_path), "%s%s", soldier_folders[type], animation_files[i]);
        
        struct Animation* target_anim = NULL;
        switch(i) {
            case 0: target_anim = &player->idle; break;
            case 1: target_anim = &player->walking; break;
            case 2: target_anim = &player->running; break;
            case 3: target_anim = &player->jumping; break;
            case 4: target_anim = &player->shooting; break;
        }

        if (target_anim) {
            split_spritesheet(full_path, SPRITE_SIZE, SPRITE_SIZE, 
                            target_anim->frames, &target_anim->frame_count);
        }
    }

    // Carrega animações opcionais (crouch)
    if (soldier_supports_crouch(type)) {
        // Crouch
        snprintf(full_path, sizeof(full_path), "%s%s", soldier_folders[type], animation_files[5]);
        split_spritesheet(full_path, SPRITE_SIZE, SPRITE_SIZE,
                         player->crouching.frames, &player->crouching.frame_count);

        // Crouch Shot (se existir)
        snprintf(full_path, sizeof(full_path), "%s%s", soldier_folders[type], animation_files[6]);
        if (file_exists(full_path)) {
            split_spritesheet(full_path, SPRITE_SIZE, SPRITE_SIZE,
                            player->crouch_shot.frames, &player->crouch_shot.frame_count);
        } else {
            // Fallback: usa animação normal de tiro
            player->crouch_shot = player->shooting;
        }
    } else {
        // Soldier 1 não tem crouch, usa fallbacks
        player->crouching = player->idle;
        player->crouch_shot = player->shooting;
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
                player->is_moving = true;
                player->facing_right = false;
                break;
            case ALLEGRO_KEY_RIGHT:
                player->entity.vel_x = player->is_crouching ? 
                    (PLAYER_MOVE_SPEED * 0.2f) : // Movimento mais caso agachado
                    (PLAYER_MOVE_SPEED);
                player->is_moving = true;
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
                player->is_moving = false;
                break;
                
            case ALLEGRO_KEY_RIGHT:
                if (player->entity.vel_x > 0) // Só zera se ainda estiver indo para direita
                    player->entity.vel_x = 0;
                player->is_moving = false;
                break;
                
            case ALLEGRO_KEY_SPACE:
                player->is_shooting = false;
                break;
        }
    }
}

void update_player(struct Player *player, float delta_time, struct GameLevel *level) {
    if (!player || !level) {
        fprintf(stderr, "Invalid player or level in update_player\n");
        return;
    }

    // Ajuste de pulo para parecer natural
    player->entity.vel_y += GRAVITY * delta_time * 4;                // Aumentar para aumentar gravidade
    player->entity.y += player->entity.vel_y * delta_time * 12;      // Aumentar para pular mais alto
    
    // Colisão com o chão
    if (player->entity.y >= level->ground_level - player->entity.height) {
        player->entity.y = level->ground_level - player->entity.height;
        player->entity.vel_y = 0;
        player->is_jumping = false;
    }

    // Máquina de estados para animações
    if (player->is_crouching) {
        player->current_animation = player->is_shooting ? &player->crouch_shot : &player->crouching;
    } 
    else if (player->is_shooting) {
        player->current_animation = &player->shooting;
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

void draw_player(struct Player *player) {
    if (!player || !player->current_animation || 
        !player->current_animation->frames || 
        player->current_animation->frame_count <= 0 ||
        player->current_animation->current_frame < 0 ||
        player->current_animation->current_frame >= player->current_animation->frame_count) {
        fprintf(stderr, "Invalid animation state in draw_player\n");
        return;
    }

    ALLEGRO_BITMAP *frame = player->current_animation->frames[player->current_animation->current_frame];
    if (!frame) {
        fprintf(stderr, "Invalid frame in animation\n");
        return;
    }

    int flags = player->facing_right ? 0 : ALLEGRO_FLIP_HORIZONTAL;
    al_draw_bitmap(frame, 
                  player->entity.x, 
                  player->entity.y - (SPRITE_SIZE - player->entity.height),
                  flags);
}