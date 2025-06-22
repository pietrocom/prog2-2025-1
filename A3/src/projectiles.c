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
                            struct Player *player, struct EnemySystem *enemy_system, struct GameLevel *level) {
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
    check_projectile_collisions(system, player, enemy_system, level);
}

// Desenha todos os projéteis ativos
void draw_projectiles(struct ProjectileSystem *system, struct GameLevel *level, struct Player *player) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (system->projectiles[i].is_active) {
            
            struct Projectile *p = &system->projectiles[i];

            // --- LÓGICA DE DESENHO CORRIGIDA ---

            // 1. A âncora (p->entity.x, p->entity.y) está no espaço do MUNDO.
            //    Vamos convertê-la para o espaço da TELA.
            float anchor_screen_x = p->entity.x - level->scroll_x;
            float anchor_screen_y = p->entity.y;

            // 2. Com base na âncora (centro-base), calculamos o canto superior esquerdo
            //    onde a sprite deve ser desenhada.
            float draw_x = anchor_screen_x - (p->entity.width / 2);
            float draw_y = anchor_screen_y - p->entity.height;
            
            if (p->sprite) {
                float sw = al_get_bitmap_width(p->sprite);
                float sh = al_get_bitmap_height(p->sprite);
                int flags = (p->entity.vel_x > 0) ? 0 : ALLEGRO_FLIP_HORIZONTAL;
                
                // 3. CORREÇÃO: Desenha a sprite usando seu tamanho de entidade (p->entity.width/height),
                //    NÃO o tamanho da hitbox.
                al_draw_scaled_bitmap(p->sprite, 
                                     0, 0, sw, sh,
                                     draw_x, draw_y,
                                     p->entity.width,  // CORRIGIDO
                                     p->entity.height, // CORRIGIDO
                                     flags);
            } else {
                // Lógica de desenho com primitivas (se a sprite falhar)
                al_draw_filled_rectangle(draw_x, draw_y, draw_x + p->entity.width, draw_y + p->entity.height, al_map_rgb(255,0,255));
            }

            // O desenho da hitbox (para debug) também é ajustado para a tela
            if (player->hitbox_show) {
                al_draw_rectangle(
                    p->entity.hitbox.x - level->scroll_x, // Converte x da hitbox para a tela
                    p->entity.hitbox.y,
                    p->entity.hitbox.x + p->entity.hitbox.width - level->scroll_x, // Converte x final para a tela
                    p->entity.hitbox.y + p->entity.hitbox.height,
                    al_map_rgb(0, 255, 0), 1.0f);
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
            
            // O ponto de âncora (x,y) é o CENTRO DA BASE do projétil.
            p->entity.x = x;
            p->entity.y = y;
            p->entity.vel_x = facing_right ? PROJECTILE_SPEED : -PROJECTILE_SPEED;
            p->entity.vel_y = 0;
            
            p->type = type;
            p->behavior = behavior;
            p->is_active = true;
            p->damage = damage;
            p->lifetime = PROJECTILE_LIFETIME;

            // Define a sprite
            if (type == PROJECTILE_PLAYER) {
                p->sprite = system->player_bullet_sprite;
            } else {
                p->sprite = system->enemy_bullet_sprite;
            }
            
            // Define o TAMANHO DA SPRITE (para o desenho)
            if (p->sprite) {
                float scale = (type == PROJECTILE_PLAYER) ? PLAYER_BULLET_SCALE : ENEMY_BULLET_SCALE;
                p->entity.width = al_get_bitmap_width(p->sprite) * scale;
                p->entity.height = al_get_bitmap_height(p->sprite) * scale;
            } else { 
                // Fallback caso a sprite não carregue
                p->entity.width = 10.0f;
                p->entity.height = 5.0f;
            }
            
            // --- CONFIGURAÇÃO DA HITBOX ---
            // A hitbox agora tem seu próprio tamanho, definido pelas novas constantes.
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
            
            // A função update_hitbox_position agora funciona corretamente, pois
            // a âncora do projétil foi padronizada para centro-base.
            update_hitbox_position(&p->entity, facing_right);
            
            system->active_count++;
            break;
        }
    }
}

// Cria projétil do jogador
void spawn_player_projectile(struct ProjectileSystem *system, struct Player *player, struct GameLevel *level) {
    float player_world_x = player->entity.x + level->scroll_x;

    float muzzle_offset_x = player->facing_right ? PLAYER_MUZZLE_OFFSET_X : -PLAYER_MUZZLE_OFFSET_X;
    float muzzle_offset_y = player->is_crouching ? PLAYER_MUZZLE_OFFSET_Y_CROUCH : PLAYER_MUZZLE_OFFSET_Y_STANDING;

    float muzzle_x = player_world_x + muzzle_offset_x;
    float muzzle_y = player->entity.y - muzzle_offset_y; 

    spawn_projectile(
        system,
        muzzle_x,
        muzzle_y, 
        player->facing_right,
        PROJECTILE_PLAYER,
        PROJECTILE_NORMAL,
        25 
    );
}

