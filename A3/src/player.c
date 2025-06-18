#include <stdio.h>
#include <sys/stat.h> 
#include <allegro5/allegro_primitives.h>

#include "types.h"
#include "player.h"
#include "utils.h"
#include "projectiles.h"

// ---- Funções Auxiliares ----

void handle_player_ground_collision(struct Player *player, struct GameLevel *level) {
    if (player->entity.hitbox.y > level->ground_level) {
        // Calcula quanto o player afundou no chão
        float penetration_depth = player->entity.hitbox.y - level->ground_level;
        
        // Corrige a posição
        player->entity.y -= penetration_depth;
        player->entity.vel_y = 0;
        player->is_jumping = false;
        
        update_hitbox_position(&player->entity, player->facing_right);
    }
}

void set_player_scale(struct Player *player, float scale) {
    if (scale <= 0) {
        fprintf(stderr, "Invalid scale value: %f\n", scale);
        return;
    }
    player->scale = scale;
}


// ---- Inicialização ----

void init_player (struct Player * player) {
    // Entidade
    player->entity.x = 0;
    player->entity.y = 0;
    player->entity.width = PLAYER_WIDTH;
    player->entity.height = PLAYER_HEIGHT;

    player->entity.hitbox.width = PLAYER_WIDTH * 0.8f;  // 80% da largura do sprite
    player->entity.hitbox.height = PLAYER_HEIGHT * 0.9f; // 90% da altura
    player->entity.hitbox.offset_x = PLAYER_HITBOX_OFFSET_X;
    player->entity.hitbox.offset_y = PLAYER_HITBOX_OFFSET_Y;

    player->entity.vel_x = 0;
    player->entity.vel_y = 0;

    // Bitmap sera carregado em load_player_sprites
    player->scale = 1.0f;

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
    player->hitbox_show = false;

    // Disparos
    player->shoot_cooldown = PLAYER_PROJECTILE_COOLDOWN;
    player->current_shoot_cooldown = 0;
    player->can_shoot = true;

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

void handle_player_input(struct Player *player, ALLEGRO_EVENT *event, struct GameLevel *level) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_UP:
            case ALLEGRO_KEY_W:
                if (!player->is_jumping) {  // Só permite pular se não estiver no ar
                    player->entity.vel_y = PLAYER_JUMP_VEL;
                    player->is_jumping = true;
                }
                break;
            case ALLEGRO_KEY_DOWN:
            case ALLEGRO_KEY_S:
                if (soldier_supports_crouch(player->soldier_type)) {
                    player->is_crouching = true; // Agachar
                }
                break;
            case ALLEGRO_KEY_LEFT:
            case ALLEGRO_KEY_A:
                player->entity.vel_x = player->is_crouching ? 
                    (-PLAYER_MOVE_SPEED * 0.2f) : // Movimento mais caso agachado
                    (-PLAYER_MOVE_SPEED);
                player->is_moving = true;
                player->facing_right = false;
                break;
            case ALLEGRO_KEY_RIGHT:
            case ALLEGRO_KEY_D:
                player->entity.vel_x = player->is_crouching ? 
                    (PLAYER_MOVE_SPEED * 0.2f) : // Movimento mais caso agachado
                    (PLAYER_MOVE_SPEED);
                player->is_moving = true;
                player->facing_right = true;
                break;
            case ALLEGRO_KEY_SPACE:
                player->is_shooting = true; // Atirar
                break;
            case ALLEGRO_KEY_H:
                player->hitbox_show = !player->hitbox_show; // Alterna exibição da hitbox
                level->draw_ground_line = !level->draw_ground_line; 
                break;
        }
    } 
    else if (event->type == ALLEGRO_EVENT_KEY_UP) {
        switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_DOWN:
            case ALLEGRO_KEY_S:
                player->is_crouching = false;
                break;
                
            case ALLEGRO_KEY_LEFT:
            case ALLEGRO_KEY_A:
                if (player->entity.vel_x < 0) // Só zera se ainda estiver indo para esquerda
                    player->entity.vel_x = 0;
                player->is_moving = false;
                break;
                
            case ALLEGRO_KEY_RIGHT:
            case ALLEGRO_KEY_D:
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

void update_player(struct Player *player, float delta_time, struct GameLevel *level, struct ProjectileSystem *projectile_system) {
    if (!player || !level) {
        fprintf(stderr, "Invalid player or level in update_player\n");
        return;
    }

    // Ajuste de pulo para parecer natural
    player->entity.vel_y += GRAVITY * delta_time * 4;                // Aumentar para aumentar gravidade
    player->entity.y += player->entity.vel_y * delta_time * 12;      // Aumentar para pular mais alto
    
    update_hitbox_position(&player->entity, player->facing_right);

    handle_player_ground_collision(player, level);

    // Lógica para disparos
    if (player->current_shoot_cooldown > 0) {
        player->current_shoot_cooldown -= delta_time;
    } else {
        player->can_shoot = true;
    }
    if (player->is_shooting && player->can_shoot) {
        player->can_shoot = false;
        player->current_shoot_cooldown = PLAYER_PROJECTILE_COOLDOWN;
        spawn_player_projectile(projectile_system, player);
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


// ---- Estado do Jogador ----

bool is_player_dead(struct Player *player) {
    if (!player) {
        fprintf(stderr, "Invalid player in is_player_dead\n");
        return false;
    }
    return player->health <= 0;
}

void damage_player(struct Player *player, int amount) {
    if (!player) {
        fprintf(stderr, "Invalid player in damage_player\n");
        return;
    }
    player->health -= amount;
    if (player->health < 0) {
        player->health = 0; // Não deixa a vida ficar negativa
    }
}


// ---- Renderização ----

void draw_player(struct Player *player) {
    if (!player || !player->current_animation || 
        player->current_animation->current_frame < 0 ||
        player->current_animation->current_frame >= player->current_animation->frame_count) {
        return;
    }

    ALLEGRO_BITMAP *frame = player->current_animation->frames[player->current_animation->current_frame];
    if (!frame) return;

    if (player->hitbox_show)
        show_player_hitbox(player); 

    set_player_scale(player, PLAYER_SCALE);
    al_draw_scaled_bitmap(
        frame,
        0, 0, // Origem
        al_get_bitmap_width(frame), al_get_bitmap_height(frame), // Dimensões originais
        player->entity.hitbox.x - (player->facing_right ? PLAYER_RSPRITE_OFFSET_X : PLAYER_LSPRITE_OFFSET_X),  // Ajuste X
        player->entity.hitbox.y - (al_get_bitmap_height(frame) * player->scale), // Ajuste Y
        al_get_bitmap_width(frame) * player->scale,
        al_get_bitmap_height(frame) * player->scale,
        player->facing_right ? 0 : ALLEGRO_FLIP_HORIZONTAL
    );
}

void show_player_hitbox(struct Player *player) {
    if (!player) return;

    update_hitbox_position(&player->entity, player->facing_right);

    // Desenha a hitbox atualizada
    al_draw_rectangle(
        player->entity.hitbox.x,
        player->entity.hitbox.y,
        player->entity.hitbox.x + player->entity.hitbox.width,
        player->entity.hitbox.y - player->entity.hitbox.height,
        al_map_rgb(255, 0, 0), 2);
    
    // Ponto de referência (base do personagem)
    al_draw_filled_circle(
        player->entity.hitbox.x + player->entity.hitbox.width / 2, // Centraliza horizontalmente
        player->entity.hitbox.y - player->entity.hitbox.height / 2, // Centraliza verticalmente
        3, al_map_rgb(0, 255, 0));
}