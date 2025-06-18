#include "utils.h"
#include "config.h"

void tela_gameover(ALLEGRO_DISPLAY* disp, ALLEGRO_BITMAP* gameover_img) {
    if (!gameover_img) {
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_text(al_create_builtin_font(), al_map_rgb(255,0,0), X_SCREEN/2, Y_SCREEN/2, ALLEGRO_ALIGN_CENTRE, "GAME OVER");
        al_flip_display();
        al_rest(GAMEOVER_TIME);
        return;
    }
    // Calcula a escala para encaixar a largura da tela
    float escala = (float)X_SCREEN / 2048;
    int w = X_SCREEN; // vai encaixar na largura total da tela
    int h = 3072 * escala; // mantém a proporção original da imagem
    int x = 0;
    int y = (Y_SCREEN - h) / 2; // centraliza verticalmente (pode ficar negativo, cortando em cima/baixo)
    al_draw_scaled_bitmap(gameover_img, 0, 0, 1800, 1800, x, y, w, h, 0);
    al_flip_display();
    al_rest(GAMEOVER_TIME);
}

void tela_vitoria(ALLEGRO_DISPLAY* disp, ALLEGRO_BITMAP* victory_img) {
    if (!victory_img) {
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_text(al_create_builtin_font(), al_map_rgb(0,255,0), X_SCREEN/2, Y_SCREEN/2, ALLEGRO_ALIGN_CENTRE, "VOCÊ VENCEU!");
        al_flip_display();
        al_rest(GAMEOVER_TIME);
        return;
    }
    // Preencher largura total (ajuste igual ao anterior)
    float escala = (float)X_SCREEN / 2048;
    int w = X_SCREEN;
    int h = 3072 * escala;
    int x = 0;
    int y = (Y_SCREEN - h) / 2;
    al_draw_scaled_bitmap(victory_img, 0, 0, 1800, 1800, x, y, w, h, 0);
    al_flip_display();
    al_rest(GAMEOVER_TIME);
}

void tela_pausa(ALLEGRO_DISPLAY* disp, ALLEGRO_BITMAP* pause_img) {
    if (!pause_img) {
        al_clear_to_color(al_map_rgba(0,0,0,200));
        ALLEGRO_FONT* f = al_create_builtin_font();
        al_draw_text(f, al_map_rgb(255,255,0), X_SCREEN/2, Y_SCREEN/2, ALLEGRO_ALIGN_CENTRE, "PAUSADO");
        al_draw_text(f, al_map_rgb(255,255,255), X_SCREEN/2, Y_SCREEN/2+40, ALLEGRO_ALIGN_CENTRE, "Pressione P novamente para continuar");
        al_flip_display();
        al_destroy_font(f);
        return;
    }
    // Escala para preencher a largura da tela e centralizar verticalmente
    float escala = (float)X_SCREEN / 1024.0f;
    int w = X_SCREEN;
    int h = 1536 * escala;
    int x = 0;
    int y = (Y_SCREEN - h) / 2;
    al_draw_scaled_bitmap(
        pause_img,
        0, 0, 1024, 1536,
        x, y, w, h, 0
    );
    al_flip_display();
}