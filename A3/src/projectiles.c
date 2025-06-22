#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "projectiles.h"
#include "utils.h"
#include "enemy.h"
#include "player.h"
#include "boss.h" // Incluído para usar as constantes do chefe

// Prepara o sistema de projéteis e carrega as sprites das balas
void init_projectile_system(struct ProjectileSystem *system) {
    // Zera o array de projéteis para garantir que todos comecem como inativos
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        system->projectiles[i].is_active = false;
        system->projectiles[i].sprite = NULL;
    }
    system->active_count = 0;

    // Carrega as sprites que serão usadas para os projéteis
    system->player_bullet_sprite = al_load_bitmap("assets/bullets/Bullet_5.56.png");
    if (!system->player_bullet_sprite) {
        fprintf(stderr, "Falha ao carregar a sprite 'assets/bullets/Bullet_5.56.png'\n");
    }

    system->enemy_bullet_sprite = al_load_bitmap("assets/bullets/Bullet_.45ACP.png");
    if (!system->enemy_bullet_sprite) {
        fprintf(stderr, "Falha ao carregar a sprite 'assets/bullets/Bullet_.45ACP.png'\n");
    }
}

// Atualiza a posição, tempo de vida e colisões de todos os projéteis ativos
void update_projectile_system(struct ProjectileSystem *system, float delta_time, 
                            struct Player *player, struct EnemySystem *enemy_system, struct GameLevel *level) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (system->projectiles[i].is_active) {
            // Move o projétil com base em sua velocidade
            system->projectiles[i].entity.x += system->projectiles[i].entity.vel_x * delta_time;
            
            // Reduz o tempo de vida do projétil e o desativa se o tempo acabar
            system->projectiles[i].lifetime -= delta_time;
            if (system->projectiles[i].lifetime <= 0) {
                system->projectiles[i].is_active = false;
                system->active_count--;
                continue; // Pula para o próximo projétil do loop
            }
            
            // Atualiza a posição da hitbox para seguir a entidade
            update_hitbox_position(&system->projectiles[i].entity, system->projectiles[i].entity.vel_x > 0);
        }
    }
    
    // Após mover todos os projéteis, verifica se colidiram com algo
    check_projectile_collisions(system, player, enemy_system, level);
}

// Desenha todos os projéteis ativos na tela
void draw_projectiles(struct ProjectileSystem *system, struct GameLevel *level, struct Player *player) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (system->projectiles[i].is_active) {
            struct Projectile *p = &system->projectiles[i];

            // Converte a posição de mundo do projétil para uma posição na tela
            float anchor_screen_x = p->entity.x - level->scroll_x;
            float anchor_screen_y = p->entity.y;

            // Calcula o canto superior esquerdo para o desenho a partir da âncora (centro-base)
            float draw_x = anchor_screen_x - (p->entity.width / 2);
            float draw_y = anchor_screen_y - p->entity.height;
            
            if (p->sprite) {
                float sw = al_get_bitmap_width(p->sprite);
                float sh = al_get_bitmap_height(p->sprite);
                int flags = (p->entity.vel_x > 0) ? 0 : ALLEGRO_FLIP_HORIZONTAL;
                
                // Desenha a sprite usando seu tamanho de entidade, não o da hitbox
                al_draw_scaled_bitmap(p->sprite, 
                                     0, 0, sw, sh,
                                     draw_x, draw_y,
                                     p->entity.width,
                                     p->entity.height,
                                     flags);
            } else {
                // Desenha uma forma geométrica simples caso a sprite não tenha sido carregada
                al_draw_filled_rectangle(draw_x, draw_y, draw_x + p->entity.width, draw_y + p->entity.height, al_map_rgb(255,0,255));
            }

            // Opcionalmente desenha a hitbox para fins de debug
            if (player->hitbox_show) {
                al_draw_rectangle(
                    p->entity.hitbox.x - level->scroll_x, // Converte x da hitbox para a tela
                    p->entity.hitbox.y,
                    p->entity.hitbox.x + p->entity.hitbox.width - level->scroll_x,
                    p->entity.hitbox.y + p->entity.hitbox.height,
                    al_map_rgb(0, 255, 0), 1.0f);
            }
        }
    }
}

