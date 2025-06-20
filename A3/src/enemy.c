#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "enemy.h"
#include "boss.h"
#include "game.h"
#include "player.h"
#include "utils.h"
#include "projectiles.h"

// Inicialização do sistema de inimigos

void draw_enemy_health_bar(struct Enemy *enemy, float scroll_x);


// =================================================
// === SISTEMA DE INIMIGOS E ONDAS (WAVES) =========
// =================================================

void init_enemy_system(struct EnemySystem *system) {
    memset(system->enemies, 0, sizeof(system->enemies));
    system->active_count = 0;
    system->wave_number = 0;
    system->boss.is_active = false;
    srand(time(NULL));
}

void update_enemy_system(struct EnemySystem *system, struct Player *player, struct GameLevel *level, struct ProjectileSystem *projectile_system, float delta_time) {
    // 1. ATUALIZA INIMIGOS NORMAIS
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (system->enemies[i].is_active) {
            update_enemy(&system->enemies[i], player, level, projectile_system, delta_time);
        }
    }

    // 2. ATUALIZA O CHEFE (se estiver ativo)
    if (system->boss.is_active) {
        update_boss(&system->boss, player, level, projectile_system, delta_time);
    }
    
    // 3. CONTA INIMIGOS ATIVOS E VERIFICA LÓGICA DE SPAWN
    int current_active_enemies = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (system->enemies[i].is_active) {
            current_active_enemies++;
        }
    }
    system->active_count = current_active_enemies;

    // Se o jogo acabou de começar, spawna as waves iniciais.
    if (system->wave_number == 0) {
        printf("Iniciando o jogo: Spawning waves 1 e 2.\n");
        spawn_enemy_wave(system, level);
        spawn_enemy_wave(system, level);
    }
    // Se não há inimigos e o chefe não está ativo, spawna a próxima wave.
    else if (system->active_count == 0 && !system->boss.is_active && system->wave_number < 5) {
        printf("Wave %d eliminada. Spawning proxima wave.\n", system->wave_number);
        spawn_enemy_wave(system, level);
    } 
    // Se as waves acabaram e não há inimigos, spawna o chefe.
    else if (system->active_count == 0 && !system->boss.is_active && system->wave_number >= 5) {
        printf("Wave 5 eliminada. O CHEFE ESTA CHEGANDO!\n");
        float boss_x = level->scroll_x + al_get_display_width(al_get_current_display()) + 200;
        float boss_y = level->ground_level;
        init_boss(&system->boss, boss_x, boss_y);
        system->wave_number++; // Incrementa para não entrar mais aqui
    }
}

void spawn_enemy_wave(struct EnemySystem *system, struct GameLevel *level) {
    system->wave_number++;
    int enemies_to_spawn = 2 + (system->wave_number / 2); 
    if (enemies_to_spawn > 7) enemies_to_spawn = 7;
    if (system->active_count + enemies_to_spawn > MAX_ENEMIES) {
        enemies_to_spawn = MAX_ENEMIES - system->active_count;
    }
    if (enemies_to_spawn <= 0) return;
    const float GAP_BETWEEN_ENEMIES = 40.0f;
    float base_spawn_x = level->scroll_x + al_get_display_width(al_get_current_display()) + (rand() % 150 + 100);
    float spawn_y = level->ground_level;
    int spawned_count = 0;
    for (int i = 0; i < MAX_ENEMIES && spawned_count < enemies_to_spawn; i++) {
        if (!system->enemies[i].is_active) {
            float spawn_x = base_spawn_x + (spawned_count * (ENEMY_WIDTH + GAP_BETWEEN_ENEMIES));
            EnemyType type = (system->wave_number < 2) ? ENEMY_MELEE : ((rand() % 3 == 0) ? ENEMY_RANGED : ENEMY_MELEE);
            init_enemy(&system->enemies[i], type, spawn_x, spawn_y);
            spawned_count++;
        }
    }
}

