#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>

// Enum dos índices dos sprites do player
enum PlayerSpriteIndex {
    SPRITE_PARADO_DIR = 0,
    SPRITE_PULANDO_DIR = 1,
    SPRITE_PULANDO_ESQ = 2,
    SPRITE_PARADO_ESQ = 3,
    SPRITE_CORRENDO_DIR = 4,
    SPRITE_ATIRANDO_DIR = 5,
    SPRITE_ATIRANDO_ESQ = 6,
    SPRITE_CORRENDO_ESQ = 7,
    SPRITE_ABAIXADO_ESQ = 8,
    SPRITE_ABAIXADO_DIR = 9,
    SPRITE_ATIRANDO_ABAIXADO_ESQ = 10,
    SPRITE_ATIRANDO_ABAIXADO_DIR = 11,
    SPRITE_ATIRANDO_CIMA_ESQ = 12,
    SPRITE_ATIRANDO_CIMA_DIR = 13,
    SPRITE_ATIRANDO_DIAG_ESQ = 14,
    SPRITE_ATIRANDO_DIAG_DIR = 15
};

// Struct do player
struct Player {
    float x, y;
    int direcao; // 0: direita, 1: esquerda
    bool no_chao;
    bool pulando;
    bool abaixado;
    bool atirando;
    bool atirando_cima;
    bool atirando_diag;
};

// Struct para frames do sprite
struct SpriteFrame {
    int x, y, w, h;
};

// Array de frames do player -posicionamento dos sprites na imagem
extern struct SpriteFrame player_frames[16];

// Função para escolher o índice do sprite do player
int player_get_sprite_index(struct Player* p);

#endif