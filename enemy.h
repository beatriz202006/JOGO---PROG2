#ifndef ENEMY_H
#define ENEMY_H

#include "config.h"

// Struct do inimigo comum (fogo)
struct Inimigo {
    float x, y;
    int frame;
    int direcao; // 0: direita, 1: esquerda
    float velocidade;
};

// Arrays e variáveis globais dos inimigos
extern struct Inimigo fogos[NUM_FOGOS];
extern int fogos_vida[NUM_FOGOS];
extern int fogos_respawn_timer[NUM_FOGOS];
extern int fogo_derrotado[NUM_FOGOS];
extern int fogo_morto_por_tiro[NUM_FOGOS];

// Funções para inicializar os inimigos (implementação básica)
void inimigos_inicializa(float plat_x, float plat_w, float plat_y, float fogo_frame_h, float escala);

#endif