#include <stdio.h>
#include <math.h>
#include <sys/stat.h> 
#include <allegro5/allegro_primitives.h>

#include "types.h"
#include "player.h"
#include "utils.h"
#include "projectiles.h"

// ---- Funções Auxiliares ----

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

    player->entity.hitbox.width = PLAYER_WIDTH; 
    player->entity.hitbox.height = PLAYER_HEIGHT; 
    player->entity.hitbox.offset_x = PLAYER_HITBOX_OFFSET_X; 
    player->entity.hitbox.offset_y = PLAYER_HITBOX_OFFSET_Y;

    player->entity.vel_x = 0;
    player->entity.vel_y = 0;

    // Bitmap sera carregado em load_player_sprites
    player->scale = 1.0f;

    // Vai carregar o soldado correto (padrão é o 2)
    player->soldier_type = SOLDIER_2;

    // Status
    player->max_health = PLAYER_MAX_HEALTH;
    player->health = PLAYER_MAX_HEALTH;
    player->score = 0;

    // Estado
    player->is_jumping = false;
    player->is_crouching = false;
    player->is_shooting = false;
    player->facing_right = true;
    player->hitbox_show = false;
    player->is_running = false;

    // Estamina
    player->stamina = MAX_STAMINA;
    player->max_stamina = MAX_STAMINA;

    // Disparos
    player->shoot_cooldown = PLAYER_PROJECTILE_COOLDOWN;
    player->current_shoot_cooldown = 0;

    // Reload
    player->max_ammo = MAX_AMMO;
    player->current_ammo = MAX_AMMO;
    player->is_reloading = false;
    player->current_reload_time = 0.0f;

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

    player->reloading.frame_delay = 0.22f;
    player->reloading.elapsed_time = 0;
    player->reloading.current_frame = 0;
    
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
        "Recharge.png",// 5
        "Crouch.png",  // 6 (opcional)
        "Crouch_Shot.png" // 7 (opcional)
    };

    SoldierType type = player->soldier_type;
    char full_path[256];

    // Carrega cada animação
    for (int i = 0; i < 6; i++) { // Idle, Walk, Run, Jump, Shoot, Reload
        snprintf(full_path, sizeof(full_path), "%s%s", soldier_folders[type], animation_files[i]);
        
        struct Animation* target_anim = NULL;
        switch(i) {
            case 0: target_anim = &player->idle; break;
            case 1: target_anim = &player->walking; break;
            case 2: target_anim = &player->running; break;
            case 3: target_anim = &player->jumping; break;
            case 4: target_anim = &player->shooting; break;
            case 5: target_anim = &player->reloading; break;
        }

        if (target_anim) {
            split_spritesheet(full_path, SPRITE_SIZE, SPRITE_SIZE, 
                            target_anim->frames, &target_anim->frame_count);
        }
    }

    // Carrega animações opcionais (crouch)
    if (soldier_supports_crouch(type)) {
        // Crouch
        snprintf(full_path, sizeof(full_path), "%s%s", soldier_folders[type], animation_files[6]);
        split_spritesheet(full_path, SPRITE_SIZE, SPRITE_SIZE,
                         player->crouching.frames, &player->crouching.frame_count);

        // Crouch Shot (se existir)
        snprintf(full_path, sizeof(full_path), "%s%s", soldier_folders[type], animation_files[7]);
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
    struct Animation* all_animations[] = {
        &player->idle, &player->walking, &player->running,
        &player->jumping, &player->shooting, &player->reloading,
        &player->crouching, &player->crouch_shot
    };
    int num_animations = sizeof(all_animations) / sizeof(all_animations[0]);

    // Lista para rastrear ponteiros já liberados e evitar "double free"
    ALLEGRO_BITMAP* freed_pointers[MAX_FRAMES * num_animations];
    int freed_count = 0;
    for(int i = 0; i < MAX_FRAMES * num_animations; i++) freed_pointers[i] = NULL;

    for (int anim_idx = 0; anim_idx < num_animations; anim_idx++) {
        struct Animation* anim = all_animations[anim_idx];
        for (int i = 0; i < anim->frame_count; i++) {
            if (anim->frames[i]) {
                bool already_freed = false;
                for (int j = 0; j < freed_count; j++) {
                    if (freed_pointers[j] == anim->frames[i]) {
                        already_freed = true;
                        break;
                    }
                }

                if (!already_freed) {
                    al_destroy_bitmap(anim->frames[i]);
                    if (freed_count < MAX_FRAMES * num_animations) {
                        freed_pointers[freed_count++] = anim->frames[i];
                    }
                }
                anim->frames[i] = NULL;
            }
        }
        anim->frame_count = 0;
    }
}