// Cria uma nova instância de projétil no primeiro slot disponível
void spawn_projectile(struct ProjectileSystem *system, float x, float y, 
                     bool facing_right, ProjectileType type, 
                     ProjectileBehavior behavior, int damage) {

    if (system->active_count >= MAX_PROJECTILES) return; // Impede a criação se o limite foi atingido
    
    // Procura por um projétil inativo no array para reutilizá-lo
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!system->projectiles[i].is_active) {
            struct Projectile *p = &system->projectiles[i];
            
            // Define os atributos básicos do projétil
            p->entity.x = x;
            p->entity.y = y;
            p->entity.vel_x = facing_right ? PROJECTILE_SPEED : -PROJECTILE_SPEED;
            p->entity.vel_y = 0;
            p->type = type;
            p->behavior = behavior;
            p->is_active = true;
            p->damage = damage;
            p->lifetime = PROJECTILE_LIFETIME;

            p->sprite = (type == PROJECTILE_PLAYER) ? system->player_bullet_sprite : system->enemy_bullet_sprite;
            
            // Define o tamanho da sprite (visual) com base na escala
            if (p->sprite) {
                float scale = (type == PROJECTILE_PLAYER) ? PLAYER_BULLET_SCALE : ENEMY_BULLET_SCALE;
                p->entity.width = al_get_bitmap_width(p->sprite) * scale;
                p->entity.height = al_get_bitmap_height(p->sprite) * scale;
            } else { 
                p->entity.width = 10.0f;
                p->entity.height = 5.0f;
            }
            
            // Define o tamanho da hitbox (colisão) com base nas constantes
            if (type == PROJECTILE_PLAYER) {
                p->entity.hitbox.width = PLAYER_PROJECTILE_HITBOX_W;
                p->entity.hitbox.height = PLAYER_PROJECTILE_HITBOX_H;
            } else { // Inimigo ou Chefe
                p->entity.hitbox.width = ENEMY_PROJECTILE_HITBOX_W;
                p->entity.hitbox.height = ENEMY_PROJECTILE_HITBOX_H;
            }
            
            // Define os deslocamentos (offsets) da hitbox
            p->entity.hitbox.offset_x = PROJECTILE_HITBOX_OFFSET_X;
            p->entity.hitbox.offset_y = PROJECTILE_HITBOX_OFFSET_Y;
            
            update_hitbox_position(&p->entity, facing_right);
            
            system->active_count++;
            break; // Sai do loop após encontrar um slot e criar o projétil
        }
    }
}

// Cria um projétil originado do jogador
void spawn_player_projectile(struct ProjectileSystem *system, struct Player *player, struct GameLevel *level) {
    // Converte a posição do jogador de coordenadas da tela para coordenadas do mundo
    float player_world_x = player->entity.x + level->scroll_x;

    // Calcula a posição da "boca da arma" usando as constantes de offset do jogador
    float muzzle_offset_x = player->facing_right ? PLAYER_MUZZLE_OFFSET_X : -PLAYER_MUZZLE_OFFSET_X;
    float muzzle_offset_y = player->is_crouching ? PLAYER_MUZZLE_OFFSET_Y_CROUCH : PLAYER_MUZZLE_OFFSET_Y_STANDING;
    float muzzle_x = player_world_x + muzzle_offset_x;
    float muzzle_y = player->entity.y - muzzle_offset_y; 

    spawn_projectile(system, muzzle_x, muzzle_y, player->facing_right, PROJECTILE_PLAYER, PROJECTILE_NORMAL, 25);
}

// Cria um projétil originado de um inimigo comum
void spawn_enemy_projectile(struct ProjectileSystem *system, struct Enemy *enemy) {
    // Calcula a posição da "boca da arma" usando as constantes de offset do inimigo
    float muzzle_offset_x = enemy->facing_right ? ENEMY_MUZZLE_OFFSET_X : -ENEMY_MUZZLE_OFFSET_X;
    float muzzle_x = enemy->entity.x + muzzle_offset_x;
    float muzzle_y = enemy->entity.y - ENEMY_MUZZLE_OFFSET_Y;
    
    spawn_projectile(system, muzzle_x, muzzle_y, enemy->facing_right, PROJECTILE_ENEMY, PROJECTILE_NORMAL, enemy->damage);
}

