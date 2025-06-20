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
    memset(system->enemies, 0, sizeof(system->enemies));
    system->active_count = 0;
    system->wave_number = 0; // Inicia em 0. A primeira leva de spawns será a wave 1.
    
    memset(&system->boss, 0, sizeof(struct Enemy));
    system->boss.is_active = false;
    
    srand(time(NULL));
}

void update_enemy_system(struct EnemySystem *system, struct Player *player, struct GameLevel *level, float delta_time) {
    
    // 1. ATUALIZAÇÃO DOS INIMIGOS E BOSS ATIVOS
    // Primeiro, atualiza o estado de todos os inimigos que já estão na tela.
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (system->enemies[i].is_active) {
            update_enemy(&system->enemies[i], player, level, delta_time);
        }
    }

    // Atualiza o boss se ele estiver ativo
    if (system->boss.is_active) {
        update_boss(&system->boss, player, delta_time);
        // Desativa o boss após sua animação de morte terminar
        if (is_enemy_dead(&system->boss) &&
            system->boss.current_animation == &system->boss.animations[ENEMY_ANIM_DEATH] &&
            system->boss.current_animation->current_frame >= system->boss.current_animation->frame_count - 1) {
                system->boss.is_active = false; 
                // Aqui você poderia adicionar uma lógica de vitória do jogo
        }
    }
    
    // 2. RECONTAGEM DE INIMIGOS ATIVOS
    // Conta quantos inimigos realmente estão ativos. Esta contagem só diminui
    // depois que a animação de morte do inimigo termina, garantindo que uma
    // wave só seja considerada "eliminada" quando todos os corpos sumiram.
    int current_active_enemies = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (system->enemies[i].is_active) {
            current_active_enemies++;
        }
    }
    system->active_count = current_active_enemies;

    // 3. LÓGICA DE SPAWN DE WAVES
    // Se não há inimigos na tela e o boss ainda não apareceu...
    if (system->active_count == 0 && !system->boss.is_active) {
        
        // CONDIÇÃO INICIAL: Se for a primeira vez, spawna as duas primeiras waves
        if (system->wave_number == 0) {
            printf("Iniciando o jogo: Spawning waves 1 e 2.\n");
            spawn_enemy_wave(system, level); // Spawna a Wave 1
            spawn_enemy_wave(system, level); // Spawna a Wave 2
        } 
        // CONDIÇÕES NORMAIS: Para as waves seguintes
        else if (system->wave_number > 0) {
            
            // CONDIÇÃO DO BOSS: Verifica se a 5ª wave foi eliminada
            if (system->wave_number >= 5) {
                printf("Wave 5 eliminada. O CHEFE ESTA CHEGANDO!\n");
                
                // Posição de spawn do boss
                float boss_x = level->scroll_x + al_get_display_width(al_get_current_display()) + 200;
                float boss_y = level->ground_level;
                
                // Inicializa e ativa o boss
                init_boss(&system->boss);
                system->boss.entity.x = boss_x;
                system->boss.entity.y = boss_y;
                system->boss.is_active = true;
                update_hitbox_position(&system->boss.entity, false);

                // Incrementa a wave para não entrar mais nesta lógica
                system->wave_number++; 
            } 
            // CONDIÇÃO DE WAVE NORMAL: Spawna a próxima wave regular
            else {
                printf("Wave %d eliminada. Spawning proxima wave.\n", system->wave_number);
                spawn_enemy_wave(system, level);
            }
        }
    }
}