void destroy_enemy_system(struct EnemySystem *system) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (system->enemies[i].is_active) {
             unload_enemy_sprites(&system->enemies[i]);
        }
    }
    if (system->boss.is_active) {
        unload_boss_sprites(&system->boss);
    }
}

// =================================================
// === INIMIGOS COMUNS =============================
// =================================================

void init_enemy(struct Enemy *enemy, EnemyType type, float x, float y) {
    memset(enemy, 0, sizeof(struct Enemy));
    enemy->entity.x = x;
    enemy->entity.y = y;
    enemy->type = type;
    enemy->is_active = true;
    
    switch(type) {
        case ENEMY_MELEE:
            enemy->entity.width = ENEMY_WIDTH;
            enemy->entity.height = ENEMY_HEIGHT;
            enemy->health = 150;
            enemy->max_health = 150;
            enemy->damage = 20;
            enemy->speed = 120.0f;
            enemy->attack_range = 60.0f;
            enemy->attack_cooldown = 1.2f;
            // CORREÇÃO: Inicializa o range de detecção
            enemy->detection_range = DETECTION_RANGE; 
            break;
        case ENEMY_RANGED:
            enemy->entity.width = ENEMY_WIDTH;
            enemy->entity.height = ENEMY_HEIGHT - 5;
            enemy->health = 100;
            enemy->max_health = 100;
            enemy->damage = 15;
            enemy->speed = 90.0f;
            enemy->attack_range = 450.0f;
            enemy->attack_cooldown = 2.0f;
            // CORREÇÃO: Inicializa o range de detecção
            enemy->detection_range = DETECTION_RANGE;
            break;
    }

    enemy->entity.hitbox.width = enemy->entity.width * 0.7f;
    enemy->entity.hitbox.height = enemy->entity.height;
    
    load_enemy_sprites(enemy, type);
    update_hitbox_position(&enemy->entity, enemy->facing_right);
    enemy->current_animation = &enemy->animations[ENEMY_ANIM_IDLE];
}

void load_enemy_sprites(struct Enemy *enemy, EnemyType type) {
    const char* enemy_folders[] = {
        "assets/gangster_sprites/Gangsters_2/", // ENEMY_MELEE
        "assets/gangster_sprites/Gangsters_1/"  // ENEMY_RANGED
    };
    // Esta função agora NUNCA será chamada com um tipo de chefe.
    // O 'if' foi removido porque 'ENEMY_BOSS' não existe mais na enum EnemyType.

    const char* animation_files[] = {
        "Idle.png", "Walk.png", "Attack_1.png", "Hurt.png", "Dead.png"
    };

    const char* base_folder = enemy_folders[type];
    char full_path[256];

    for (int i = 0; i < ENEMY_ANIM_COUNT; i++) {
        snprintf(full_path, sizeof(full_path), "%s%s", base_folder, animation_files[i]);
        if (file_exists(full_path)) {
            split_spritesheet(full_path, SPRITE_SIZE, SPRITE_SIZE,
                            enemy->animations[i].frames, &enemy->animations[i].frame_count);
            enemy->animations[i].frame_delay = (i == ENEMY_ANIM_ATTACK) ? 0.08f : 0.12f;
        }
    }
}

void unload_enemy_sprites(struct Enemy *enemy) {
    struct Animation* all_animations[ENEMY_ANIM_COUNT];
    for (int i = 0; i < ENEMY_ANIM_COUNT; i++) {
        all_animations[i] = &enemy->animations[i];
    }

    // Lista para rastrear ponteiros já liberados e evitar "double free"
    ALLEGRO_BITMAP* freed_pointers[ENEMY_ANIM_COUNT * MAX_FRAMES];
    int freed_count = 0;
    for(int i = 0; i < ENEMY_ANIM_COUNT * MAX_FRAMES; i++) freed_pointers[i] = NULL;

    for (int anim_idx = 0; anim_idx < ENEMY_ANIM_COUNT; anim_idx++) {
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
                    if (freed_count < ENEMY_ANIM_COUNT * MAX_FRAMES) {
                        freed_pointers[freed_count++] = anim->frames[i];
                    }
                }
                anim->frames[i] = NULL;
            }
        }
        anim->frame_count = 0;
    }
}


