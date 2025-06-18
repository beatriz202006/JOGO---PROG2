#include "projetil.h"

// Definição dos arrays globais
struct Bullet bullets[MAX_BULLETS] = {0};
struct Chama chamas[MAX_CHAMAS] = {0};

// Função para resetar todos os projéteis e chamas (ativa = 0)
void projeteis_resetar(void) {
    for (int i = 0; i < MAX_BULLETS; i++)
        bullets[i].ativa = 0;
    for (int i = 0; i < MAX_CHAMAS; i++)
        chamas[i].ativa = 0;
}