// ---- Controles ----

void handle_player_input(struct Player *player, ALLEGRO_EVENT *event, struct GameLevel *level) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_LSHIFT:
            case ALLEGRO_KEY_RSHIFT:
                player->is_running = true;
                break;

            case ALLEGRO_KEY_UP:
            case ALLEGRO_KEY_W:
                if (!player->is_jumping) {
                    player->entity.vel_y = PLAYER_JUMP_VEL;
                    player->is_jumping = true;
                }
                break;

            case ALLEGRO_KEY_DOWN:
            case ALLEGRO_KEY_S:
                if (soldier_supports_crouch(player->soldier_type)) {
                    player->is_crouching = true;
                }
                break;

            case ALLEGRO_KEY_LEFT:
            case ALLEGRO_KEY_A:
                player->entity.vel_x = -PLAYER_MOVE_SPEED;
                player->facing_right = false;
                break;

            case ALLEGRO_KEY_RIGHT:
            case ALLEGRO_KEY_D:
                player->entity.vel_x = PLAYER_MOVE_SPEED;
                player->facing_right = true;
                break;

            case ALLEGRO_KEY_SPACE:
                player->is_shooting = true;
                break;
            case ALLEGRO_KEY_R:
                start_reload(player);
                break;
            case ALLEGRO_KEY_H:
                player->hitbox_show = !player->hitbox_show;
                level->draw_ground_line = !level->draw_ground_line;
                break;
        }
    } 
    else if (event->type == ALLEGRO_EVENT_KEY_UP) {
        switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_LSHIFT:
            case ALLEGRO_KEY_RSHIFT:
                player->is_running = false;
                break;

            case ALLEGRO_KEY_DOWN:
            case ALLEGRO_KEY_S:
                player->is_crouching = false;
                break;
                
            case ALLEGRO_KEY_LEFT:
            case ALLEGRO_KEY_A:
                if (player->entity.vel_x < 0) player->entity.vel_x = 0;
                break;

            case ALLEGRO_KEY_RIGHT:
            case ALLEGRO_KEY_D:
                if (player->entity.vel_x > 0) player->entity.vel_x = 0;
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

    player->is_moving = (fabs(player->entity.vel_x) > 0.1f);

    if (player->is_moving) {
        float target_speed;
        if (player->is_running && player->stamina > 0 && !player->is_crouching) {
            target_speed = PLAYER_RUN_SPEED;
        } else {
            target_speed = PLAYER_MOVE_SPEED;
        }
        
        // Aplica a velocidade correta à direção atual
        player->entity.vel_x = (player->entity.vel_x > 0) ? target_speed : -target_speed;
    }

    // --- Lógica de Estamina ---
    // Gasta estamina se estiver correndo E se movendo
    if (player->is_running && player->is_moving && !player->is_crouching) {
        player->stamina -= STAMINA_DEPLETION_RATE * delta_time;
        if (player->stamina <= 0) {
            player->stamina = 0;
            player->is_running = false; // Força a parada da corrida quando a estamina acaba
        }
    } else {
        // Regenera estamina se não estiver correndo
        if (player->stamina < player->max_stamina) {
            player->stamina += STAMINA_REGEN_RATE * delta_time;
            if (player->stamina > player->max_stamina) {
                player->stamina = player->max_stamina;
            }
        }
    }

    // Lógica de recarga
    if (player->is_reloading) {
        player->current_reload_time -= delta_time;
        if (player->current_reload_time <= 0) {
            // Terminou de recarregar
            player->is_reloading = false;
            player->current_ammo = player->max_ammo;
            player->current_animation = &player->idle;
            printf("Recarga completa!\n");
        }
    }

    // Aplica gravidade e atualiza posição Y
    player->entity.vel_y += GRAVITY * delta_time;
    player->entity.y += player->entity.vel_y * delta_time;
    
    // Checa colisão com o chão ANTES de atualizar a hitbox
    handle_player_ground_collision(player, level);

    // Lógica de hitbox caso esteja agachado
    if (player->is_crouching) {
        player->entity.hitbox.height = PLAYER_CROUCH_HEIGHT;
    } else {
        player->entity.hitbox.height = PLAYER_HEIGHT;
    }
    // Atualiza a posição da hitbox com base na posição final da entidade
    update_hitbox_position(&player->entity, player->facing_right);

    // Lógica dos disparos
    if (player->current_shoot_cooldown > 0) {
        player->current_shoot_cooldown -= delta_time;
    }

    // Condições para atirar: quer atirar, cooldown zerado, TEM MUNIÇÃO e NÃO ESTÁ RECARREGANDO
    if (player->is_shooting && player->current_shoot_cooldown <= 0 && player->current_ammo > 0 && !player->is_reloading) {
        spawn_player_projectile(projectile_system, player, level);
        player->current_shoot_cooldown = PLAYER_PROJECTILE_COOLDOWN;
        player->current_ammo--; 

        if (player->current_ammo <= 0) {
            start_reload(player);
        }
    }

    // Maquina de estados para animações
    if (player->is_reloading) {
        player->current_animation = &player->reloading;
    } else if (player->is_crouching) {
        player->current_animation = player->is_shooting && player->current_ammo > 0 ? &player->crouch_shot : &player->crouching;
    } else if (player->is_shooting && player->current_ammo > 0) {
        player->current_animation = &player->shooting;
    } else if (player->is_jumping) {
        player->current_animation = &player->jumping;
    } else if (player->is_moving) {
        player->current_animation = (player->is_running && player->stamina > 0) ? &player->running : &player->walking;
    } else {
        player->current_animation = &player->idle;
    }

    // --- Atualização do Frame da Animação ---
    if (player->current_animation && player->current_animation->frame_count > 0) {
        player->current_animation->elapsed_time += delta_time;
        if (player->current_animation->elapsed_time >= player->current_animation->frame_delay) {
            player->current_animation->elapsed_time = 0;
            player->current_animation->current_frame = 
                (player->current_animation->current_frame + 1) % 
                player->current_animation->frame_count;
        }
    }
}

