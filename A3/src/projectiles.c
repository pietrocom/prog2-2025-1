#include <stdio.h>
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

    system->player_bullet_sprite = al_load_bitmap("assets/bullets/Bullet_5.56.png");
    if (!system->player_bullet_sprite) {
        fprintf(stderr, "Falha ao carregar a sprite 'assets/bullets/Bullet_5.56.png'\n");
    }

    system->enemy_bullet_sprite = al_load_bitmap("assets/bullets/Bullet_.45ACP.png");
    if (!system->enemy_bullet_sprite) {
        fprintf(stderr, "Falha ao carregar a sprite 'assets/bullets/Bullet_.45ACP.png'\n");
    }
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
void draw_projectiles(struct ProjectileSystem *system, struct GameLevel *level, struct Player *player) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (system->projectiles[i].is_active) {
            
            struct Projectile *p = &system->projectiles[i];
            float draw_x = p->entity.x - level->scroll_x;
            float draw_y = p->entity.y;

            if (p->sprite) {
                float sw = al_get_bitmap_width(p->sprite);
                float sh = al_get_bitmap_height(p->sprite);

                int flags = (p->entity.vel_x < 0) ? ALLEGRO_FLIP_HORIZONTAL : 0;
                
                al_draw_scaled_bitmap(p->sprite, 
                                     0, 0, sw, sh,     // Região de origem da sprite
                                     draw_x, draw_y,   // Posição na tela
                                     p->entity.hitbox.width, p->entity.hitbox.height, // Tamanho final na tela
                                     flags);           // Usa a flag calculada aqui
            } else {
                // Desenha primitiva 
                ALLEGRO_COLOR color = p->color;
                if (p->type == PROJECTILE_PLAYER) {
                    al_draw_filled_rectangle(
                        draw_x,
                        draw_y,
                        draw_x + PROJECTILE_WIDTH,
                        draw_y + PROJECTILE_HEIGHT,
                        color
                    );
                } else { // Inimigo
                    al_draw_filled_circle(
                        draw_x + PROJECTILE_WIDTH/2,
                        draw_y + PROJECTILE_HEIGHT/2,
                        PROJECTILE_WIDTH/2,
                        color
                    );
                }
            }

            if (player->hitbox_show) {
                al_draw_rectangle(
                    p->entity.hitbox.x - level->scroll_x,
                    p->entity.hitbox.y,
                    p->entity.hitbox.x + p->entity.hitbox.width - level->scroll_x,
                    p->entity.hitbox.y + p->entity.hitbox.height,
                    al_map_rgb(255, 0, 0), 1.0f);
            }
        }
    }
}

// Cria um novo projétil
void spawn_projectile(struct ProjectileSystem *system, float x, float y, 
                     bool facing_right, ProjectileType type, 
                     ProjectileBehavior behavior, int damage) {

    if (system->active_count >= MAX_PROJECTILES) {
        return;
    }
    
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!system->projectiles[i].is_active) {
            struct Projectile *p = &system->projectiles[i];
            
            p->entity.x = x;
            p->entity.y = y;
            p->entity.vel_x = facing_right ? PROJECTILE_SPEED : -PROJECTILE_SPEED;
            p->entity.vel_y = 0;
            
            p->type = type;
            p->behavior = behavior;
            p->is_active = true;
            p->damage = damage;
            p->lifetime = PROJECTILE_LIFETIME;

            if (type == PROJECTILE_PLAYER) {
                p->sprite = system->player_bullet_sprite;
            } else {
                p->sprite = system->enemy_bullet_sprite;
            }

            if (p->sprite) {
                float scale = 1.0f; // Escala padrão caso não seja definida
                if (p->type == PROJECTILE_PLAYER) {
                    scale = PLAYER_BULLET_SCALE;
                } else { 
                    scale = ENEMY_BULLET_SCALE;
                }
                
                // Calcula o tamanho da entidade/hitbox usando a escala correta
                p->entity.width = al_get_bitmap_width(p->sprite) * scale;
                p->entity.height = al_get_bitmap_height(p->sprite) * scale;
            } else { 
                p->entity.width = PROJECTILE_WIDTH;
                p->entity.height = PROJECTILE_HEIGHT;
            }
            
            p->entity.hitbox.width = p->entity.width;
            p->entity.hitbox.height = p->entity.height;
            p->entity.hitbox.offset_x = 0;
            p->entity.hitbox.offset_y = 0;
            
            update_hitbox_position(&p->entity, facing_right);
            
            system->active_count++;
            break;
        }
    }
}

