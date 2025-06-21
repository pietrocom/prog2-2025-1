#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <allegro5/allegro_primitives.h>

#include "types.h"
#include "enemy.h"
#include "boss.h"
#include "utils.h"

// As funções auxiliares (kill_boss, draw_boss_health_bar, init, load, unload)
// permanecem as mesmas da sua versão. Incluindo aqui por completude.

void kill_boss(struct Boss *boss, struct Player *player) {
    if (boss->is_dead) return;
    
    boss->state = BOSS_STATE_DEAD;
    boss->is_dead = true;
    boss->health = 0;
    boss->entity.vel_x = 0;
    boss->entity.vel_y = 0;
    player->score += 5000;
    boss->current_animation = &boss->animations[BOSS_ANIM_DEATH];
    boss->current_animation->current_frame = 0;
    boss->death_timer = BOSS_DEATH_FADEOUT_TIME;
}

void draw_boss_health_bar(struct Boss *boss) {
    if (boss->is_dead) return;
    int display_w = al_get_display_width(al_get_current_display());
    float bar_margin = 20.0f;
    float bar_height = 25.0f;
    float bar_width = display_w - (bar_margin * 2);
    float health_percent = (float)boss->health / (float)boss->max_health;
    if (health_percent < 0) health_percent = 0;

    // VISUAL: Cores dinâmicas para a barra de vida
    ALLEGRO_COLOR fill_color;
    ALLEGRO_COLOR border_color;

    if (boss->is_enraged) {
        // Cores do Modo Fúria
        fill_color = al_map_rgb(255, 50, 50);   // Vermelho vibrante
        border_color = al_map_rgb(255, 200, 0); // Laranja/Dourado
    } else {
        // Cores Normais
        fill_color = al_map_rgb(200, 0, 0);   // Vermelho padrão
        border_color = al_map_rgb(255, 255, 255); // Branco
    }

    al_draw_filled_rectangle(bar_margin, bar_margin, bar_margin + bar_width, bar_margin + bar_height, al_map_rgb(50, 50, 50));
    al_draw_filled_rectangle(bar_margin, bar_margin, bar_margin + (bar_width * health_percent), bar_margin + bar_height, fill_color);
    al_draw_rectangle(bar_margin, bar_margin, bar_margin + bar_width, bar_margin + bar_height, border_color, 2.0f);
}

void init_boss(struct Boss *boss, float x, float y) {
    memset(boss, 0, sizeof(struct Boss));
    boss->entity.x = x;
    boss->entity.y = y;
    boss->is_active = true;
    boss->entity.width = 128;
    boss->entity.height = 128;
    boss->entity.hitbox.width = 80;
    boss->entity.hitbox.height = 115;
    boss->health = BOSS_HEALTH;
    boss->max_health = BOSS_HEALTH;
    boss->projectile_damage = BOSS_PROJECTILE_DAMAGE;
    boss->lunge_damage = BOSS_LUNGE_DAMAGE;
    boss->punch_damage = BOSS_PUNCH_DAMAGE;
    boss->state = BOSS_STATE_IDLE;
    boss->state_timer = 1.5f; // Começa a agir mais rápido
    boss->is_enraged = false;
    boss->poise_hits = 0;
    load_boss_sprites(boss);
    boss->current_animation = &boss->animations[BOSS_ANIM_IDLE];
}

void load_boss_sprites(struct Boss *boss) {
    const char* base_folder = "assets/gangster_sprites/Gangsters_3/";
    const char* animation_files[BOSS_ANIM_COUNT] = {
        "Idle.png", "Walk.png", "Run.png", "Jump.png", "Attack_1.png",
        "Attack_2.png", "Taunt.png", "Hurt.png", "Dead.png"
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
            case BOSS_ANIM_SHOOT:   boss->animations[i].frame_delay = BOSS_SHOOT_FRAME_DELAY; break;
            case BOSS_ANIM_LUNGE_ATTACK: boss->animations[i].frame_delay = 0.07f; break;
            case BOSS_ANIM_TAUNT:   boss->animations[i].frame_delay = 0.18f; break;
            case BOSS_ANIM_HURT:    boss->animations[i].frame_delay = 0.1f; break;
            case BOSS_ANIM_DEATH:   boss->animations[i].frame_delay = 0.15f; break;
        }
    }
}

