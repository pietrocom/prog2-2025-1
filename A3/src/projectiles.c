#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "projectiles.h"
#include "utils.h"
#include "enemy.h"
#include "player.h"

// Inicializa o sistema de projéteis
void init_projectile_system(struct ProjectileSystem *system) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        system->projectiles[i].is_active = false;
        system->projectiles[i].sprite = NULL;
        system->projectiles[i].color = al_map_rgb(255, 255, 255); // Cor padrão branca
    }
    system->active_count = 0;
}

// Atualiza todos os projéteis ativos
void update_projectile_system(struct ProjectileSystem *system, float delta_time, 
                            struct Player *player, struct EnemySystem *enemy_system) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (system->projectiles[i].is_active) {
            // Atualiza posição
            system->projectiles[i].entity.x += system->projectiles[i].entity.vel_x * delta_time;
            system->projectiles[i].entity.y += system->projectiles[i].entity.vel_y * delta_time;
            
            // Atualiza tempo de vida
            system->projectiles[i].lifetime -= delta_time;
            if (system->projectiles[i].lifetime <= 0) {
                system->projectiles[i].is_active = false;
                system->active_count--;
                continue;
            }
            
            // Atualiza hitbox
            update_hitbox_position(&system->projectiles[i].entity, 
                                 system->projectiles[i].entity.vel_x > 0);
        }
    }
    
    // Verifica colisões
    check_projectile_collisions(system, player, enemy_system);
}

// Desenha todos os projéteis ativos
void draw_projectiles(struct ProjectileSystem *system) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (system->projectiles[i].is_active) {
            if (system->projectiles[i].sprite) {
                // Desenha com sprite
                al_draw_bitmap(system->projectiles[i].sprite, 
                             system->projectiles[i].entity.x, 
                             system->projectiles[i].entity.y, 0);
            } else {
                // Desenha primitiva (retângulo ou círculo)
                ALLEGRO_COLOR color = system->projectiles[i].color;
                if (system->projectiles[i].type == PROJECTILE_PLAYER) {
                    al_draw_filled_rectangle(
                        system->projectiles[i].entity.x,
                        system->projectiles[i].entity.y,
                        system->projectiles[i].entity.x + PROJECTILE_WIDTH,
                        system->projectiles[i].entity.y + PROJECTILE_HEIGHT,
                        color
                    );
                } else { // Inimigo
                    al_draw_filled_circle(
                        system->projectiles[i].entity.x + PROJECTILE_WIDTH/2,
                        system->projectiles[i].entity.y + PROJECTILE_HEIGHT/2,
                        PROJECTILE_WIDTH/2,
                        color
                    );
                }
            }
        }
    }
}

// Cria um novo projétil
void spawn_projectile(struct ProjectileSystem *system, float x, float y, 
                     bool facing_right, ProjectileType type, 
                     ProjectileBehavior behavior, int damage) {
    // Encontra slot vazio
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!system->projectiles[i].is_active) {
            struct Projectile *p = &system->projectiles[i];
            
            // Configura entidade
            p->entity.x = x;
            p->entity.y = y;
            p->entity.width = PROJECTILE_WIDTH;
            p->entity.height = PROJECTILE_HEIGHT;
            p->entity.vel_x = facing_right ? PROJECTILE_SPEED : -PROJECTILE_SPEED;
            p->entity.vel_y = 0;
            
            // Configura hitbox
            p->entity.hitbox.width = PROJECTILE_WIDTH;
            p->entity.hitbox.height = PROJECTILE_HEIGHT;
            p->entity.hitbox.offset_x = 0;
            p->entity.hitbox.offset_y = 0;
            update_hitbox_position(&p->entity, facing_right);
            
            // Configura propriedades do projétil
            p->type = type;
            p->behavior = behavior;
            p->is_active = true;
            p->damage = damage;
            p->lifetime = 2.0f; // 2 segundos por padrão
            p->max_lifetime = 2.0f;
            
            // Configura aparência baseada no tipo
            if (type == PROJECTILE_PLAYER) {
                p->color = al_map_rgb(0, 255, 0); // Verde para jogador
            } else {
                p->color = al_map_rgb(255, 0, 0); // Vermelho para inimigos
            }
            
            system->active_count++;
            break;
        }
    }
}

// Cria projétil do jogador
void spawn_player_projectile(struct ProjectileSystem *system, struct Player *player) {
    float offset_x = player->facing_right ? 
        player->entity.width - 10 : 
        -PROJECTILE_WIDTH;
    
    spawn_projectile(
        system,
        player->entity.x + offset_x,
        player->entity.y - player->entity.height/2,
        player->facing_right,
        PROJECTILE_PLAYER,
        PROJECTILE_NORMAL,
        10 // Dano base do jogador
    );
}

// Cria projétil do inimigo
void spawn_enemy_projectile(struct ProjectileSystem *system, struct Enemy *enemy) {
    float offset_x = enemy->facing_right ? 
        enemy->entity.width - 10 : 
        -PROJECTILE_WIDTH;
    
    spawn_projectile(
        system,
        enemy->entity.x + offset_x,
        enemy->entity.y - enemy->entity.height/2,
        enemy->facing_right,
        PROJECTILE_ENEMY,
        PROJECTILE_NORMAL,
        enemy->damage
    );
}

// Verifica colisões com jogador e inimigos
void check_projectile_collisions(struct ProjectileSystem *system, 
                               struct Player *player, 
                               struct EnemySystem *enemy_system) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!system->projectiles[i].is_active) continue;
        
        struct Projectile *p = &system->projectiles[i];
        
        // Colisão com jogador (apenas projéteis inimigos)
        if (p->type == PROJECTILE_ENEMY && 
            check_projectile_hit(p, &player->entity)) {
            damage_player(player, p->damage);
            p->is_active = false;
            system->active_count--;
            continue;
        }
        
        // Colisão com inimigos (apenas projéteis do jogador)
        if (p->type == PROJECTILE_PLAYER) {
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (enemy_system->enemies[j].is_active && 
                    !enemy_system->enemies[j].is_dead &&
                    check_projectile_hit(p, &enemy_system->enemies[j].entity)) {
                    damage_enemy(&enemy_system->enemies[j], p->damage);
                    p->is_active = false;
                    system->active_count--;
                    break;
                }
            }
            
            // Verifica colisão com o boss
            if (enemy_system->boss.is_active && 
                check_projectile_hit(p, &enemy_system->boss.entity)) {
                damage_enemy(&enemy_system->boss, p->damage);
                p->is_active = false;
                system->active_count--;
            }
        }
    }
}

// Verifica colisão entre projétil e alvo
bool check_projectile_hit(struct Projectile *projectile, struct Entity *target) {
    return check_collision(&projectile->entity, target);
}

// Remove todos os projéteis ativos
void clear_projectiles(struct ProjectileSystem *system) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        system->projectiles[i].is_active = false;
    }
    system->active_count = 0;
}

// Libera recursos do sistema de projéteis
void destroy_projectile_system(struct ProjectileSystem *system) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (system->projectiles[i].sprite) {
            al_destroy_bitmap(system->projectiles[i].sprite);
        }
    }
    clear_projectiles(system);
}