// Cria projétil do jogador
void spawn_player_projectile(struct ProjectileSystem *system, struct Player *player, struct GameLevel *level) {
    float player_world_x = player->entity.x;
    if (level) { // Se o level for passado, considera o scroll
        player_world_x += level->scroll_x;
    }

    float offset_x = player->facing_right ? (player->entity.width / 2) : (-player->entity.width / 2);
    
    float vertical_offset = player->is_crouching ? CROUCH_PROJECTILE_OFFSET_Y : STANDING_PROJECTILE_OFFSET_Y;
    float spawn_y = player->entity.y - vertical_offset;

    spawn_projectile(
        system,
        player_world_x + offset_x,
        spawn_y, 
        player->facing_right,
        PROJECTILE_PLAYER,
        PROJECTILE_NORMAL,
        25 
    );
}

// Cria projétil do inimigo
void spawn_enemy_projectile(struct ProjectileSystem *system, struct Enemy *enemy) {
    float offset_x = enemy->facing_right ? (enemy->entity.width / 2) : (-enemy->entity.width / 2);
    
    float spawn_y = enemy->entity.y - ENEMY_PROJECTILE_OFFSET_Y;
    
    spawn_projectile(
        system,
        enemy->entity.x + offset_x,
        spawn_y,
        enemy->facing_right,
        PROJECTILE_ENEMY,
        PROJECTILE_NORMAL,
        enemy->damage
    );
}

void spawn_boss_projectile(struct ProjectileSystem *system, struct Boss *boss) {
    float offset_x = boss->facing_right ? (boss->entity.width / 2) : (-boss->entity.width / 2);

    float spawn_y = boss->entity.y - BOSS_PROJECTILE_OFFSET_Y;

    spawn_projectile(
        system,
        boss->entity.x + offset_x,
        spawn_y,
        boss->facing_right,
        PROJECTILE_ENEMY, // Ainda é um projétil de inimigo
        PROJECTILE_NORMAL,
        boss->projectile_damage
    );
}

void check_projectile_collisions(struct ProjectileSystem *system, 
                               struct Player *player, 
                               struct EnemySystem *enemy_system) {

    struct Entity player_world_entity = player->entity;
    player_world_entity.hitbox.x += player->entity.x - player->entity.hitbox.x; // Centraliza a hitbox no mundo
    player_world_entity.x += player->entity.x; // Posição X no mundo
    update_hitbox_position(&player_world_entity, player->facing_right); // Atualiza a hitbox na posição do mundo
    player_world_entity.hitbox.x = player->entity.hitbox.x + player->entity.x;


    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!system->projectiles[i].is_active) continue;
        
        struct Projectile *p = &system->projectiles[i];
        
        // Usa a hitbox do jogador no espaço do mundo para a verificação
        if (p->type == PROJECTILE_ENEMY && !is_player_dead(player) && 
            check_collision(&p->entity, &player_world_entity)) {
            
            damage_player(player, p->damage);
            p->is_active = false;
            system->active_count--;
            continue;
        }
        
        // Colisão com inimigos (apenas projéteis do jogador)
        if (p->type == PROJECTILE_PLAYER) {
            bool hit_someone = false;

            // Loop para inimigos normais
            for (int j = 0; j < MAX_ENEMIES; j++) {
                if (enemy_system->enemies[j].is_active && !is_enemy_dead(&enemy_system->enemies[j]) &&
                    check_collision(&p->entity, &enemy_system->enemies[j].entity)) {
                    
                    damage_enemy(&enemy_system->enemies[j], p->damage, player);
                    hit_someone = true;
                }
            }
            
            if (enemy_system->boss.is_active && !is_boss_dead(&enemy_system->boss) &&
                check_collision(&p->entity, &enemy_system->boss.entity)) {
                
                damage_boss(&enemy_system->boss, p->damage, player);
                hit_someone = true;
            }

            // Se o projétil atingiu alguém e for do tipo normal, ele é destruído.
            if (hit_someone && p->behavior == PROJECTILE_NORMAL) {
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