void unload_boss_sprites(struct Boss *boss) {
    ALLEGRO_BITMAP* freed_pointers[BOSS_ANIM_COUNT * MAX_FRAMES] = {NULL};
    int freed_count = 0;
    for (int anim_idx = 0; anim_idx < BOSS_ANIM_COUNT; anim_idx++) {
        struct Animation* anim = &boss->animations[anim_idx];
        for (int i = 0; i < anim->frame_count; i++) {
            if (anim->frames[i]) {
                bool already_freed = false;
                for (int j = 0; j < freed_count; j++) { if (freed_pointers[j] == anim->frames[i]) { already_freed = true; break; } }
                if (!already_freed) {
                    al_destroy_bitmap(anim->frames[i]);
                    if (freed_count < BOSS_ANIM_COUNT * MAX_FRAMES) { freed_pointers[freed_count++] = anim->frames[i]; }
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
    if (!boss->is_active) return;

    // A lógica principal agora é separada: ou o chefe está morto, ou está vivo.
    if (boss->state == BOSS_STATE_DEAD) {
        boss->death_timer -= delta_time;
        if (boss->death_timer <= 0) {
            boss->is_active = false; // Desativa o chefe para que ele pare de ser desenhado/atualizado.
        }
    } else { // Toda a lógica de combate vai aqui dentro
        if (!boss->is_enraged && boss->health < boss->max_health / 2) {
            boss->is_enraged = true;
            boss->state = BOSS_STATE_COOLDOWN;
            boss->state_timer = 1.0f;
        }

        if (boss->state != BOSS_STATE_HURT) {
            boss->poise_hits = 0;
        }
        
        boss->entity.x += boss->entity.vel_x * delta_time;
        boss->entity.vel_y += GRAVITY * delta_time;
        boss->entity.y += boss->entity.vel_y * delta_time;
        if (boss->entity.y > level->ground_level) {
            boss->entity.y = level->ground_level;
            boss->entity.vel_y = 0;
        }

        float player_world_x = player->entity.x + level->scroll_x;
        float dx = player_world_x - boss->entity.x;
        float distance = fabs(dx);
        if (boss->state != BOSS_STATE_LUNGING && boss->state != BOSS_STATE_HURT && boss->state != BOSS_STATE_PUNCHING) {
            boss->facing_right = (dx > 0);
        }

        boss->state_timer -= delta_time;
        static float shoot_cooldown = 0;
        if(shoot_cooldown > 0) shoot_cooldown -= delta_time;

        switch (boss->state) {
            case BOSS_STATE_IDLE:
                boss->entity.vel_x = 0;
                boss->current_animation = &boss->animations[BOSS_ANIM_IDLE];
                if (boss->state_timer <= 0) {
                    if (distance < BOSS_PUNCH_RANGE) {
                        boss->state = BOSS_STATE_PUNCHING;
                        boss->state_timer = BOSS_PUNCH_DURATION;
                        boss->has_hit_with_punch = false;
                    } else {
                        int choice = rand() % 3;
                        if (choice == 0) {
                            boss->state = BOSS_STATE_SHOOTING;
                            boss->state_timer = BOSS_SHOOT_DURATION;
                        } else {
                            boss->state = BOSS_STATE_PREPARING_LUNGE;
                            boss->state_timer = boss->is_enraged ? BOSS_ENRAGED_LUNGE_PREP_TIME : BOSS_LUNGE_PREP_TIME;
                        }
                    }
                }
                break;

            case BOSS_STATE_HURT:
                boss->current_animation = &boss->animations[BOSS_ANIM_HURT];
                if (boss->state_timer <= 0) {
                    boss->state = BOSS_STATE_COOLDOWN;
                    boss->state_timer = 0.5f;
                }
                break;

            case BOSS_STATE_SHOOTING:
                boss->current_animation = &boss->animations[BOSS_ANIM_SHOOT];
                boss->entity.vel_x = 0;
                if (shoot_cooldown <= 0) {
                    struct Enemy temp_enemy_view = {.entity = boss->entity, .damage = boss->projectile_damage, .facing_right = boss->facing_right};
                    spawn_enemy_projectile(projectile_system, &temp_enemy_view);
                    shoot_cooldown = boss->is_enraged ? BOSS_ENRAGED_SHOOT_COOLDOWN : BOSS_SHOOT_COOLDOWN;
                }
                if (boss->state_timer <= 0) {
                    boss->state = BOSS_STATE_COOLDOWN;
                    boss->state_timer = boss->is_enraged ? BOSS_ENRAGED_POST_ATTACK_COOLDOWN : BOSS_POST_ATTACK_COOLDOWN;
                }
                break;

            case BOSS_STATE_PREPARING_LUNGE:
                boss->current_animation = &boss->animations[BOSS_ANIM_TAUNT];
                boss->entity.vel_x = 0;
                if (boss->state_timer <= 0) {
                    boss->state = BOSS_STATE_LUNGING;
                    boss->state_timer = BOSS_LUNGE_DURATION;
                    boss->has_hit_with_lunge = false;
                    float current_lunge_speed = boss->is_enraged ? (BOSS_LUNGE_SPEED * 1.2f) : BOSS_LUNGE_SPEED;
                    boss->entity.vel_x = boss->facing_right ? current_lunge_speed : -current_lunge_speed;
                }
                break;

            case BOSS_STATE_LUNGING:
                boss->current_animation = &boss->animations[BOSS_ANIM_RUN];
                float y_diff = fabs(boss->entity.y - player->entity.y);
                if (!boss->has_hit_with_lunge && y_diff < (player->entity.height * 0.5f)) {
                    struct Entity player_world_hitbox = player->entity;
                    player_world_hitbox.hitbox.x += level->scroll_x;
                    if(check_collision(&boss->entity, &player_world_hitbox)) {
                        damage_player(player, boss->lunge_damage);
                        boss->has_hit_with_lunge = true;
                    }
                }
                if(boss->state_timer <= 0) {
                    boss->entity.vel_x = 0;
                    boss->state = BOSS_STATE_COOLDOWN;
                    boss->state_timer = boss->is_enraged ? BOSS_ENRAGED_POST_ATTACK_COOLDOWN : BOSS_POST_ATTACK_COOLDOWN;
                }
                break;

            case BOSS_STATE_COOLDOWN:
                boss->current_animation = &boss->animations[BOSS_ANIM_IDLE];
                boss->entity.vel_x = 0;
                if (boss->state_timer <= 0) {
                    boss->state = BOSS_STATE_IDLE;
                    boss->state_timer = boss->is_enraged ? BOSS_ENRAGED_ATTACK_COOLDOWN : BOSS_ATTACK_COOLDOWN;
                }
                break;

            case BOSS_STATE_PUNCHING:
                boss->entity.vel_x = 0;
                boss->current_animation = &boss->animations[BOSS_ANIM_LUNGE_ATTACK]; 
                
                // Causa dano em um frame específico da animação para sincronia
                if (!boss->has_hit_with_punch && boss->current_animation->current_frame == 2) {
                    if (distance < BOSS_PUNCH_RANGE * 1.2f) { // Alcance um pouco maior que o gatilho
                        damage_player(player, boss->punch_damage);
                        boss->has_hit_with_punch = true;
                    }
                }

                if (boss->state_timer <= 0) {
                    boss->state = BOSS_STATE_COOLDOWN;
                    boss->state_timer = boss->is_enraged ? BOSS_ENRAGED_POST_ATTACK_COOLDOWN : BOSS_POST_ATTACK_COOLDOWN;
                }
                break;

            case BOSS_STATE_DEAD:
                break;
        }
    }

    // --- Atualização da Animação ---
    if (boss->current_animation && boss->current_animation->frame_count > 0) {
        boss->current_animation->elapsed_time += delta_time;
        if (boss->current_animation->elapsed_time >= boss->current_animation->frame_delay) {
            boss->current_animation->elapsed_time = 0;
            if (boss->state == BOSS_STATE_HURT || boss->state == BOSS_STATE_PREPARING_LUNGE ||
                boss->state == BOSS_STATE_PUNCHING || boss->state == BOSS_STATE_DEAD) {
                if (boss->current_animation->current_frame < boss->current_animation->frame_count - 1) {
                    boss->current_animation->current_frame++;
                }
            } else {
                boss->current_animation->current_frame = (boss->current_animation->current_frame + 1) % boss->current_animation->frame_count;
            }
        }
    }
    update_hitbox_position(&boss->entity, boss->facing_right);
}

void draw_boss(struct Boss *boss, float scroll_x, bool show_hitbox) {
    if (!boss->is_active || !boss->current_animation || boss->current_animation->frame_count == 0) return;
    ALLEGRO_BITMAP *frame = boss->current_animation->frames[boss->current_animation->current_frame];
    if (!frame) return;
    float sprite_w = al_get_bitmap_width(frame);
    float sprite_h = al_get_bitmap_height(frame);
    float scaled_w = sprite_w * ENEMY_SCALE;
    float scaled_h = sprite_h * ENEMY_SCALE;
    float draw_x = boss->entity.x - (scaled_w / 2) - scroll_x;
    float draw_y = boss->entity.y - scaled_h;
    int flags = boss->facing_right ? 0 : ALLEGRO_FLIP_HORIZONTAL;

    if (boss->is_enraged && boss->state != BOSS_STATE_DEAD && (int)(al_get_time() * 10) % 2 == 0) {
        al_draw_tinted_scaled_bitmap(frame, al_map_rgb(255, 120, 120),
            0, 0, sprite_w, sprite_h, draw_x, draw_y, scaled_w, scaled_h, flags);
    } else {
        al_draw_scaled_bitmap(frame, 0, 0, sprite_w, sprite_h, draw_x, draw_y, scaled_w, scaled_h, flags);
    }
    
    if (show_hitbox) {
        float hitbox_draw_x = boss->entity.hitbox.x - scroll_x;
        float hitbox_draw_y = boss->entity.hitbox.y;
        al_draw_rectangle(
            hitbox_draw_x, hitbox_draw_y,
            hitbox_draw_x + boss->entity.hitbox.width,
            hitbox_draw_y + boss->entity.hitbox.height,
            al_map_rgb(255, 0, 255), 2
        );
    }
    draw_boss_health_bar(boss);
}

void damage_boss(struct Boss *boss, int amount, struct Player *player) {
    if (boss->is_dead) return;

    boss->health -= amount;

    if (boss->health <= 0) {
        kill_boss(boss, player);
        return;
    }

    if (boss->state == BOSS_STATE_LUNGING || boss->state == BOSS_STATE_PUNCHING || boss->state == BOSS_STATE_PREPARING_LUNGE || boss->state == BOSS_STATE_SHOOTING) {
        return;
    }

    boss->poise_hits++;
    
    if (boss->poise_hits >= BOSS_POISE_THRESHOLD) {
        boss->poise_hits = 0;
        boss->state = BOSS_STATE_PREPARING_LUNGE;
        boss->state_timer = boss->is_enraged ? (BOSS_ENRAGED_LUNGE_PREP_TIME * 0.5f) : (BOSS_LUNGE_PREP_TIME * 0.8f);
    } else {
        boss->state = BOSS_STATE_HURT;
        boss->state_timer = BOSS_HURT_DURATION;
        boss->current_animation = &boss->animations[BOSS_ANIM_HURT];
        boss->current_animation->current_frame = 0;
        boss->entity.vel_x = 0;
    }
}

bool is_boss_dead(struct Boss *boss) {
    return boss->is_dead;
}
