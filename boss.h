#ifndef BOSS_H
#define BOSS_H

#include "config.h"

// Enum para os estados do boss
enum BossEstados {
    BOSS_ESCUDO = 0,
    BOSS_PARADO = 1,
    BOSS_ATACANDO = 2,
    BOSS_ANDANDO = 3,
    BOSS_DESFAZENDO = 4,
    BOSS_DANO = 5,
    BOSS_MORTO = 6
};

// Struct do Boss
struct Boss {
    float x, y;
    int estado; // usa enum BossEstados
    int vida;
};

// Struct da bola de fogo do boss
struct BolaFogo {
    float x, y;
    float vx, vy;
    int ativa;
};

// Array global das bolas de fogo
extern struct BolaFogo bolas_fogo[MAX_BOLAS_FOGO];

// Funções relacionadas ao boss
int boss_bar_col(struct Boss boss);
int boss_bar_row(struct Boss boss);
void boss_lanca_bola_fogo(struct Boss *boss);

#endif