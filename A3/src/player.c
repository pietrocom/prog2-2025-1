#include <stdio.h>
#include <math.h>
#include <sys/stat.h> 
#include <allegro5/allegro_primitives.h>

#include "types.h"
#include "player.h"
#include "utils.h"
#include "projectiles.h"

// ---- Funções Auxiliares ----

// Define a escala de renderização da sprite do jogador
void set_player_scale(struct Player *player, float scale) {
    if (scale <= 0) {
        fprintf(stderr, "Invalid scale value: %f\n", scale);
        return;
    }
    player->scale = scale;
}


// ---- Inicialização ----

// Inicializa a estrutura do jogador com todos os seus valores e estados padrão
void init_player (struct Player * player) {
    // Entidade e física
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

    player->scale = 1.0f;
    player->soldier_type = SOLDIER_2; // Define o tipo de soldado padrão

    // Atributos de combate
    player->max_health = PLAYER_MAX_HEALTH;
    player->health = PLAYER_MAX_HEALTH;
    player->score = 0;

    // Flags de estado inicial
    player->is_jumping = false;
    player->is_crouching = false;
    player->is_shooting = false;
    player->facing_right = true;
    player->hitbox_show = false;
    player->is_running = false;

    // Sistema de estamina
    player->stamina = MAX_STAMINA;
    player->max_stamina = MAX_STAMINA;

    // Sistema de disparos e recarga
    player->shoot_cooldown = PLAYER_PROJECTILE_COOLDOWN;
    player->current_shoot_cooldown = 0;
    player->max_ammo = MAX_AMMO;
    player->current_ammo = MAX_AMMO;
    player->is_reloading = false;
    player->current_reload_time = 0.0f;

    // Configuração inicial das animações (zerando timers e frames)
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

// Carrega todas as folhas de sprites do jogador e as associa às suas respectivas animações
void load_player_sprites(struct Player *player) {
    const char* soldier_folders[] = {
        "assets/soldier_sprites/Soldier_1/",
        "assets/soldier_sprites/Soldier_2/",
        "assets/soldier_sprites/Soldier_3/"
    };

    const char* animation_files[] = {
        "Idle.png", "Walk.png", "Run.png", "Jump.png", "Shot_1.png", 
        "Recharge.png", "Crouch.png", "Crouch_Shot.png"
    };

    SoldierType type = player->soldier_type;
    char full_path[256];

    // Carrega as animações básicas que todos os soldados possuem
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

    // Carrega animações opcionais, como a de se agachar
    if (soldier_supports_crouch(type)) {
        snprintf(full_path, sizeof(full_path), "%s%s", soldier_folders[type], animation_files[6]);
        split_spritesheet(full_path, SPRITE_SIZE, SPRITE_SIZE,
                         player->crouching.frames, &player->crouching.frame_count);

        snprintf(full_path, sizeof(full_path), "%s%s", soldier_folders[type], animation_files[7]);
        if (file_exists(full_path)) {
            split_spritesheet(full_path, SPRITE_SIZE, SPRITE_SIZE,
                            player->crouch_shot.frames, &player->crouch_shot.frame_count);
        } else {
            // Se não houver uma animação de tiro agachado, usa a de tiro normal como alternativa
            player->crouch_shot = player->shooting;
        }
    } else {
        // Para soldados que não se agacham, as animações de agachar apontam para outras animações
        player->crouching = player->idle;
        player->crouch_shot = player->shooting;
    }
}

// Libera da memória todas as sprites carregadas para o jogador
void unload_player_sprites(struct Player *player) {
    struct Animation* all_animations[] = {
        &player->idle, &player->walking, &player->running,
        &player->jumping, &player->shooting, &player->reloading,
        &player->crouching, &player->crouch_shot
    };
    int num_animations = sizeof(all_animations) / sizeof(all_animations[0]);

    // Estratégia para evitar erros de "double free" caso animações compartilhem os mesmos bitmaps
    ALLEGRO_BITMAP* freed_pointers[MAX_FRAMES * num_animations];
    int freed_count = 0;
    memset(freed_pointers, 0, sizeof(freed_pointers));

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

// Processa os inputs do teclado para controlar as ações do jogador
void handle_player_input(struct Player *player, ALLEGRO_EVENT *event, struct GameLevel *level) {
    // Lógica para quando uma tecla é pressionada
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
            // Tecla de Debug para mostrar/esconder hitboxes e a linha do chão
            case ALLEGRO_KEY_H:
                player->hitbox_show = !player->hitbox_show;
                level->draw_ground_line = !level->draw_ground_line;
                break;
        }
    } 
    // Lógica para quando uma tecla é solta
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

// Atualiza a lógica, física e animações do jogador a cada quadro
void update_player(struct Player *player, float delta_time, struct GameLevel *level, struct ProjectileSystem *projectile_system) {
    if (!player || !level) return;

    // --- Lógica de Movimento e Velocidade ---
    player->is_moving = (fabs(player->entity.vel_x) > 0.1f);
    if (player->is_moving) {
        float target_speed = (player->is_running && player->stamina > 0 && !player->is_crouching) 
                           ? PLAYER_RUN_SPEED 
                           : PLAYER_MOVE_SPEED;
        
        player->entity.vel_x = (player->entity.vel_x > 0) ? target_speed : -target_speed;
    }

    // --- Lógica de Estamina ---
    if (player->is_running && player->is_moving && !player->is_crouching) {
        player->stamina -= STAMINA_DEPLETION_RATE * delta_time;
        if (player->stamina <= 0) {
            player->stamina = 0;
            player->is_running = false; // Força a parada da corrida
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

    // --- Lógica de Recarga ---
    if (player->is_reloading) {
        player->current_reload_time -= delta_time;
        if (player->current_reload_time <= 0) {
            player->is_reloading = false;
            player->current_ammo = player->max_ammo;
            player->current_animation = &player->idle;
        }
    }

    // --- Física e Colisão ---
    player->entity.vel_y += GRAVITY * delta_time;
    player->entity.y += player->entity.vel_y * delta_time;
    handle_player_ground_collision(player, level);

    // Ajusta a altura da hitbox se o jogador estiver agachado
    player->entity.hitbox.height = player->is_crouching ? PLAYER_CROUCH_HEIGHT : PLAYER_HEIGHT;
    update_hitbox_position(&player->entity, player->facing_right);

    // --- Lógica de Disparos ---
    if (player->current_shoot_cooldown > 0) {
        player->current_shoot_cooldown -= delta_time;
    }

    // Verifica todas as condições para poder atirar
    if (player->is_shooting && player->current_shoot_cooldown <= 0 && player->current_ammo > 0 && !player->is_reloading) {
        spawn_player_projectile(projectile_system, player, level);
        player->current_shoot_cooldown = PLAYER_PROJECTILE_COOLDOWN;
        player->current_ammo--; 
        // Inicia a recarga automaticamente se a munição acabar
        if (player->current_ammo <= 0) {
            start_reload(player);
        }
    }

    // --- Máquina de Estados de Animação ---
    // Define a animação correta com base no estado atual do jogador, em ordem de prioridade
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
                (player->current_animation->current_frame + 1) % player->current_animation->frame_count;
        }
    }
}

// Verifica e corrige a posição do jogador para que ele não caia através do chão
void handle_player_ground_collision(struct Player *player, struct GameLevel *level) {
    if (player->entity.y > level->ground_level) {
        player->entity.y = level->ground_level; // Reposiciona no chão
        player->entity.vel_y = 0;               // Zera a velocidade vertical
        player->is_jumping = false;             // Sai do estado de pulo
    }
}


// ---- Estado do Jogador ----

// Verifica se a vida do jogador chegou a zero ou menos
bool is_player_dead(struct Player *player) {
    if (!player) return false;
    return player->health <= 0;
}

// Reduz a vida do jogador em uma determinada quantidade
void damage_player(struct Player *player, int amount) {
    if (!player) return;
    player->health -= amount;
    // Impede que a vida fique com um valor negativo
    if (player->health < 0) {
        player->health = 0;
    }
}

// Inicia o processo de recarga de munição
void start_reload(struct Player *player) {
    // Impede a recarga se já estiver recarregando ou com o pente cheio
    if (player->is_reloading || player->current_ammo == player->max_ammo) {
        return;
    }

    player->is_reloading = true;
    player->current_reload_time = RELOAD_TIME;
    player->current_animation = &player->reloading;
}


// ---- Renderização ----

// Desenha a sprite de animação atual do jogador na tela
void draw_player(struct Player *player) {
    if (!player || !player->current_animation || player->current_animation->frame_count <= 0) return;

    ALLEGRO_BITMAP *frame = player->current_animation->frames[player->current_animation->current_frame];
    if (!frame) return;

    set_player_scale(player, PLAYER_SCALE);

    // Calcula as dimensões e a posição para desenhar a sprite
    float sprite_w = al_get_bitmap_width(frame);
    float sprite_h = al_get_bitmap_height(frame);
    float scaled_w = sprite_w * player->scale;
    float scaled_h = sprite_h * player->scale;

    // Calcula a posição do canto superior esquerdo (draw_x, draw_y) a partir da âncora (centro-base)
    float final_offset_x = player->facing_right ? PLAYER_SPRITE_OFFSET_X : -PLAYER_SPRITE_OFFSET_X;
    float draw_x = player->entity.x - (scaled_w / 2) + final_offset_x;
    float draw_y = player->entity.y - scaled_h;

    // Espelha a sprite horizontalmente se o jogador estiver virado para a esquerda
    int flags = player->facing_right ? 0 : ALLEGRO_FLIP_HORIZONTAL;

    al_draw_scaled_bitmap(frame, 0, 0, sprite_w, sprite_h, draw_x, draw_y, scaled_w, scaled_h, flags);

    // Desenha a hitbox se a flag de debug estiver ativa
    if (player->hitbox_show) {
        show_player_hitbox(player);
    }
}

// Desenha a hitbox e o ponto de âncora do jogador para fins de debug
void show_player_hitbox(struct Player *player) {
    if (!player) return;

    // O retângulo vermelho representa a área de colisão (hitbox)
    al_draw_rectangle(
        player->entity.hitbox.x,
        player->entity.hitbox.y,
        player->entity.hitbox.x + player->entity.hitbox.width,
        player->entity.hitbox.y + player->entity.hitbox.height,
        al_map_rgb(255, 0, 0), 2.0f);
    
    // O círculo verde representa o ponto de âncora (x,y) da entidade
    al_draw_filled_circle(
        player->entity.x, 
        player->entity.y, 
        4, al_map_rgb(0, 255, 0));
}