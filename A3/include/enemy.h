#ifndef ENEMY_H
#define ENEMY_H

#include "types.h"

#define MAX_ENEMIES 20

// ---- Estruturas de dados básica ----

struct Enemy {
    struct Entity entity;
    ALLEGRO_BITMAP *sprite;
    int health;
    int damage;
    bool is_active;
};


// ---- Funções ----

// Inicialização
void init_enemies(struct Enemy enemies[], int size);
void load_enemy_sprites(struct Enemy *enemy, const char *sprite_path);
void unload_enemy_sprites(struct Enemy *enemy);

// Controle
void update_enemy(struct Enemy *enemy, struct Player *player);
void enemy_ai(struct Enemy *enemy, struct Player *player);
void enemy_shoot(struct Enemy *enemy, struct Projectile projectiles[], int *projectile_count);

// Estado
void damage_enemy(struct Enemy *enemy, int amount);
bool is_enemy_dead(struct Enemy *enemy);
void spawn_enemy(struct Enemy *enemy, float x, float y);

// Renderização
void draw_enemy(struct Enemy *enemy);

// Chefe
void init_boss(struct Enemy *boss);
void update_boss(struct Enemy *boss, struct Player *player);
void boss_attack_pattern(struct Enemy *boss, struct Projectile projectiles[], int *projectile_count);

#endif