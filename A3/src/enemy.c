#include <allegro5/allegro_primitives.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "types.h"
#include "enemy.h"
#include "game.h"
#include "player.h"
#include "utils.h"
#include "projectiles.h"

// Inicialização do sistema de inimigos

void init_enemy_system(struct EnemySystem *system) {
    // Limpa o array de inimigos normais para começar
    memset(system->enemies, 0, sizeof(system->enemies));

    // Configuração inicial do sistema
    system->spawn_timer = ENEMY_SPAWN_COOLDOWN;
    system->active_count = 0;
    system->wave_number = 0;
    
    // Limpa a estrutura do boss e o inicializa
    memset(&system->boss, 0, sizeof(struct Enemy));
    init_boss(&system->boss);
    system->boss.is_active = false; // Desativa até que a wave correta chegue
    
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
                // Posição X aleatória (fora da tela, à direita)
                float spawn_x = level->scroll_x + al_get_display_width(al_get_current_display()) + (rand() % 200 + 100);
                
                // Posição Y no chão
                float spawn_y = level->ground_level;
                
                // Escolhe tipo de inimigo baseado na wave
                EnemyType type;
                if (system->wave_number < 2) {
                    type = ENEMY_MELEE; // Waves iniciais só tem melee
                } else if (system->wave_number < 5) {
                    // Waves 2-4 podem ter melee ou ranged
                    type = (rand() % 2) ? ENEMY_MELEE : ENEMY_RANGED;
                } else {
                    // --> CORREÇÃO ADICIONADA AQUI <--
                    // Garante que para todas as waves futuras, um tipo seja escolhido.
                    // Previne o uso de variável não inicializada que causava o crash.
                    type = (rand() % 2) ? ENEMY_MELEE : ENEMY_RANGED;
                }
                
                // Inicializa o inimigo
                init_enemy(&system->enemies[j], type, spawn_x, spawn_y);
                system->enemies[j].facing_right = false; // Inimigos sempre spawnan virados para o jogador (esquerda)
                
                system->active_count++;
                break; // Sai do loop interno, pois já achamos um slot e spawnamos um inimigo
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

void destroy_enemy_system(struct EnemySystem *system) {
    // Descarrega os sprites de todos os inimigos que foram inicializados
    for (int i = 0; i < MAX_ENEMIES; i++) {
        // Uma checagem simples para ver se o inimigo foi usado
        if (system->enemies[i].animations[ENEMY_ANIM_IDLE].frame_count > 0) {
             unload_enemy_sprites(&system->enemies[i]);
        }
    }
    // Descarrega o boss
    if (system->boss.animations[ENEMY_ANIM_IDLE].frame_count > 0) {
        unload_enemy_sprites(&system->boss);
    }
}


// Inimigos individuais
void init_enemy(struct Enemy *enemy, EnemyType type, float x, float y) {

    memset(enemy, 0, sizeof(struct Enemy));

    // Configuração básica da entidade
    enemy->ground_level = GROUND_LEVEL; 
    enemy->entity.x = x;
    enemy->entity.y = y;
    enemy->type = type;
    enemy->is_active = true;
    enemy->facing_right = false;
    
    // Configuração específica por tipo (seu código original)
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

    enemy->can_shoot = (type == ENEMY_RANGED || type == ENEMY_BOSS);
    
    // Configuração da hitbox (proporcional ao tamanho do sprite)
    enemy->entity.hitbox.width = enemy->entity.width * 0.7f;
    enemy->entity.hitbox.height = enemy->entity.height * 0.8f;
    enemy->entity.hitbox.offset_x = ENEMY_HITBOX_OFFSET_X;
    enemy->entity.hitbox.offset_y = ENEMY_HITBOX_OFFSET_Y;
    
    // Inicializa animações
    load_enemy_sprites(enemy, type);
    
    // Atualiza a hitbox e configura a animação inicial após o carregamento
    update_hitbox_position(&enemy->entity, enemy->facing_right);
    enemy->current_animation = &enemy->animations[ENEMY_ANIM_IDLE];
}

void load_enemy_sprites(struct Enemy *enemy, EnemyType type) {
    // Mapeamento de pastas por tipo de inimigo
    const char* enemy_folders[] = {
        "assets/gangster_sprites/Gangsters_2/",     // ENEMY_MELEE
        "assets/gangster_sprites/Gangsters_1/",     // ENEMY_RANGED
        "assets/gangster_sprites/Gangsters_3/"       // ENEMY_BOSS
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
    // Lista para rastrear ponteiros de bitmap que já foram liberados, evitando "double free".
    // O tamanho máximo é o número de tipos de animação vezes o máximo de frames.
    ALLEGRO_BITMAP* freed_pointers[ENEMY_ANIM_COUNT * MAX_FRAMES] = {NULL};
    int freed_count = 0;

    // Array para iterar sobre todas as animações do inimigo
    struct Animation* all_animations[ENEMY_ANIM_COUNT];
    for (int i = 0; i < ENEMY_ANIM_COUNT; i++) {
        all_animations[i] = &enemy->animations[i];
    }

    for (int anim_idx = 0; anim_idx < ENEMY_ANIM_COUNT; anim_idx++) {
        struct Animation* anim = all_animations[anim_idx];
        
        for (int i = 0; i < anim->frame_count; i++) {
            if (anim->frames[i]) {
                bool already_freed = false;
                
                // 1. Verifica se este ponteiro já está na nossa lista de liberados
                for (int j = 0; j < freed_count; j++) {
                    if (freed_pointers[j] == anim->frames[i]) {
                        already_freed = true;
                        break;
                    }
                }

                // 2. Se não foi liberado ainda, libere-o e adicione à lista
                if (!already_freed) {
                    al_destroy_bitmap(anim->frames[i]);
                    
                    if (freed_count < ENEMY_ANIM_COUNT * MAX_FRAMES) {
                        freed_pointers[freed_count++] = anim->frames[i];
                    }
                }
                
                // 3. Define o ponteiro como NULL para segurança
                anim->frames[i] = NULL;
            }
        }
        anim->frame_count = 0;
    }
}


// Controle

void update_enemy(struct Enemy *enemy, struct Player *player, float delta_time) {
    if (!enemy->is_active || enemy->is_dead) {
        return;
    }

    // Atualiza cooldown de disparo
    if (enemy->current_shoot_cooldown > 0) {
        enemy->current_shoot_cooldown -= delta_time;
    }

    // Atualiza cooldown do ataque
    if (enemy->current_cooldown > 0) {
        enemy->current_cooldown -= delta_time;
    }

    // Executa a lógica de IA
    enemy_ai(enemy, player, delta_time);

    // Atualiza animação
    if (enemy->current_animation) {
        enemy->current_animation->elapsed_time += delta_time;
        if (enemy->current_animation->elapsed_time >= enemy->current_animation->frame_delay) {
            enemy->current_animation->elapsed_time = 0;
            
            // Verifica se a animação tem frames antes de tentar acessá-los.
            // Isso previne o crash de divisão por zero (% 0).
            if (enemy->current_animation->frame_count > 0) {
                enemy->current_animation->current_frame = 
                    (enemy->current_animation->current_frame + 1) % enemy->current_animation->frame_count;
            
                // A verificação da animação de morte só faz sentido se o frame foi atualizado.
                if (enemy->is_dead && 
                    enemy->current_animation == &enemy->animations[ENEMY_ANIM_DEATH] &&
                    enemy->current_animation->current_frame == enemy->current_animation->frame_count - 1) {
                    enemy->is_active = false;
                }
            }
        }
    }

    // Atualiza posição da hitbox
    update_hitbox_position(&enemy->entity, enemy->facing_right);
}

void enemy_ai(struct Enemy *enemy, struct Player *player, float delta_time) {
    float dx = player->entity.x - enemy->entity.x;
    float dy = player->entity.y - enemy->entity.y;
    float distance = sqrtf(dx*dx + dy*dy);

    // Atualiza direção que o inimigo está olhando
    enemy->facing_right = (dx > 0);

    // Comportamento baseado no tipo de inimigo
    switch(enemy->type) {
        case ENEMY_MELEE:
            if (distance < enemy->detection_range) {
                if (distance > enemy->attack_range) {
                    // Persegue o jogador
                    float move_x = (dx / distance) * enemy->speed * delta_time;
                    enemy_move(enemy, move_x, 0);
                    enemy->current_animation = &enemy->animations[ENEMY_ANIM_WALK];
                } 
                else if (enemy->current_cooldown <= 0) {
                    // Ataque melee
                    enemy_attack(enemy, player);
                }
            } else {
                enemy->current_animation = &enemy->animations[ENEMY_ANIM_IDLE];
            }
            break;

        case ENEMY_RANGED:
            if (distance < enemy->detection_range) {
                // Mantém distância do jogador
                if (distance < enemy->attack_range * 0.7f) {
                    // Recua se o jogador chegar muito perto
                    float move_x = (-dx / distance) * enemy->speed * delta_time;
                    enemy_move(enemy, move_x, 0);
                    enemy->current_animation = &enemy->animations[ENEMY_ANIM_WALK];
                } 
                else if (distance > enemy->attack_range * 1.2f) {
                    // Aproxima se estiver muito longe
                    float move_x = (dx / distance) * enemy->speed * delta_time;
                    enemy_move(enemy, move_x, 0);
                    enemy->current_animation = &enemy->animations[ENEMY_ANIM_WALK];
                }
                else if (enemy->current_cooldown <= 0) {
                    // Ataque ranged
                    enemy->current_animation = &enemy->animations[ENEMY_ANIM_ATTACK];
                    // Aqui você chamaria a função para criar um novo projétil
                    // spawn_enemy_projectile(projectile_system, enemy);
                    enemy->current_cooldown = enemy->attack_cooldown;
                }
            } else {
                enemy->current_animation = &enemy->animations[ENEMY_ANIM_IDLE];
            }
            break;

        case ENEMY_BOSS:
            // Comportamento especial será implementado em update_boss()
            break;
    }
}

void enemy_move(struct Enemy *enemy, float dx, float dy) {
    enemy->entity.x += dx;
    enemy->entity.y += dy;
}

void enemy_attack(struct Enemy *enemy, struct Player *player) {
    enemy->is_attacking = true;
    enemy->current_animation = &enemy->animations[ENEMY_ANIM_ATTACK];
    enemy->current_cooldown = enemy->attack_cooldown;
    
    // Verifica se o jogador está dentro do range de ataque
    float dx = player->entity.x - enemy->entity.x;
    float distance = fabs(dx);
    
    if (distance <= enemy->attack_range) {
        damage_player(player, enemy->damage);
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

void damage_enemy(struct Enemy *enemy, int amount) {
    if (enemy->is_dead) return;
    
    enemy->health -= amount;
    enemy->current_animation = &enemy->animations[ENEMY_ANIM_HURT];
    
    if (enemy->health <= 0) {
        enemy->health = 0;
        kill_enemy(enemy);
    }
}

bool is_enemy_dead(struct Enemy *enemy) {
    return enemy->is_dead || enemy->health <= 0;
}

void kill_enemy(struct Enemy *enemy) {
    enemy->is_dead = true;
    enemy->current_animation = &enemy->animations[ENEMY_ANIM_DEATH];
    enemy->current_animation->current_frame = 0;
    enemy->current_animation->elapsed_time = 0;
}

// Renderização

void draw_enemy(struct Enemy *enemy) {
    if (!enemy || !enemy->is_active || !enemy->current_animation || 
        enemy->current_animation->current_frame >= enemy->current_animation->frame_count) {
        return; 
    }

    ALLEGRO_BITMAP *frame = enemy->current_animation->frames[enemy->current_animation->current_frame];
    if (!frame) {
        return;
    }
    
    // --- LÓGICA DE ESCALA E POSICIONAMENTO COM PONTO DE ÂNCORA ---

    // 1. Pega as dimensões do sprite e aplica a escala
    float sprite_w = al_get_bitmap_width(frame);
    float sprite_h = al_get_bitmap_height(frame);
    float scaled_w = sprite_w * ENEMY_SCALE;
    float scaled_h = sprite_h * ENEMY_SCALE;
    
    // 2. O ponto de âncora do inimigo (entity.x, entity.y) é o centro da base dele.
    //    Calculamos o canto superior esquerdo (draw_x, draw_y) para desenhar o sprite.
    float draw_x = enemy->entity.x - (scaled_w / 2);
    float draw_y = enemy->entity.y - scaled_h;
    
    // 3. Define se o sprite deve ser virado horizontalmente
    int flags = enemy->facing_right ? 0 : ALLEGRO_FLIP_HORIZONTAL;
    
    // 4. Desenha o sprite com a escala e posição corretas
    al_draw_scaled_bitmap(frame,
                        0, 0,                     // Origem x, y no bitmap
                        sprite_w, sprite_h,       // Largura e altura da origem
                        draw_x, draw_y,           // Posição x, y na tela
                        scaled_w,                 // Largura final na tela
                        scaled_h,                 // Altura final na tela
                        flags);
    
    // Desenho da hitbox e vida (o seu já estava bom, mas precisa da hitbox atualizada)
    if (enemy->hitbox_show) {
        al_draw_rectangle(
            enemy->entity.hitbox.x, 
            enemy->entity.hitbox.y,
            enemy->entity.hitbox.x + enemy->entity.hitbox.width,
            enemy->entity.hitbox.y + enemy->entity.hitbox.height,
            al_map_rgb(255, 0, 0), 2);
    }
    
    draw_enemy_health_bar(enemy);
}

void draw_enemy_health_bar(struct Enemy *enemy) {
    if (enemy->health <= 0) return;
    
    float bar_width = 50.0f;
    float bar_height = 5.0f;
    float health_percent = (float)enemy->health / (float)enemy->max_health;
    
    // Posição acima do inimigo
    float bar_x = enemy->entity.x - bar_width/2;
    float bar_y = enemy->entity.y - enemy->entity.height - 10;
    
    // Fundo da barra
    al_draw_filled_rectangle(bar_x, bar_y, 
                            bar_x + bar_width, bar_y + bar_height,
                            al_map_rgb(50, 50, 50));
    
    // Vida atual
    al_draw_filled_rectangle(bar_x, bar_y, 
                            bar_x + bar_width * health_percent, bar_y + bar_height,
                            health_percent > 0.6f ? al_map_rgb(0, 255, 0) :
                            health_percent > 0.3f ? al_map_rgb(255, 255, 0) :
                            al_map_rgb(255, 0, 0));
}

void draw_enemies(struct EnemySystem *system, struct GameLevel *level, struct Player *player) {
    // Desenha todos os inimigos normais
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (system->enemies[i].is_active) {
            // Sincroniza a flag de exibição da hitbox do inimigo com a do player
            system->enemies[i].hitbox_show = player->hitbox_show;

            // Converte a posição de mundo para posição de tela para desenhar
            float original_x = system->enemies[i].entity.x;
            system->enemies[i].entity.x -= level->scroll_x;
            draw_enemy(&system->enemies[i]);
            system->enemies[i].entity.x = original_x; // Restaura a posição de mundo
        }
    }

    // Desenha o boss se estiver ativo
    if (system->boss.is_active) {
        // Sincroniza a flag do boss também
        system->boss.hitbox_show = player->hitbox_show;
        
        float original_x = system->boss.entity.x;
        system->boss.entity.x -= level->scroll_x;
        draw_enemy(&system->boss);
        system->boss.entity.x = original_x;
    }
}

// Chefe

void init_boss(struct Enemy *boss) {
    // PRIMEIRO: Limpa a estrutura para evitar lixo de memória.
    memset(boss, 0, sizeof(struct Enemy));

    // SEGUNDO: Configura os atributos específicos do Boss
    boss->type = ENEMY_BOSS;
    boss->is_active = true;
    boss->facing_right = false;
    boss->ground_level = GROUND_LEVEL;

    boss->entity.width = 80;  // Exemplo: Boss é maior
    boss->entity.height = 150;
    boss->entity.vel_x = 0;
    boss->entity.vel_y = 0;

    boss->health = 500;
    boss->max_health = 500;
    boss->damage = 40;
    boss->speed = 50.0f;
    boss->attack_range = 100.0f;
    boss->detection_range = 1000.0f;
    boss->attack_cooldown = 1.0f;

    // Configuração da hitbox do Boss
    boss->entity.hitbox.width = boss->entity.width * 0.8f;
    boss->entity.hitbox.height = boss->entity.height * 0.9f;
    boss->entity.hitbox.offset_x = 0;
    boss->entity.hitbox.offset_y = 0;

    // TERCEIRO: Carrega os sprites do Boss
    load_enemy_sprites(boss, ENEMY_BOSS);
    
    // Configura a animação inicial
    boss->current_animation = &boss->animations[ENEMY_ANIM_IDLE];
}

void update_boss(struct Enemy *boss, struct Player *player, float delta_time) {}
void boss_attack_pattern(struct Enemy *boss, struct Player *player, struct Projectile *projectiles[], int *projectile_count) {}