void handle_player_ground_collision(struct Player *player, struct GameLevel *level) {
    // Se a base do jogador (entity.y) passou do nível do chão
    if (player->entity.y > level->ground_level) {
        player->entity.y = level->ground_level; // Corrige a posição
        player->entity.vel_y = 0;
        player->is_jumping = false;
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

void start_reload(struct Player *player) {
    // Não recarrega se já estiver recarregando ou se o pente estiver cheio
    if (player->is_reloading || player->current_ammo == player->max_ammo) {
        return;
    }
    printf("Recarregando!\n");
    player->is_reloading = true;
    player->current_reload_time = RELOAD_TIME;
    player->current_animation = &player->reloading; // Troca para a animação de recarga
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

    // Define a escala do jogador
    set_player_scale(player, PLAYER_SCALE);

    // 1. Pega as dimensões do sprite e aplica a escala
    float sprite_w = al_get_bitmap_width(frame);
    float sprite_h = al_get_bitmap_height(frame);
    float scaled_w = sprite_w * player->scale;
    float scaled_h = sprite_h * player->scale;

    // 2. Calcula a posição do canto superior esquerdo (draw_x, draw_y) para desenhar o sprite,
    //    baseado no ponto de âncora (entity.x, entity.y) que é o CENTRO da BASE.
    float final_offset_x = player->facing_right ? PLAYER_SPRITE_OFFSET_X : -PLAYER_SPRITE_OFFSET_X;
    float draw_x = player->entity.x - (scaled_w / 2) + final_offset_x;
    float draw_y = player->entity.y - scaled_h;

    // 3. Define se o sprite deve ser espelhado
    int flags = player->facing_right ? 0 : ALLEGRO_FLIP_HORIZONTAL;

    // 4. Desenha o bitmap
    al_draw_scaled_bitmap(
        frame,
        0, 0,          // Origem x, y no bitmap do frame
        sprite_w, sprite_h, // Dimensões da origem
        draw_x, draw_y,     // Posição x, y na tela (canto superior esquerdo)
        scaled_w, scaled_h, // Largura e altura final na tela
        flags
    );

    // O desenho da hitbox (opcional) também foi atualizado
    if (player->hitbox_show)
        show_player_hitbox(player); 
}

void show_player_hitbox(struct Player *player) {
    if (!player) return;

    // Desenha o retângulo da hitbox
    al_draw_rectangle(
        player->entity.hitbox.x,
        player->entity.hitbox.y,
        player->entity.hitbox.x + player->entity.hitbox.width,
        player->entity.hitbox.y + player->entity.hitbox.height,
        al_map_rgb(255, 0, 0), 2.0f);
    
    // Desenha um círculo no ponto de âncora para fácil visualização
    al_draw_filled_circle(
        player->entity.x, 
        player->entity.y, 
        4, al_map_rgb(0, 255, 0));
}