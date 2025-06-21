#ifndef BOSS_H
#define BOSS_H

#include "types.h"
#include "player.h"
#include "projectiles.h"

// =================================================
// === CONSTANTES DE CONFIGURAÇÃO DO CHEFE ========
// =================================================

#define BOSS_HEALTH 2500
#define BOSS_PROJECTILE_DAMAGE 25
#define BOSS_PROJECTILE_OFFSET_Y (128 * 0.75f) 

// --- Ataques ---
#define BOSS_LUNGE_SPEED 520.0f
#define BOSS_LUNGE_DAMAGE 75
#define BOSS_PUNCH_RANGE 120.0f
#define BOSS_PUNCH_DAMAGE 50
#define BOSS_PUNCH_DURATION 0.8f

// O chefe contra-ataca após ser atingido este número de vezes seguidas.
#define BOSS_POISE_THRESHOLD 3

// --- Temporizadores da IA (AJUSTE: Mais agressivo) ---
#define BOSS_ATTACK_COOLDOWN 1.2f
#define BOSS_LUNGE_PREP_TIME 0.9f
#define BOSS_LUNGE_DURATION 1.6f
#define BOSS_SHOOT_DURATION 2.0f
#define BOSS_SHOOT_COOLDOWN 0.15f
#define BOSS_POST_ATTACK_COOLDOWN 0.8f
#define BOSS_SHOOT_FRAME_DELAY 0.09f

// MODO FÚRIA (AJUSTE: Ainda mais agressivo)
#define BOSS_ENRAGED_ATTACK_COOLDOWN 0.6f
#define BOSS_ENRAGED_LUNGE_PREP_TIME 0.5f
#define BOSS_ENRAGED_SHOOT_COOLDOWN 0.11f
#define BOSS_ENRAGED_POST_ATTACK_COOLDOWN 0.5f

// --- Outros Timers ---
#define BOSS_HURT_DURATION 0.35f
#define BOSS_DEATH_FADEOUT_TIME 4.0f

// =================================================
// === ESTRUTURAS E ENUMS DO CHEFE =================
// =================================================

typedef enum {
    BOSS_STATE_IDLE,
    BOSS_STATE_SHOOTING,
    BOSS_STATE_PREPARING_LUNGE,
    BOSS_STATE_LUNGING,
    BOSS_STATE_PUNCHING,
    BOSS_STATE_COOLDOWN,
    BOSS_STATE_HURT,
    BOSS_STATE_DEAD
} BossState;

typedef enum {
    BOSS_ANIM_IDLE, BOSS_ANIM_WALK, BOSS_ANIM_RUN, BOSS_ANIM_JUMP, BOSS_ANIM_SHOOT,
    BOSS_ANIM_LUNGE_ATTACK, BOSS_ANIM_TAUNT, BOSS_ANIM_HURT, BOSS_ANIM_DEATH,
    BOSS_ANIM_COUNT
} BossAnimState;

struct Boss {
    struct Entity entity;
    BossState state;

    int health;
    int max_health;
    int projectile_damage;
    int lunge_damage;
    int punch_damage;

    float state_timer;
    bool has_hit_with_lunge;
    bool has_hit_with_punch;
    
    // NOVO MECANISMO: Contador de postura
    int poise_hits;
    
    bool is_enraged;

    struct Animation animations[BOSS_ANIM_COUNT];
    struct Animation *current_animation;

    bool is_active;
    bool facing_right;
    bool is_dead;
    float death_timer;
};


// =================================================
// === PROTÓTIPOS DAS FUNÇÕES ======================
// =================================================

// --- Funções auxiliares ---
void draw_boss_health_bar(struct Boss *boss);
void kill_boss(struct Boss *boss, struct Player *player);

// --- Funções de Ciclo de Vida ---
void init_boss(struct Boss *boss, float x, float y);
void load_boss_sprites(struct Boss *boss);
void unload_boss_sprites(struct Boss *boss);

// --- Funções de Jogo ---
void update_boss(struct Boss *boss, struct Player *player, struct GameLevel *level, struct ProjectileSystem *projectile_system, float delta_time);
void draw_boss(struct Boss *boss, float scroll_x, bool show_hitbox);

// --- Funções de Estado ---
void damage_boss(struct Boss *boss, int amount, struct Player *player);
bool is_boss_dead(struct Boss *boss);

#endif 