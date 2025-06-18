#ifndef UTILS_H
#define UTILS_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>

// Funções utilitárias
void tela_gameover(ALLEGRO_DISPLAY* disp, ALLEGRO_BITMAP* gameover_img);
void tela_vitoria(ALLEGRO_DISPLAY* disp, ALLEGRO_BITMAP* victory_img);
void tela_pausa(ALLEGRO_DISPLAY* disp, ALLEGRO_BITMAP* pause_img);

#endif