void spawn_enemy_wave(struct EnemySystem *system, struct GameLevel *level) {
    system->wave_number++; // Incrementa o contador para a wave que está sendo criada

    // Define quantos inimigos criar (aumenta progressivamente com as waves)
    int enemies_to_spawn = 2 + (system->wave_number / 2); 
    if (enemies_to_spawn > 7) enemies_to_spawn = 7; // Limita o nro de inimigos por wave

    // Garante que não vamos ultrapassar o limite total de inimigos
    if (system->active_count + enemies_to_spawn > MAX_ENEMIES) {
        enemies_to_spawn = MAX_ENEMIES - system->active_count;
    }
    
    if (enemies_to_spawn <= 0) return;

    // --- Lógica de Spawn em Proximidade ---
    const float GAP_BETWEEN_ENEMIES = 40.0f; // Espaço em pixels entre cada inimigo

    // Posição de spawn base (para o primeiro inimigo), fora da tela à direita
    float base_spawn_x = level->scroll_x + al_get_display_width(al_get_current_display()) + (rand() % 150 + 100);
    float spawn_y = level->ground_level;

    int spawned_count = 0;
    for (int i = 0; i < MAX_ENEMIES && spawned_count < enemies_to_spawn; i++) {
        // Encontra um slot de inimigo vazio no array
        if (!system->enemies[i].is_active) {
            
            // Calcula a posição X deste inimigo, colocando-o ao lado do anterior
            float spawn_x = base_spawn_x + (spawned_count * (ENEMY_WIDTH + GAP_BETWEEN_ENEMIES));
            
            // Define o tipo de inimigo com base na wave atual
            EnemyType type;
            if (system->wave_number < 2) { // Wave 1 só tem inimigos melee
                type = ENEMY_MELEE; 
            } else { // Waves seguintes introduzem inimigos ranged
                type = (rand() % 3 == 0) ? ENEMY_RANGED : ENEMY_MELEE; // 33% de chance de ser ranged
            }
            
            // Inicializa o inimigo na posição calculada
            init_enemy(&system->enemies[i], type, spawn_x, spawn_y);
            system->enemies[i].facing_right = false; // Nascem virados para o jogador
            
            spawned_count++;
        }
    }
}

void destroy_enemy_system(struct EnemySystem *system) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (system->enemies[i].animations[ENEMY_ANIM_IDLE].frame_count > 0) {
             unload_enemy_sprites(&system->enemies[i]);
        }
    }
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
            enemy->entity.width = ENEMY_WIDTH;
            enemy->entity.height = ENEMY_HEIGHT;
            enemy->health = 150;
            enemy->max_health = 150;
            enemy->damage = 20;
            enemy->speed = 120.0f;
            enemy->attack_range = 60.0f;
            enemy->detection_range = DETECTION_RANGE;
            enemy->attack_cooldown = 1.2f;
            break;
            
        case ENEMY_RANGED:
            enemy->entity.width = ENEMY_WIDTH;
            enemy->entity.height = ENEMY_HEIGHT - 5;
            enemy->health = 100;
            enemy->max_health = 100;
            enemy->damage = 15;
            enemy->speed = 90.0f;
            enemy->attack_range = 450.0f;
            enemy->detection_range = DETECTION_RANGE;
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