// Cria projétil do inimigo
void spawn_enemy_projectile(struct ProjectileSystem *system, struct Enemy *enemy) {
    float muzzle_offset_x = enemy->facing_right ? ENEMY_MUZZLE_OFFSET_X : -ENEMY_MUZZLE_OFFSET_X;
    
    float muzzle_x = enemy->entity.x + muzzle_offset_x;
    float muzzle_y = enemy->entity.y - ENEMY_MUZZLE_OFFSET_Y;
    
    spawn_projectile(
        system,
        muzzle_x,
        muzzle_y,
        enemy->facing_right,
        PROJECTILE_ENEMY,
        PROJECTILE_NORMAL,
        enemy->damage
    );
}

void spawn_boss_projectile(struct ProjectileSystem *system, struct Boss *boss) {
    float muzzle_offset_x = boss->facing_right ? BOSS_MUZZLE_OFFSET_X : -BOSS_MUZZLE_OFFSET_X;

    float muzzle_x = boss->entity.x + muzzle_offset_x;
    float muzzle_y = boss->entity.y - BOSS_PROJECTILE_OFFSET_Y; 

    spawn_projectile(
        system,
        muzzle_x,
        muzzle_y,
        boss->facing_right,
        PROJECTILE_ENEMY,
        PROJECTILE_NORMAL,
        boss->projectile_damage
    );
}

void check_projectile_collisions(struct ProjectileSystem *system, 
                               struct Player *player, 
                               struct EnemySystem *enemy_system, 
                               struct GameLevel *level) {

    // --- CORREÇÃO INÍCIO ---
    // Crie uma entidade temporária para o jogador com coordenadas de MUNDO.
    struct Entity player_world_entity = player->entity;
    // A posição X no mundo é a posição na tela + o deslocamento da câmera.
    player_world_entity.x = player->entity.x + level->scroll_x;
    // A posição Y não é afetada pelo scroll neste jogo.
    player_world_entity.y = player->entity.y;
    // Recalcule a hitbox para esta posição no mundo.
    update_hitbox_position(&player_world_entity, player->facing_right);
    // --- CORREÇÃO FIM ---

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!system->projectiles[i].is_active) continue;
        
        struct Projectile *p = &system->projectiles[i];
        
        // Colisão de projéteis de INIMIGOS contra o JOGADOR
        if (p->type == PROJECTILE_ENEMY && !is_player_dead(player)) {
            // Agora comparamos o projétil (mundo) com a entidade temporária do jogador (mundo)
            if (check_collision(&p->entity, &player_world_entity)) {
                damage_player(player, p->damage);
                p->is_active = false;
                system->active_count--;
                continue; // Pula para o próximo projétil
            }
        }
        
        // Colisão de projéteis do JOGADOR contra INIMIGOS
        if (p->type == PROJECTILE_PLAYER) {
            bool hit_someone = false;

            // Loop para inimigos normais
            for (int j = 0; j < MAX_ENEMIES; j++) {
                struct Enemy *current_enemy = &enemy_system->enemies[j];
                if (current_enemy->is_active && !is_enemy_dead(current_enemy)) {
                    // Esta checagem já estava correta, pois ambos (projétil e inimigo) estão no espaço do mundo.
                    if (check_collision(&p->entity, &current_enemy->entity)) {
                        damage_enemy(current_enemy, p->damage, player);
                        hit_someone = true;
                        // Se o projétil não for perfurante, paramos aqui
                        if (p->behavior == PROJECTILE_NORMAL) break;
                    }
                }
            }
            
            // Se o projétil atingiu alguém e não é perfurante, ele é desativado.
            // Se for perfurante, ele continua, então a checagem do chefe é feita separadamente.
            if (hit_someone && p->behavior == PROJECTILE_NORMAL) {
                p->is_active = false;
                system->active_count--;
                continue; // Pula para o próximo projétil
            }

            // Colisão com o chefe (só acontece se o projétil não foi destruído acima)
            if (enemy_system->boss.is_active && !is_boss_dead(&enemy_system->boss)) {
                 if (check_collision(&p->entity, &enemy_system->boss.entity)) {
                    damage_boss(&enemy_system->boss, p->damage, player);
                    hit_someone = true;
                }
            }

            // Se o projétil atingiu o chefe (ou um inimigo, no caso de ser perfurante)
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
