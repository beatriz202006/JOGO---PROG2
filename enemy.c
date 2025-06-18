#include "enemy.h"

// Definição dos arrays globais
struct Inimigo fogos[NUM_FOGOS];
int fogos_vida[NUM_FOGOS];
int fogos_respawn_timer[NUM_FOGOS] = {0};
int fogo_derrotado[NUM_FOGOS] = {0};
int fogo_morto_por_tiro[NUM_FOGOS] = {0};

