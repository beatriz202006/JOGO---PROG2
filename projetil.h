#ifndef PROJETIL_H
#define PROJETIL_H

#include "config.h"

// Struct para projétil normal do player
struct Bullet {
    float x, y;
    float vx, vy;
    int ativa;
};

// Struct para chamas (projetil dos inimigos comuns)
struct Chama {
    float x, y;
    float vx, vy;
    int ativa;
};

// Arrays globais dos projéteis e chamas
extern struct Bullet bullets[MAX_BULLETS];
extern struct Chama chamas[MAX_CHAMAS];

// Funções para inicializar (resetar) os projéteis/chamas
void projeteis_resetar(void);

#endif