void update_enemy(struct Enemy *enemy, struct Player *player, struct GameLevel *level, float delta_time) {
    if (!enemy->is_active) {
        return; 
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
        
        enemy_ai(enemy, player, level, delta_time); 
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

void enemy_ai(struct Enemy *enemy, struct Player *player, struct GameLevel *level, float delta_time) {
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
    
    // 1. Calcula a posição REAL do jogador no mundo do jogo
    float player_world_x = player->entity.x + level->scroll_x;

    // 2. Calcula a distância e a direção usando a coordenada de mundo do jogador
    float dx = player_world_x - enemy->entity.x;
    float distance = fabs(dx);

    enemy->facing_right = (dx > 0);

    // Comportamento baseado no tipo de inimigo
    switch(enemy->type) {
        case ENEMY_MELEE:
            if (distance < enemy->detection_range) {
                // Se estiver fora do alcance de ataque, APROXIME-SE
                if (distance > enemy->attack_range) {
                    float move_x = (dx > 0 ? 1 : -1) * enemy->speed * delta_time;
                    enemy_move(enemy, move_x, 0);
                    enemy->current_animation = &enemy->animations[ENEMY_ANIM_WALK];
                } 
                // Se estiver no alcance e puder atacar, ATAQUE
                else if (enemy->current_cooldown <= 0) {
                    enemy_attack(enemy, player);
                } 
                // Se estiver no alcance mas esperando o cooldown, fique IDLE
                else {
                    enemy->current_animation = &enemy->animations[ENEMY_ANIM_IDLE];
                }
            } else {
                // Se o jogador estiver muito longe, fique IDLE
                enemy->current_animation = &enemy->animations[ENEMY_ANIM_IDLE];
            }
            break;

        case ENEMY_RANGED:
             if (distance < enemy->detection_range) {
                // Se o jogador estiver perto demais, recue
                if (distance < enemy->attack_range * 0.7f) {
                    float move_x = (dx > 0 ? -1 : 1) * enemy->speed * delta_time;
                    enemy_move(enemy, move_x, 0);
                    enemy->current_animation = &enemy->animations[ENEMY_ANIM_WALK];
                } 
                // Se estiver muito longe para atirar, aproxime-se
                else if (distance > enemy->attack_range) { 
                    float move_x = (dx > 0 ? 1 : -1) * enemy->speed * delta_time;
                    enemy_move(enemy, move_x, 0);
                    enemy->current_animation = &enemy->animations[ENEMY_ANIM_WALK];
                }
                // Se estiver na distância ideal e puder atacar, ATAQUE
                else if (enemy->current_cooldown <= 0) {
                    enemy_attack(enemy, player);
                } 
                // Se na distância ideal mas esperando cooldown, IDLE
                else {
                    enemy->current_animation = &enemy->animations[ENEMY_ANIM_IDLE];
                }
            } else {
                enemy->current_animation = &enemy->animations[ENEMY_ANIM_IDLE];
            }
            break;

        case ENEMY_BOSS:
            // update_boss() cuida da IA do chefe
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
    enemy->current_animation->current_frame = 0; // Reinicia a animação de ataque
    enemy->current_animation->elapsed_time = 0;
    enemy->current_cooldown = enemy->attack_cooldown;
    
    // Para inimigos melee, o dano pode ser aplicado em um frame específico da animação.
    // Para simplificar por agora, aplicamos o dano se o jogador estiver no alcance no início do ataque.
    if (enemy->type == ENEMY_MELEE) {
        float distance = fabs(player->entity.x - enemy->entity.x);
        if (distance <= enemy->attack_range) {
            damage_player(player, enemy->damage);
        }
    }
    // Para inimigos RANGED, a lógica de spawnar projétil seria chamada aqui ou na IA
    // baseada em um frame da animação de ataque.
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
    
    // Se ainda estiver vivo após o dano, executa o hurt
    if (enemy->health > 0) {
        enemy->current_animation = &enemy->animations[ENEMY_ANIM_HURT];
        enemy->current_animation->current_frame = 0;
        enemy->current_animation->elapsed_time = 0;
    } else {
        // Se o dano for fatal, chama a função de morte, passando o jogador para dar os pontos
        kill_enemy(enemy, player);
    }
}

bool is_enemy_dead(struct Enemy *enemy) {
    return enemy->is_dead || enemy->health <= 0;
}

void kill_enemy(struct Enemy *enemy, struct Player *player) {
    if (enemy->is_dead) return;

    // --- LÓGICA DE PONTUAÇÃO ---
    switch(enemy->type) {
        case ENEMY_MELEE:
            player->score += 100;
            break;
        case ENEMY_RANGED:
            player->score += 150;
            break;
        case ENEMY_BOSS:
            player->score += 1000;
            break;
    }

    enemy->is_dead = true;
    enemy->health = 0; 
    enemy->entity.vel_x = 0; 
    
    enemy->current_animation = &enemy->animations[ENEMY_ANIM_DEATH];
    enemy->current_animation->current_frame = 0;
    enemy->current_animation->elapsed_time = 0;

    enemy->death_timer = DEATH_FADEOUT_DELAY;
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
    float final_offset_x = enemy->facing_right ? ENEMY_SPRITE_OFFSET_X : -ENEMY_SPRITE_OFFSET_X;
    float draw_x = enemy->entity.x - (scaled_w / 2) + final_offset_x;
    float draw_y = enemy->entity.y - scaled_h + ENEMY_SPRITE_OFFSET_Y;
    
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

            // Salva as posições ORIGINAIS (de mundo)
            float original_entity_x = system->enemies[i].entity.x;
            float original_hitbox_x = system->enemies[i].entity.hitbox.x; // NOVO: Salva a pos da hitbox

            // Converte a posição da entidade E da hitbox para coordenadas de TELA
            system->enemies[i].entity.x -= level->scroll_x;
            system->enemies[i].entity.hitbox.x -= level->scroll_x; // NOVO: Converte a pos da hitbox também

            // Desenha tudo (agora ambos estão na mesma coordenada de tela)
            draw_enemy(&system->enemies[i]);

            // Restaura as posições ORIGINAIS (de mundo) para a próxima atualização lógica
            system->enemies[i].entity.x = original_entity_x;
            system->enemies[i].entity.hitbox.x = original_hitbox_x; // NOVO: Restaura a pos da hitbox também
        }
    }

    // Desenha o boss se estiver ativo (aplicando a mesma lógica)
    if (system->boss.is_active) {
        system->boss.hitbox_show = player->hitbox_show;
        
        float original_entity_x = system->boss.entity.x;
        float original_hitbox_x = system->boss.entity.hitbox.x;

        system->boss.entity.x -= level->scroll_x;
        system->boss.entity.hitbox.x -= level->scroll_x;
        
        draw_enemy(&system->boss);
        
        system->boss.entity.x = original_entity_x;
        system->boss.entity.hitbox.x = original_hitbox_x;
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