// Controle

void update_enemy(struct Enemy *enemy, struct Player *player, struct GameLevel *level, struct ProjectileSystem *projectile_system, float delta_time) {
    if (!enemy->is_active) {
        return; 
    }

    if (enemy->is_attacking && enemy->type == ENEMY_RANGED &&
        enemy->current_animation->current_frame == 3) { // Dispara no 4º frame (índice 3)
        
        // Verifica se o cooldown de tiro já passou para evitar múltiplos disparos por animação
        if (enemy->current_shoot_cooldown <= 0) {
            spawn_enemy_projectile(projectile_system, enemy);
            enemy->current_shoot_cooldown = enemy->attack_cooldown; // Usa o cooldown de ataque 
        }
    }

    if (enemy->is_dead) {
        bool death_anim_finished = (enemy->current_animation->current_frame == enemy->current_animation->frame_count - 1);
        if (death_anim_finished) {
            enemy->death_timer -= delta_time;
            if (enemy->death_timer <= 0) {
                enemy->is_active = false;
            }
        }
    } else {
        if (enemy->current_cooldown > 0) {
            enemy->current_cooldown -= delta_time;
        }
        if (enemy->current_shoot_cooldown > 0) {
            enemy->current_shoot_cooldown -= delta_time;
        }
        
        enemy_ai(enemy, player, level, projectile_system, delta_time); 
    }

    if (enemy->current_animation && enemy->current_animation->frame_count > 0) {
        enemy->current_animation->elapsed_time += delta_time;
        if (enemy->current_animation->elapsed_time >= enemy->current_animation->frame_delay) {
            enemy->current_animation->elapsed_time = 0;

            if (enemy->is_dead && enemy->current_animation == &enemy->animations[ENEMY_ANIM_DEATH]) {
                if (enemy->current_animation->current_frame < enemy->current_animation->frame_count - 1) {
                    enemy->current_animation->current_frame++;
                }
            } else {
                enemy->current_animation->current_frame = 
                    (enemy->current_animation->current_frame + 1) % enemy->current_animation->frame_count;
            }
        }
    }

    update_hitbox_position(&enemy->entity, enemy->facing_right);
}

void enemy_ai(struct Enemy *enemy, struct Player *player, struct GameLevel *level, struct ProjectileSystem *projectile_system, float delta_time) {
    // (Esta função está correta, sem alterações. O bug era na inicialização.)
    if (enemy->current_animation == &enemy->animations[ENEMY_ANIM_HURT]) {
        if (enemy->current_animation->current_frame >= enemy->current_animation->frame_count - 1) {
            enemy->current_animation = &enemy->animations[ENEMY_ANIM_IDLE];
        }
        return;
    }
    if (enemy->is_attacking) {
        if (enemy->current_animation->current_frame >= enemy->current_animation->frame_count - 1) {
            enemy->is_attacking = false;
            enemy->current_animation = &enemy->animations[ENEMY_ANIM_IDLE];
        }
        return;
    }
    
    float player_world_x = player->entity.x + level->scroll_x;
    float dx = player_world_x - enemy->entity.x;
    float distance = fabs(dx);
    enemy->facing_right = (dx > 0);

    // Agora esta lógica funcionará corretamente pois detection_range > 0
    switch(enemy->type) {
        case ENEMY_MELEE:
            if (distance < enemy->detection_range || enemy->has_been_aggroed) {
                if (distance > enemy->attack_range) {
                    float move_x = (dx > 0 ? 1 : -1) * enemy->speed * delta_time;
                    enemy_move(enemy, move_x, 0);
                    enemy->current_animation = &enemy->animations[ENEMY_ANIM_WALK];
                } 
                else if (enemy->current_cooldown <= 0) {
                    enemy_attack(enemy, player, level, projectile_system);
                } 
                else {
                    enemy->current_animation = &enemy->animations[ENEMY_ANIM_IDLE];
                }
            } else {
                enemy->current_animation = &enemy->animations[ENEMY_ANIM_IDLE];
            }
            break;

        case ENEMY_RANGED:
             if (distance < enemy->detection_range || enemy->has_been_aggroed) {
                if (distance < enemy->attack_range * 0.7f) {
                    float move_x = (dx > 0 ? -1 : 1) * enemy->speed * delta_time;
                    enemy_move(enemy, move_x, 0);
                    enemy->current_animation = &enemy->animations[ENEMY_ANIM_WALK];
                } 
                else if (distance > enemy->attack_range) { 
                    float move_x = (dx > 0 ? 1 : -1) * enemy->speed * delta_time;
                    enemy_move(enemy, move_x, 0);
                    enemy->current_animation = &enemy->animations[ENEMY_ANIM_WALK];
                }
                else if (enemy->current_cooldown <= 0) {
                    enemy_attack(enemy, player, level, projectile_system);
                } 
                else {
                    enemy->current_animation = &enemy->animations[ENEMY_ANIM_IDLE];
                }
            } else {
                enemy->current_animation = &enemy->animations[ENEMY_ANIM_IDLE];
            }
            break;
    }
}