// Cria um projétil originado do chefe
void spawn_boss_projectile(struct ProjectileSystem *system, struct Boss *boss) {
    // Calcula a posição da "boca da arma" usando as constantes de offset do chefe
    float muzzle_offset_x = boss->facing_right ? BOSS_MUZZLE_OFFSET_X : -BOSS_MUZZLE_OFFSET_X;
    float muzzle_x = boss->entity.x + muzzle_offset_x;
    float muzzle_y = boss->entity.y - BOSS_PROJECTILE_OFFSET_Y; 

    spawn_projectile(system, muzzle_x, muzzle_y, boss->facing_right, PROJECTILE_ENEMY, PROJECTILE_NORMAL, boss->projectile_damage);
}

// Verifica a colisão de todos os projéteis contra o jogador e os inimigos
void check_projectile_collisions(struct ProjectileSystem *system, struct Player *player, struct EnemySystem *enemy_system, struct GameLevel *level) {
    // Cria uma entidade temporária para o jogador com coordenadas de MUNDO para a checagem de colisão
    struct Entity player_world_entity = player->entity;
    player_world_entity.x = player->entity.x + level->scroll_x;
    update_hitbox_position(&player_world_entity, player->facing_right);

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!system->projectiles[i].is_active) continue;
        
        struct Projectile *p = &system->projectiles[i];
        
        // --- Colisão de projéteis de INIMIGOS contra o JOGADOR ---
        if (p->type == PROJECTILE_ENEMY && !is_player_dead(player)) {
            // Compara o projétil (em coordenadas de mundo) com a entidade temporária do jogador (também em mundo)
            if (check_collision(&p->entity, &player_world_entity)) {
                damage_player(player, p->damage);
                p->is_active = false;
                system->active_count--;
                continue;
            }
        }
        
        // --- Colisão de projéteis do JOGADOR contra INIMIGOS ---
        if (p->type == PROJECTILE_PLAYER) {
            bool hit_someone = false;

            // Loop para inimigos normais
            for (int j = 0; j < MAX_ENEMIES; j++) {
                struct Enemy *current_enemy = &enemy_system->enemies[j];
                if (current_enemy->is_active && !is_enemy_dead(current_enemy)) {
                    if (check_collision(&p->entity, &current_enemy->entity)) {
                        damage_enemy(current_enemy, p->damage, player);
                        hit_someone = true;
                        // Se o projétil for normal (não perfurante), ele para no primeiro inimigo que atingir
                        if (p->behavior == PROJECTILE_NORMAL) break;
                    }
                }
            }
            
            // Se o projétil atingiu alguém e não é perfurante, ele é desativado
            if (hit_someone && p->behavior == PROJECTILE_NORMAL) {
                p->is_active = false;
                system->active_count--;
                continue;
            }

            // Colisão com o chefe (só acontece se o projétil ainda estiver ativo)
            if (enemy_system->boss.is_active && !is_boss_dead(&enemy_system->boss)) {
                 if (check_collision(&p->entity, &enemy_system->boss.entity)) {
                    damage_boss(&enemy_system->boss, p->damage, player);
                    hit_someone = true;
                }
            }

            // Desativa o projétil se ele atingiu o chefe (e não for perfurante)
            if (hit_someone && p->behavior == PROJECTILE_NORMAL) {
                p->is_active = false;
                system->active_count--;
            }
        }
    }
}

// Função auxiliar que verifica a colisão entre um projétil e um alvo
bool check_projectile_hit(struct Projectile *projectile, struct Entity *target) {
    return check_collision(&projectile->entity, target);
}

// Desativa todos os projéteis, limpando a tela
void clear_projectiles(struct ProjectileSystem *system) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        system->projectiles[i].is_active = false;
    }
    system->active_count = 0;
}

// Libera os recursos do sistema, como as sprites das balas
void destroy_projectile_system(struct ProjectileSystem *system) {
    clear_projectiles(system);
    if (system->player_bullet_sprite) {
        al_destroy_bitmap(system->player_bullet_sprite);
        system->player_bullet_sprite = NULL;
    }
    if (system->enemy_bullet_sprite) {
        al_destroy_bitmap(system->enemy_bullet_sprite);
        system->enemy_bullet_sprite = NULL;
    }
}