#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <allegro5/allegro_primitives.h>

#include "types.h"
#include "enemy.h"
#include "boss.h"
#include "utils.h" 

// =================================================
// === FUNÇÕES AUXILIARES DO CHEFE =================
// =================================================

void kill_boss(struct Boss *boss, struct Player *player) {
    if (boss->is_dead) return;

    boss->health = 0;
    boss->is_dead = true;
    boss->entity.vel_x = 0;
    boss->entity.vel_y = 0;

    player->score += 5000; 

    boss->current_animation = &boss->animations[BOSS_ANIM_DEATH];
    boss->current_animation->current_frame = 0;
    boss->death_timer = 5.0f; 
}

void draw_boss_health_bar(struct Boss *boss) {
    if (boss->is_dead) return;

    // Barra de vida grande no topo da tela
    int display_w = al_get_display_width(al_get_current_display());
    float bar_margin = 20.0f;
    float bar_height = 25.0f;
    float bar_width = display_w - (bar_margin * 2);

    float health_percent = (float)boss->health / (float)boss->max_health;
    if (health_percent < 0) health_percent = 0;
    
    // Fundo da barra
    al_draw_filled_rectangle(bar_margin, bar_margin, bar_margin + bar_width, bar_margin + bar_height, al_map_rgb(50, 50, 50));
    
    // Vida atual
    al_draw_filled_rectangle(bar_margin, bar_margin, bar_margin + (bar_width * health_percent), bar_margin + bar_height, al_map_rgb(200, 0, 0));

    // Borda
    al_draw_rectangle(bar_margin, bar_margin, bar_margin + bar_width, bar_margin + bar_height, al_map_rgb(255, 255, 255), 2.0f);
}


// =================================================
// === FUNÇÕES DE CICLO DE VIDA DO CHEFE ===========
// =================================================

void init_boss(struct Boss *boss, float x, float y) {
    memset(boss, 0, sizeof(struct Boss));

    // Configuração da entidade
    boss->entity.x = x;
    boss->entity.y = y;
    boss->entity.width = 128; // Sprites de 128, mas a hitbox é menor
    boss->entity.height = 128;
    boss->entity.vel_x = 0;
    boss->entity.vel_y = 0;
    boss->is_active = true;
    
    // Hitbox (ajustada para o tamanho real do personagem no sprite)
    boss->entity.hitbox.width = 80;
    boss->entity.hitbox.height = 115;
    boss->entity.hitbox.offset_x = 0;
    boss->entity.hitbox.offset_y = 0;
    
    // Atributos de combate
    boss->health = BOSS_HEALTH;
    boss->max_health = BOSS_HEALTH;
    boss->projectile_damage = BOSS_PROJECTILE_DAMAGE;
    boss->lunge_damage = BOSS_LUNGE_DAMAGE;
    
    // Estado inicial da IA
    boss->state = BOSS_STATE_IDLE;
    boss->state_timer = 3.0f; // Pausa antes de começar a atacar

    // Carrega os recursos gráficos
    load_boss_sprites(boss);
    boss->current_animation = &boss->animations[BOSS_ANIM_IDLE];
}

void load_boss_sprites(struct Boss *boss) {
    const char* base_folder = "assets/gangster_sprites/Gangsters_3/";

    const char* animation_files[BOSS_ANIM_COUNT] = {
        "Idle.png",         // BOSS_ANIM_IDLE
        "Walk.png",         // BOSS_ANIM_WALK
        "Run.png",          // BOSS_ANIM_RUN
        "Jump.png",         // BOSS_ANIM_JUMP
        "Attack_1.png",     // BOSS_ANIM_SHOOT
        "Attack_2.png",     // BOSS_ANIM_LUNGE_ATTACK
        "Taunt.png",        // BOSS_ANIM_TAUNT
        "Hurt.png",         // BOSS_ANIM_HURT
        "Dead.png"          // BOSS_ANIM_DEATH
    };

    char full_path[256];

    for (int i = 0; i < BOSS_ANIM_COUNT; i++) {
        snprintf(full_path, sizeof(full_path), "%s%s", base_folder, animation_files[i]);
        
        if (!file_exists(full_path)) {
            fprintf(stderr, "Aviso: Sprite do chefe nao encontrado: %s. Pulando.\n", full_path);
            boss->animations[i].frame_count = 0;
            continue;
        }

        split_spritesheet(full_path, SPRITE_SIZE, SPRITE_SIZE,
                        boss->animations[i].frames, &boss->animations[i].frame_count);

        switch(i) {
            case BOSS_ANIM_IDLE:    boss->animations[i].frame_delay = 0.15f; break;
            case BOSS_ANIM_WALK:    boss->animations[i].frame_delay = 0.12f; break;
            case BOSS_ANIM_RUN:     boss->animations[i].frame_delay = 0.08f; break;
            case BOSS_ANIM_JUMP:    boss->animations[i].frame_delay = 0.1f;  break;
            case BOSS_ANIM_SHOOT:   boss->animations[i].frame_delay = 0.09f; break;
            case BOSS_ANIM_LUNGE_ATTACK: boss->animations[i].frame_delay = 0.07f; break;
            case BOSS_ANIM_TAUNT:   boss->animations[i].frame_delay = 0.18f; break;
            case BOSS_ANIM_HURT:    boss->animations[i].frame_delay = 0.12f; break;
            case BOSS_ANIM_DEATH:   boss->animations[i].frame_delay = 0.15f; break;
        }
    }
}

