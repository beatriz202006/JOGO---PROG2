#include "boss.h"

// Inicializa o array global das bolas de fogo
struct BolaFogo bolas_fogo[MAX_BOLAS_FOGO] = {0};

// Retorna a coluna da barra de vida do boss
int boss_bar_col(struct Boss boss) {
    if (boss.estado == BOSS_MORTO) return 1; // última coluna
    if (boss.estado == BOSS_DESFAZENDO) return 1;
    if (boss.vida > 6) return 0; // cheia e um pouco de dano
    if (boss.vida > 3) return 0; // metade
    if (boss.vida > 0) return 1; // finzinho
    return 1;
}

// Retorna a linha da barra de vida do boss
int boss_bar_row(struct Boss boss) {
    if (boss.estado == BOSS_MORTO) return 2; // última linha
    if (boss.estado == BOSS_DESFAZENDO) return 1; // linha do finzinho
    if (boss.vida > 8) return 0; // cheia (9-10)
    if (boss.vida > 6) return 1; // tomou um pouco de dano (7-8)
    if (boss.vida > 3) return 2; // metade (4-6)
    if (boss.vida > 0) return 1; // finzinho (1-3)
    return 2; // vazia (0)
}

// Faz o boss lançar uma bola de fogo
void boss_lanca_bola_fogo(struct Boss *boss) {
    for (int i = 0; i < MAX_BOLAS_FOGO; i++) {
        if (!bolas_fogo[i].ativa) {
            bolas_fogo[i].ativa = 1;
            bolas_fogo[i].x = boss->x - 20;
            bolas_fogo[i].y = boss->y - 180;
            bolas_fogo[i].vx = -8;
            bolas_fogo[i].vy = 0;
            break;
        }
    }
}