void enemy_move(struct Enemy *enemy, float dx, float dy) {
    enemy->entity.x += dx;
    enemy->entity.y += dy;
}

void enemy_attack(struct Enemy *enemy, struct Player *player, struct GameLevel *level, struct ProjectileSystem *projectile_system) {
    (void)projectile_system;
    enemy->is_attacking = true;
    enemy->current_animation = &enemy->animations[ENEMY_ANIM_ATTACK];
    enemy->current_animation->current_frame = 0;
    enemy->current_animation->elapsed_time = 0;
    enemy->current_cooldown = enemy->attack_cooldown;
    
    if (enemy->type == ENEMY_MELEE) {
        float player_world_x = player->entity.x + level->scroll_x;
        float distance = fabs(player_world_x - enemy->entity.x);
        if (distance <= enemy->attack_range) {
            damage_player(player, enemy->damage);
        }
    }
}

void enemy_ranged_attack(struct Enemy *enemy, struct ProjectileSystem *projectile_system) {
    if (!enemy->is_active || enemy->current_shoot_cooldown > 0) return;
    
    // Configuração do projétil
    float direction = enemy->facing_right ? 1.0f : -1.0f;
    float start_x = enemy->entity.x + (direction * enemy->entity.width/2);
    float start_y = enemy->entity.y - enemy->entity.height/2;
    
    // Cria o projétil
    spawn_projectile(projectile_system, 
                    start_x, start_y,
                    direction, 
                    PROJECTILE_ENEMY, 
                    PROJECTILE_NORMAL,
                    enemy->damage);
    
    enemy->current_shoot_cooldown = enemy->shoot_cooldown;
    enemy->current_animation = &enemy->animations[ENEMY_ANIM_ATTACK];
}


// Estado

void damage_enemy(struct Enemy *enemy, int amount, struct Player *player) {
    if (enemy->is_dead) return;
    enemy->health -= amount;
    enemy->has_been_aggroed = true;
    if (enemy->health > 0) {
        enemy->current_animation = &enemy->animations[ENEMY_ANIM_HURT];
        enemy->current_animation->current_frame = 0;
    } else {
        kill_enemy(enemy, player);
    }
}

bool is_enemy_dead(struct Enemy *enemy) {
    return enemy->is_dead || enemy->health <= 0;
}

void kill_enemy(struct Enemy *enemy, struct Player *player) {
    if (enemy->is_dead) return;
    
    if(enemy->type == ENEMY_MELEE) player->score += 100;
    if(enemy->type == ENEMY_RANGED) player->score += 150;

    enemy->is_dead = true;
    enemy->health = 0; 
    enemy->entity.vel_x = 0; 
    enemy->current_animation = &enemy->animations[ENEMY_ANIM_DEATH];
    enemy->current_animation->current_frame = 0;
    enemy->death_timer = DEATH_FADEOUT_DELAY;
}

