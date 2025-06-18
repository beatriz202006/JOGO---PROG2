#include "player.h"

// Array de frames do player
struct SpriteFrame player_frames[16] = {
    {   5,   4, 108, 122}, // 1
    { 121,   5, 109, 121}, // 2
    { 265,   5, 120, 124}, // 3
    { 383,   6, 111, 122}, // 4
    { 512,   3,  94, 121}, // 5
    { 615,   1,  92, 127}, // 6
    { 772,   4,  88, 122}, // 7
    { 868,   2,  83, 119}, // 8
    { 968,   4, 103, 123}, // 9
    {1087,   5,  86, 124}, // 10
    {1217,   1, 115, 124}, // 11
    {1349,   1, 114, 124}, // 12
    {1496,   3,  87, 125}, // 13
    {1592,   4,  66, 125}, // 14
    {1672,   3, 107, 120}, // 15
    {1791,   2,  99, 125}  // 16
};

// Função para escolher o índice do sprite do player
int player_get_sprite_index(struct Player* p) {
    if (p->no_chao) {
        if (p->abaixado) {
            if (p->atirando) {
                return (p->direcao == 0) ? SPRITE_ATIRANDO_ABAIXADO_DIR : SPRITE_ATIRANDO_ABAIXADO_ESQ;
            }
            return (p->direcao == 0) ? SPRITE_ABAIXADO_DIR : SPRITE_ABAIXADO_ESQ;
        }
        if (p->atirando_diag) {
            return (p->direcao == 0) ? SPRITE_ATIRANDO_DIAG_DIR : SPRITE_ATIRANDO_DIAG_ESQ;
        }
        if (p->atirando) {
            return (p->direcao == 0) ? SPRITE_ATIRANDO_DIR : SPRITE_ATIRANDO_ESQ;
        }
        // Correndo
        return (p->direcao == 0) ? SPRITE_PARADO_DIR : SPRITE_PARADO_ESQ;
    } else {
        // No ar (pulando)
        if (p->atirando_cima) {
            return (p->direcao == 0) ? SPRITE_ATIRANDO_CIMA_DIR : SPRITE_ATIRANDO_CIMA_ESQ;
        }
        if (p->atirando_diag) {
            return (p->direcao == 0) ? SPRITE_ATIRANDO_DIAG_DIR : SPRITE_ATIRANDO_DIAG_ESQ;
        }
        if (p->atirando) {
            return (p->direcao == 0) ? SPRITE_ATIRANDO_DIR : SPRITE_ATIRANDO_ESQ;
        }
        return (p->direcao == 0) ? SPRITE_PULANDO_DIR : SPRITE_PULANDO_ESQ;
    }
}