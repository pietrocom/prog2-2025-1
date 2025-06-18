#include <stdio.h>

#include "types.h"
#include "enemy.h"
#include "game.h"
#include "player.h"
#include "utils.h"
#include "projectiles.h"

// Inicialização do sistema de inimigos

void init_enemy_system(struct EnemySystem *system) {
    // Inicializa todos os inimigos como inativos
    for (int i = 0; i < MAX_ENEMIES; i++) {
        system->enemies[i].is_active = false;
        system->enemies[i].hitbox_show = false;
    }
    
    // Configuração inicial do sistema
    system->spawn_timer = ENEMY_SPAWN_COOLDOWN;
    system->active_count = 0;
    system->wave_number = 0;
    
    // Inicializa o boss (mas não ativa ainda)
    init_boss(&system->boss);
    system->boss.is_active = false;
    
    // Seed para números aleatórios (usado no spawn)
    srand(al_get_time());
}

void update_enemy_system(struct EnemySystem *system, struct Player *player, struct GameLevel *level, float delta_time) {
    // Atualiza timer de spawn
    system->spawn_timer -= delta_time;
    
    // Verifica se é hora de spawnar uma nova wave
    if (system->spawn_timer <= 0 && system->active_count < MAX_ENEMIES) {
        spawn_enemy_wave(system, level);
        system->spawn_timer = ENEMY_SPAWN_COOLDOWN;
    }
    
    // Atualiza cada inimigo ativo
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (system->enemies[i].is_active) {
            update_enemy(&system->enemies[i], player, delta_time);
            
            // Remove inimigos mortos
            if (is_enemy_dead(&system->enemies[i])) {
                kill_enemy(&system->enemies[i]);
                system->active_count--;
            }
        }
    }
    
    // Atualiza o boss se estiver ativo
    if (system->boss.is_active) {
        update_boss(&system->boss, player, delta_time);
    }
}

void spawn_enemy_wave(struct EnemySystem *system, struct GameLevel *level) {
    // Define quantos inimigos spawnar nesta wave (mínimo 1, máximo 4)
    int enemies_to_spawn = 1 + (system->wave_number % 4);
    
    // Ajusta para não ultrapassar o limite máximo
    if (system->active_count + enemies_to_spawn > MAX_ENEMIES) {
        enemies_to_spawn = MAX_ENEMIES - system->active_count;
    }
    
    // Spawna os inimigos
    for (int i = 0; i < enemies_to_spawn; i++) {
        // Encontra um slot vazio
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!system->enemies[j].is_active) {
                // Posição X aleatória (direita da tela)
                float spawn_x = level->scroll_x + al_get_display_width(al_get_current_display()) + 100;
                
                // Posição Y no chão
                float spawn_y = level->ground_level;
                
                // Escolhe tipo de inimigo baseado na wave
                EnemyType type;
                if (system->wave_number < 2) {
                    type = ENEMY_MELEE; // Waves iniciais só tem melee
                } else if (system->wave_number < 5) {
                    // Waves 2-4 podem ter ranged
                    type = (rand() % 2) ? ENEMY_MELEE : ENEMY_RANGED;
                } 
                // Implementar com flying enemies
                
                // Inicializa o inimigo
                init_enemy(&system->enemies[j], type, spawn_x, spawn_y);
                system->enemies[j].facing_right = false; // Inimigos virados para esquerda
                
                system->active_count++;
                break;
            }
        }
    }
    
    // Incrementa o número da wave
    system->wave_number++;
    
    // Ativa o boss a cada 10 waves
    if (system->wave_number % 10 == 0 && !system->boss.is_active) {
        float boss_x = level->scroll_x + al_get_display_width(al_get_current_display()) + 200;
        float boss_y = level->ground_level - 100; // Boss fica um pouco acima do chão
        
        init_boss(&system->boss);
        system->boss.entity.x = boss_x;
        system->boss.entity.y = boss_y;
        system->boss.is_active = true;
        update_hitbox_position(&system->boss.entity, false);
    }
}