void unload_boss_sprites(struct Boss *boss) {
    // Lista para rastrear ponteiros já liberados e evitar "double free"
    ALLEGRO_BITMAP* freed_pointers[BOSS_ANIM_COUNT * MAX_FRAMES] = {NULL};
    int freed_count = 0;

    for (int anim_idx = 0; anim_idx < BOSS_ANIM_COUNT; anim_idx++) {
        struct Animation* anim = &boss->animations[anim_idx];
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
                    if (freed_count < BOSS_ANIM_COUNT * MAX_FRAMES) {
                        freed_pointers[freed_count++] = anim->frames[i];
                    }
                }
                anim->frames[i] = NULL;
            }
        }
        anim->frame_count = 0;
    }
}

// =================================================
// === FUNÇÕES DE JOGO (UPDATE E DRAW) =============
// =================================================

void update_boss(struct Boss *boss, struct Player *player, struct GameLevel *level, struct ProjectileSystem *projectile_system, float delta_time) {
    if (!boss->is_active || boss->is_dead) {
        return;
    }

    // --- FÍSICA E COLISÃO COM O CHÃO ---
    boss->entity.vel_y += GRAVITY * delta_time;
    boss->entity.y += boss->entity.vel_y * delta_time;

    if (boss->entity.y > level->ground_level) {
        boss->entity.y = level->ground_level;
        boss->entity.vel_y = 0;
        if (boss->state == BOSS_STATE_LUNGING) {
            boss->entity.vel_x = 0;
            boss->state = BOSS_STATE_COOLDOWN;
            boss->state_timer = BOSS_POST_LUNGE_COOLDOWN;
        }
    }

    // --- POSICIONAMENTO E DIREÇÃO ---
    float player_world_x = player->entity.x + level->scroll_x;
    float dx = player_world_x - boss->entity.x;
    float distance = fabs(dx);
    if (boss->state != BOSS_STATE_LUNGING) {
        boss->facing_right = (dx > 0);
    }

    // --- MÁQUINA DE ESTADOS DA IA ---
    boss->state_timer -= delta_time;
    static float shoot_cooldown = 0;
    if(shoot_cooldown > 0) shoot_cooldown -= delta_time;

    switch (boss->state) {
        case BOSS_STATE_IDLE:
            boss->current_animation = &boss->animations[BOSS_ANIM_IDLE];
            boss->entity.vel_x = 0;
            if (boss->state_timer <= 0) {
                int choice = rand() % 10;
                if (distance > 400 && choice < 7) { // 70% chance de atirar se longe
                    boss->state = BOSS_STATE_SHOOTING;
                    boss->state_timer = BOSS_SHOOT_DURATION;
                } else {
                    boss->state = BOSS_STATE_PREPARING_LUNGE;
                    boss->state_timer = BOSS_LUNGE_PREP_TIME;
                }
            }
            break;

        case BOSS_STATE_SHOOTING:
            boss->current_animation = &boss->animations[BOSS_ANIM_SHOOT];
            boss->entity.vel_x = 0;
            if (shoot_cooldown <= 0) {
                // Para spawnar o projétil, precisamos de uma struct Enemy temporária
                struct Enemy temp_enemy_view = {.entity = boss->entity, .damage = boss->projectile_damage, .facing_right = boss->facing_right};
                spawn_enemy_projectile(projectile_system, &temp_enemy_view);
                shoot_cooldown = BOSS_SHOOT_COOLDOWN;
            }
            if (boss->state_timer <= 0) {
                boss->state = BOSS_STATE_COOLDOWN;
                boss->state_timer = BOSS_ATTACK_COOLDOWN;
            }
            break;

        case BOSS_STATE_PREPARING_LUNGE:
            boss->current_animation = &boss->animations[BOSS_ANIM_TAUNT]; // Usa a provocação como "aviso"
            boss->entity.vel_x = 0;
            if (boss->state_timer <= 0) {
                boss->state = BOSS_STATE_LUNGING;
                boss->has_hit_with_lunge = false;
                boss->entity.vel_x = boss->facing_right ? BOSS_LUNGE_SPEED : -BOSS_LUNGE_SPEED;
                boss->entity.vel_y = BOSS_LUNGE_JUMP_VEL;
            }
            break;

        case BOSS_STATE_LUNGING:
            boss->current_animation = &boss->animations[BOSS_ANIM_RUN];
            
            struct Entity player_world_entity = player->entity;
            player_world_entity.hitbox.x += level->scroll_x;
            if (!boss->has_hit_with_lunge && check_collision(&boss->entity, &player_world_entity)) {
                damage_player(player, boss->lunge_damage);
                boss->has_hit_with_lunge = true;
            }
            break;

        case BOSS_STATE_COOLDOWN:
            boss->current_animation = &boss->animations[BOSS_ANIM_IDLE];
            boss->entity.vel_x = 0;
            if (boss->state_timer <= 0) {
                boss->state = BOSS_STATE_IDLE;
                boss->state_timer = 1.0f;
            }
            break;
    }

    // --- ATUALIZAÇÃO DA ANIMAÇÃO E HITBOX ---
    if (boss->current_animation->frame_count > 0) {
        boss->current_animation->elapsed_time += delta_time;
        if (boss->current_animation->elapsed_time >= boss->current_animation->frame_delay) {
            boss->current_animation->elapsed_time = 0;
            boss->current_animation->current_frame = 
                (boss->current_animation->current_frame + 1) % boss->current_animation->frame_count;
        }
    }
    update_hitbox_position(&boss->entity, boss->facing_right);
}