// Renderização

void draw_enemy(struct Enemy *enemy, float scroll_x) {
    if (!enemy || !enemy->is_active || !enemy->current_animation || enemy->current_animation->frame_count == 0) {
        return; 
    }

    ALLEGRO_BITMAP *frame = enemy->current_animation->frames[enemy->current_animation->current_frame];
    if (!frame) {
        return;
    }
    
    // 2. Cálculo de escala e dimensões do sprite
    float sprite_w = al_get_bitmap_width(frame);
    float sprite_h = al_get_bitmap_height(frame);
    float scaled_w = sprite_w * ENEMY_SCALE;
    float scaled_h = sprite_h * ENEMY_SCALE;
    
    // 3. Cálculo da posição de desenho na tela
    // O ponto de âncora (enemy->entity.x) é o centro da base do inimigo no mundo.
    // Subtraímos o scroll_x para obter a posição correta na tela.
    float draw_x = enemy->entity.x - (scaled_w / 2) - scroll_x;
    float draw_y = enemy->entity.y - scaled_h;
    
    // 4. Determina se o sprite deve ser espelhado horizontalmente
    int flags = enemy->facing_right ? 0 : ALLEGRO_FLIP_HORIZONTAL;
    
    // 5. Desenha o sprite na tela
    al_draw_scaled_bitmap(frame, 0, 0, sprite_w, sprite_h, draw_x, draw_y, scaled_w, scaled_h, flags);
    
    // 6. (Opcional) Desenha a hitbox para depuração
    if (enemy->hitbox_show) {
        // As coordenadas da hitbox também precisam ser ajustadas pelo scroll da câmera
        float hitbox_draw_x = enemy->entity.hitbox.x - scroll_x;
        float hitbox_draw_y = enemy->entity.hitbox.y;

        al_draw_rectangle(
            hitbox_draw_x, 
            hitbox_draw_y,
            hitbox_draw_x + enemy->entity.hitbox.width,
            hitbox_draw_y + enemy->entity.hitbox.height,
            al_map_rgb(255, 0, 0), 2);
    }
    
    // 7. Desenha a barra de vida do inimigo
    draw_enemy_health_bar(enemy, scroll_x);
}

void draw_enemies(struct EnemySystem *system, struct GameLevel *level, struct Player *player) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (system->enemies[i].is_active) {
            system->enemies[i].hitbox_show = player->hitbox_show;
            draw_enemy(&system->enemies[i], level->scroll_x);
        }
    }
    if (system->boss.is_active) {
        draw_boss(&system->boss, level->scroll_x);
    }
}

void draw_enemy_health_bar(struct Enemy *enemy, float scroll_x) {
    if (enemy->health <= 0) return;
    
    float bar_width = 60.0f;
    float bar_height = 6.0f;
    float health_percent = (float)enemy->health / (float)enemy->max_health;
    
    float bar_y = (enemy->entity.y - enemy->entity.hitbox.height) - 15; // X pixels acima da hitbox
    float bar_x = enemy->entity.x - (bar_width / 2) - scroll_x;
    
    ALLEGRO_COLOR health_color = (health_percent > 0.6f) ? al_map_rgb(0, 255, 0) :
                               (health_percent > 0.3f) ? al_map_rgb(255, 255, 0) :
                               al_map_rgb(255, 0, 0);

    al_draw_filled_rectangle(bar_x, bar_y, bar_x + bar_width, bar_y + bar_height, al_map_rgb(50, 50, 50));
    al_draw_filled_rectangle(bar_x, bar_y, bar_x + bar_width * health_percent, bar_y + bar_height, health_color);
    al_draw_rectangle(bar_x, bar_y, bar_x + bar_width, bar_y + bar_height, al_map_rgb(255,255,255), 1.0f);
}