// Inimigos individuais
void init_enemy(struct Enemy *enemy, EnemyType type, float x, float y) {

    // Configuração básica da entidade
    enemy->entity.x = x;
    enemy->entity.y = y;
    enemy->type = type;
    enemy->is_active = true;
    enemy->facing_right = false; // Inimigos geralmente virados para esquerda
    
    // Configuração específica por tipo
    switch(type) {
        case ENEMY_MELEE:
            enemy->entity.width = 40;
            enemy->entity.height = 105;
            enemy->health = 100;
            enemy->max_health = 100;
            enemy->damage = 20;
            enemy->speed = 80.0f;
            enemy->attack_range = 60.0f;
            enemy->detection_range = 300.0f;
            enemy->attack_cooldown = 1.2f;
            break;
            
        case ENEMY_RANGED:
            enemy->entity.width = 45;
            enemy->entity.height = 100;
            enemy->health = 70;
            enemy->max_health = 70;
            enemy->damage = 15;
            enemy->speed = 60.0f;
            enemy->attack_range = 250.0f;
            enemy->detection_range = 400.0f;
            enemy->attack_cooldown = 2.0f;
            break;
            
        case ENEMY_BOSS:
            // Será implementado separadamente em init_boss()
            break;
    }
    
    // Configuração da hitbox (proporcional ao tamanho do sprite)
    enemy->entity.hitbox.width = enemy->entity.width * 0.7f;
    enemy->entity.hitbox.height = enemy->entity.height * 0.8f;
    enemy->entity.hitbox.offset_x = ENEMY_HITBOX_OFFSET_X;
    enemy->entity.hitbox.offset_y = ENEMY_HITBOX_OFFSET_Y;
    update_hitbox_position(&enemy->entity, enemy->facing_right);
    
    // Inicializa animações
    load_enemy_sprites(enemy, type);
    
    // Configura animação inicial
    enemy->current_animation = &enemy->animations[ENEMY_ANIM_IDLE];
}

void load_enemy_sprites(struct Enemy *enemy, EnemyType type) {
    // Mapeamento de pastas por tipo de inimigo
    const char* enemy_folders[] = {
        "assets/gangster_sprites/Gangsters_2/",     // ENEMY_MELEE
        "assets/gangster_sprites/Gangsters_1/",     // ENEMY_RANGED
        "assets/gangster_sprites/Gangsters_3"       // ENEMY_BOSS
    };

    // Nomes dos arquivos de animação (deve corresponder à enum EnemyAnimState)
    const char* animation_files[] = {
        "Idle.png",       // ENEMY_ANIM_IDLE
        "Walk.png",       // ENEMY_ANIM_WALK
        "Attack_1.png",   // ENEMY_ANIM_ATTACK
        "Hurt.png",       // ENEMY_ANIM_HURT
        "Dead.png"        // ENEMY_ANIM_DEATH
    };

    char full_path[256];
    const char* base_folder = enemy_folders[type];

    // Carrega cada animação
    for (int i = 0; i < ENEMY_ANIM_COUNT; i++) {
        snprintf(full_path, sizeof(full_path), "%s%s", base_folder, animation_files[i]);
        
        // Verifica se o arquivo existe antes de carregar
        if (!file_exists(full_path)) {
            fprintf(stderr, "Warning: Enemy sprite not found: %s\n", full_path);
            continue;
        }

        // Divide o spritesheet em frames individuais
        split_spritesheet(full_path, 
                        SPRITE_SIZE, 
                        SPRITE_SIZE,
                        enemy->animations[i].frames,
                        &enemy->animations[i].frame_count);

        // Configura os tempos de animação baseados no tipo
        switch(i) {
            case ENEMY_ANIM_IDLE:
                enemy->animations[i].frame_delay = 0.15f;
                break;
            case ENEMY_ANIM_WALK:
                enemy->animations[i].frame_delay = 0.1f;
                break;
            case ENEMY_ANIM_ATTACK:
                enemy->animations[i].frame_delay = 0.08f;
                break;
            case ENEMY_ANIM_HURT:
                enemy->animations[i].frame_delay = 0.12f;
                break;
            case ENEMY_ANIM_DEATH:
                enemy->animations[i].frame_delay = 0.15f;
                break;
        }
    }
}

void unload_enemy_sprites(struct Enemy *enemy) {
    for (int i = 0; i < ENEMY_ANIM_COUNT; i++) {
        for (int j = 0; j < enemy->animations[i].frame_count; j++) {
            if (enemy->animations[i].frames[j]) {
                al_destroy_bitmap(enemy->animations[i].frames[j]);
                enemy->animations[i].frames[j] = NULL;
            }
        }
        enemy->animations[i].frame_count = 0;
    }
}


// Controle

void update_enemy(struct Enemy *enemy, struct Player *player, float delta_time) {}
void enemy_ai(struct Enemy *enemy, struct Player *player, float delta_time) {}
void enemy_move(struct Enemy *enemy, float dx, float dy) {}
void enemy_attack(struct Enemy *enemy, struct Player *player) {}
void enemy_ranged_attack(struct Enemy *enemy, struct Projectile *projectiles[], int *projectile_count) {}


// Estado

void damage_enemy(struct Enemy *enemy, int amount) {}
bool is_enemy_dead(struct Enemy *enemy) {}
void kill_enemy(struct Enemy *enemy) {}


// Renderização

void draw_enemy(struct Enemy *enemy) {}
void draw_enemy_health_bar(struct Enemy *enemy) {}


// Chefe

void init_boss(struct Enemy *boss) {}
void update_boss(struct Enemy *boss, struct Player *player, float delta_time) {}
void boss_attack_pattern(struct Enemy *boss, struct Player *player, struct Projectile *projectiles[], int *projectile_count) {}