#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include "Square.h"
#include <stdio.h>
#include <stdbool.h>
#define X_SCREEN 1200
#define Y_SCREEN 1000

typedef enum { MENU, GAME, EXIT } GameState;

int main() {
    al_init();
    al_init_primitives_addon();
    al_init_image_addon();
    al_install_keyboard();

    ALLEGRO_DISPLAY* disp = al_create_display(X_SCREEN, Y_SCREEN);
    ALLEGRO_FONT* font = al_create_builtin_font();
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));

    ALLEGRO_BITMAP* bg = al_load_bitmap("backgroundfull.png");
    if (!bg) {
        printf("Erro ao carregar background!\n");
        al_destroy_font(font);
        al_destroy_display(disp);
        al_destroy_event_queue(queue);
        return 1;
    }

    int bg_width = al_get_bitmap_width(bg);
    int bg_height = al_get_bitmap_height(bg);

    // Carrega a spritesheet do personagem
    ALLEGRO_BITMAP* sprite_sheet = al_load_bitmap("image.psd.png");
    if (!sprite_sheet) {
        printf("Erro ao carregar sprite do personagem!\n");
        al_destroy_bitmap(bg);
        al_destroy_font(font);
        al_destroy_display(disp);
        al_destroy_event_queue(queue);
        return 1;
    }

    // Carrega a spritesheet dos sprites "atirando abaixado"
    ALLEGRO_BITMAP* sprite_down = al_load_bitmap("spriteatirandoabaixadofull.png");
    if (!sprite_down) {
        printf("Erro ao carregar sprite atirando abaixado!\n");
        al_destroy_bitmap(sprite_sheet);
        al_destroy_bitmap(bg);
        al_destroy_font(font);
        al_destroy_display(disp);
        al_destroy_event_queue(queue);
        return 1;
    }
    int SPRITE_DOWN_W = 182; // 364 / 2
    int SPRITE_DOWN_H = 164;

    // Carrega a spritesheet dos sprites "atirando para cima"
    ALLEGRO_BITMAP* sprite_up = al_load_bitmap("spriteatirandocimafull.png");
    if (!sprite_up) {
        printf("Erro ao carregar sprite atirando para cima!\n");
        al_destroy_bitmap(sprite_down);
        al_destroy_bitmap(sprite_sheet);
        al_destroy_bitmap(bg);
        al_destroy_font(font);
        al_destroy_display(disp);
        al_destroy_event_queue(queue);
        return 1;
    }
    int SPRITE_UP_W = 106; // 212 / 2
    int SPRITE_UP_H = 164;

    al_convert_mask_to_alpha(sprite_sheet, al_map_rgb(200, 200, 200));
    al_convert_mask_to_alpha(sprite_sheet, al_map_rgb(255,255,255));
    al_convert_mask_to_alpha(sprite_sheet, al_map_rgb(0,0,0));
    int SPRITE_W = 128;
    int SPRITE_H = 128;

    GameState state = MENU;
    int menu_option = 0; // 0 = Iniciar, 1 = Sair

    while (state != EXIT) {
        if (state == MENU) {
            bool menu_running = true;
            while (menu_running) {
                al_clear_to_color(al_map_rgb(10, 10, 80));
                al_draw_text(font, al_map_rgb(255,255,255), X_SCREEN/2, Y_SCREEN/2-20, ALLEGRO_ALIGN_CENTRE, "INICIAR");
                al_draw_text(font, al_map_rgb(255,255,255), X_SCREEN/2, Y_SCREEN/2+20, ALLEGRO_ALIGN_CENTRE, "SAIR");
                if (menu_option == 0)
                    al_draw_rectangle(X_SCREEN/2-40, Y_SCREEN/2-25, X_SCREEN/2+40, Y_SCREEN/2-5, al_map_rgb(255,255,0), 2);
                else
                    al_draw_rectangle(X_SCREEN/2-40, Y_SCREEN/2+15, X_SCREEN/2+40, Y_SCREEN/2+35, al_map_rgb(255,255,0), 2);

                al_flip_display();

                ALLEGRO_EVENT event;
                al_wait_for_event(queue, &event);
                if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                    if (event.keyboard.keycode == ALLEGRO_KEY_UP || event.keyboard.keycode == ALLEGRO_KEY_DOWN)
                        menu_option = 1 - menu_option;
                    else if (event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                        if (menu_option == 0) { state = GAME; menu_running = false; }
                        else { state = EXIT; menu_running = false; }
                    }
                }
                else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                    state = EXIT;
                    menu_running = false;
                }
            }
        }

        if (state == GAME) {
            // Plataforma (chão)
            int plataforma_y = Y_SCREEN - 265; // chão ->define onde o quadrado vai pousar
            // Plataforma suspensa
            int plat_x = 400;
            int plat_w = 400;
            int plat_y = 750;
            int plat_h = 20;

            // Quadrado começa em cima da plataforma suspensa
            square* player = square_create(50, plat_x + plat_w/2, Y_SCREEN/2 - 200, X_SCREEN, Y_SCREEN);
            if (!player) {
                printf("Erro ao criar quadrado!\n");
                state = EXIT;
                continue;
            }

            int bg_offset_x = 0;
            int player_speed = 10;
            bool jogando = true;
            bool key[ALLEGRO_KEY_MAX] = {false};

            // Pulo
            float vel_y = 0;
            bool no_chao = false;

            // Controle de direção e animação
            int direcao = 0; // 0 = direita, 1 = esquerda

            while (jogando) {
                ALLEGRO_EVENT event;
                if (al_get_next_event(queue, &event)) {
                    if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
                        jogando = false;
                    else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
                        key[event.keyboard.keycode] = true;
                    else if (event.type == ALLEGRO_EVENT_KEY_UP)
                        key[event.keyboard.keycode] = false;
                }

                // Atualiza direção
                if (key[ALLEGRO_KEY_LEFT]) direcao = 1;
                if (key[ALLEGRO_KEY_RIGHT]) direcao = 0;

                // Movimento horizontal contínuo
                bool andando = false, correndo = false;
                static int frame_counter = 0;
                frame_counter++;

                if (key[ALLEGRO_KEY_LEFT]) {
                    square_move(player, 1, 0, X_SCREEN, Y_SCREEN);
                    bg_offset_x -= player_speed;
                    if (bg_offset_x < 0) bg_offset_x = 0;
                    andando = true;
                    if (key[ALLEGRO_KEY_LEFT] && frame_counter % 3 == 0) correndo = true;
                }
                if (key[ALLEGRO_KEY_RIGHT]) {
                    square_move(player, 1, 1, X_SCREEN, Y_SCREEN);
                    bg_offset_x += player_speed;
                    andando = true;
                    if (key[ALLEGRO_KEY_RIGHT] && frame_counter % 3 == 0) correndo = true;
                }

                // Pulo com espaço ou seta para cima
                if ((key[ALLEGRO_KEY_SPACE] || key[ALLEGRO_KEY_UP]) && no_chao) {
                    vel_y = -20;
                    no_chao = false;
                }

                // Gravidade e colisão com plataformas
                if (!no_chao) {
                    player->y += vel_y;
                    vel_y += 1.5;

                    // Colisão com plataforma suspensa (só se estiver caindo)
                    if (
                        vel_y > 0 &&
                        player->y + player->side/2 >= plat_y &&
                        player->y + player->side/2 - vel_y < plat_y &&
                        player->x + player->side/2 > plat_x &&
                        player->x - player->side/2 < plat_x + plat_w
                    ) {
                        player->y = plat_y - player->side/2;
                        vel_y = 0;
                        no_chao = true;
                    }
                    // Colisão com chão
                    else if (player->y + player->side/2 >= plataforma_y) {
                        player->y = plataforma_y - player->side/2;
                        vel_y = 0;
                        no_chao = true;
                    }
                }

                if (key[ALLEGRO_KEY_ESCAPE])
                    jogando = false;

                // Seleção do sprite
                int sprite_row = 0, sprite_col = 0;

                // Desenhar o background tileado (cobre toda a tela)
                int start_x = -(bg_offset_x % bg_width);
                for (int x = start_x; x <= X_SCREEN; x += bg_width) {
                    for (int y = 0; y <= Y_SCREEN; y += bg_height) {
                        al_draw_bitmap(bg, x, y, 0);
                    }
                }

                // --- Desenhar o personagem ---
                // Atirando para cima (seta para cima + Z)
                if (key[ALLEGRO_KEY_UP] && key[ALLEGRO_KEY_Z]) {
                    int up_col = (direcao == 0) ? 1 : 0; // 0 = esquerda, 1 = direita
                    al_draw_bitmap_region(
                        sprite_up,
                        up_col * SPRITE_UP_W, 0,
                        SPRITE_UP_W, SPRITE_UP_H,
                        player->x - SPRITE_UP_W/2,
                        player->y + player->side/2 - SPRITE_UP_H, // base alinhada
                        0
                    );
                }
                // Atirando abaixado (seta para baixo + Z)
                else if (key[ALLEGRO_KEY_DOWN] && key[ALLEGRO_KEY_Z] && no_chao) {
                    int down_col = (direcao == 0) ? 1 : 0; // 0 = esquerda, 1 = direita
                    al_draw_bitmap_region(
                        sprite_down,
                        down_col * SPRITE_DOWN_W, 0,
                        SPRITE_DOWN_W, SPRITE_DOWN_H,
                        player->x - SPRITE_DOWN_W/2,
                        player->y + player->side/2 - SPRITE_DOWN_H, // <-- base alinhada
                        0
                    );
                } else {
                    // Abaixado
                    if (key[ALLEGRO_KEY_DOWN] && no_chao) {
                        sprite_row = 1;
                        sprite_col = (direcao == 0) ? 0 : 3;
                    }
                    // Atirando (tecla Z)
                    else if (key[ALLEGRO_KEY_Z]) {
                        sprite_row = 2;
                        sprite_col = (direcao == 0) ? 1 : 2;
                    }
                    // Pulando
                    else if (!no_chao) {
                        sprite_row = 0;
                        sprite_col = (direcao == 0) ? 1 : 2;
                    }
                    // Correndo (seta pressionada continuamente)
                    else if ((key[ALLEGRO_KEY_LEFT] || key[ALLEGRO_KEY_RIGHT]) && no_chao && (key[ALLEGRO_KEY_LEFT] || key[ALLEGRO_KEY_RIGHT])) {
                        sprite_row = 1;
                        sprite_col = (direcao == 0) ? 1 : 2;
                    }
                    // Andando (apertando vez por vez)
                    else if (andando && no_chao) {
                        sprite_row = 2;
                        sprite_col = (direcao == 0) ? 0 : 3;
                    }
                    // Parado
                    else if (no_chao) {
                        sprite_row = 0;
                        sprite_col = (direcao == 0) ? 0 : 3;
                    }

                    al_draw_bitmap_region(
                        sprite_sheet,
                        sprite_col * SPRITE_W, sprite_row * SPRITE_H,
                        SPRITE_W, SPRITE_H,
                        player->x - SPRITE_W/2,
                        player->y + player->side/2 - SPRITE_H, // <-- base alinhada
                        0
                    );
                }

                al_flip_display();
                al_rest(0.01);
            }

            square_destroy(player);
            state = MENU;
        }
    }

    al_destroy_bitmap(sprite_up);
    al_destroy_bitmap(sprite_down);
    al_destroy_bitmap(sprite_sheet);
    al_destroy_bitmap(bg);
    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_event_queue(queue);
    return 0;
}