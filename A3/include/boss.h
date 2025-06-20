#ifndef BOSS_H
#define BOSS_H

#include "types.h"
#include "player.h"
#include "projectiles.h"

// =================================================
// === CONSTANTES DE CONFIGURAÇÃO DO CHEFE ========
// =================================================

#define BOSS_HEALTH 1500
#define BOSS_WALK_SPEED 80.0f
#define BOSS_PROJECTILE_DAMAGE 25

// --- Ataque de Investida (Lunge) ---
#define BOSS_LUNGE_SPEED 480.0f
#define BOSS_LUNGE_JUMP_VEL -750.0f
#define BOSS_LUNGE_DAMAGE 75

// --- Ataque de Tiro ---
#define BOSS_SHOOT_COOLDOWN 0.1f // Cadência de tiro durante a rajada

// --- Temporizadores da IA ---
#define BOSS_ATTACK_COOLDOWN 4.0f    // Tempo de espera entre diferentes ataques
#define BOSS_LUNGE_PREP_TIME 1.2f    // Tempo de "aviso" antes da investida
#define BOSS_SHOOT_DURATION 3.0f     // Duração da rajada de tiros
#define BOSS_POST_LUNGE_COOLDOWN 2.5f // Tempo de recuperação após a investida

// =================================================
// === ESTRUTURAS E ENUMS DO CHEFE =================
// =================================================

// --- Estados de Comportamento (IA) ---
typedef enum {
    BOSS_STATE_IDLE,          // Parado, decidindo o que fazer
    BOSS_STATE_SHOOTING,      // Executando o ataque de projéteis
    BOSS_STATE_PREPARING_LUNGE, // "Aviso" de que a investida vai começar
    BOSS_STATE_LUNGING,       // Movimento de investida (ataque físico)
    BOSS_STATE_COOLDOWN       // Recuperando após um ataque
} BossState;

// --- Estados de Animação ---
// Com 9 animações, como você planejou.
typedef enum {
    BOSS_ANIM_IDLE,         // 0
    BOSS_ANIM_WALK,         // 1
    BOSS_ANIM_RUN,          // 2 (Para a investida)
    BOSS_ANIM_JUMP,         // 3 (Para o pulo da investida)
    BOSS_ANIM_SHOOT,        // 4 (Ataque de projétil)
    BOSS_ANIM_LUNGE_ATTACK, // 5 (Um ataque físico no final da investida, opcional)
    BOSS_ANIM_TAUNT,        // 6 (Provocação, para usar no cooldown)
    BOSS_ANIM_HURT,         // 7
    BOSS_ANIM_DEATH,        // 8
    BOSS_ANIM_COUNT         // Total de animações: 9
} BossAnimState;

// --- Estrutura de Dados Principal do Chefe ---
struct Boss {
    struct Entity entity;
    BossState state;

    // Atributos de combate
    int health;
    int max_health;
    int projectile_damage;
    int lunge_damage;

    // Controle da IA
    float state_timer;
    bool has_hit_with_lunge;

    // Animações
    struct Animation animations[BOSS_ANIM_COUNT];
    struct Animation *current_animation;

    // Flags de estado
    bool is_active;
    bool facing_right;
    bool is_dead;
    float death_timer;
};

// =================================================
// === PROTÓTIPOS DAS FUNÇÕES ======================
// =================================================

// --- Funções auxiliares ---
static void draw_boss_health_bar(struct Boss *boss);
static void kill_boss(struct Boss *boss, struct Player *player);

// --- Funções de Ciclo de Vida ---
void init_boss(struct Boss *boss, float x, float y);
void load_boss_sprites(struct Boss *boss);
void unload_boss_sprites(struct Boss *boss);

// --- Funções de Jogo ---
void update_boss(struct Boss *boss, struct Player *player, struct GameLevel *level, struct ProjectileSystem *projectile_system, float delta_time);
void draw_boss(struct Boss *boss, float scroll_x);

// --- Funções de Estado ---
void damage_boss(struct Boss *boss, int amount, struct Player *player);
bool is_boss_dead(struct Boss *boss);

#endif 