void draw_boss(struct Boss *boss, float scroll_x) {
    if (!boss->is_active || !boss->current_animation || boss->current_animation->frame_count == 0) return;
    
    ALLEGRO_BITMAP *frame = boss->current_animation->frames[boss->current_animation->current_frame];
    if (!frame) return;

    float sprite_w = al_get_bitmap_width(frame);
    float sprite_h = al_get_bitmap_height(frame);
    float scaled_w = sprite_w * ENEMY_SCALE;
    float scaled_h = sprite_h * ENEMY_SCALE;
    
    // Posição na tela (considerando o scroll)
    float draw_x = boss->entity.x - (scaled_w / 2) - scroll_x;
    float draw_y = boss->entity.y - scaled_h;
    
    int flags = boss->facing_right ? 0 : ALLEGRO_FLIP_HORIZONTAL;
    
    al_draw_scaled_bitmap(frame, 0, 0, sprite_w, sprite_h, draw_x, draw_y, scaled_w, scaled_h, flags);
    
    // Desenha a barra de vida
    draw_boss_health_bar(boss);
}

// =================================================
// === FUNÇÕES DE ESTADO (DANO E MORTE) ============
// =================================================

void damage_boss(struct Boss *boss, int amount, struct Player *player) {
    if (boss->is_dead) return;

    boss->health -= amount;
    
    if (boss->health > 0) {
        // Se estiver se preparando para a investida, o dano pode interromper!
        if (boss->state == BOSS_STATE_PREPARING_LUNGE) {
            boss->state = BOSS_STATE_COOLDOWN;
            boss->state_timer = 1.0f;
        }
        boss->current_animation = &boss->animations[BOSS_ANIM_HURT];
        boss->current_animation->current_frame = 0;
    } else {
        kill_boss(boss, player);
    }
}

bool is_boss_dead(struct Boss *boss) {
    return boss->is_dead;
}
