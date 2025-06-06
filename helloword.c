#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include "Square.h"
#include <stdio.h>
#include <stdbool.h>
#define X_SCREEN 1500
#define Y_SCREEN 800

typedef enum { MENU, GAME, EXIT } GameState;

int main() {
    al_init();
    al_init_primitives_addon();
    al_install_keyboard();

    ALLEGRO_DISPLAY* disp = al_create_display(X_SCREEN, Y_SCREEN);
    ALLEGRO_FONT* font = al_create_builtin_font();
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));

    GameState state = MENU;
    int menu_option = 0; // 0 = Iniciar, 1 = Sair

    while (state != EXIT) {
        if (state == MENU) {
            bool menu_running = true;
            while (menu_running) {
                al_clear_to_color(al_map_rgb(10, 10, 80));
                al_draw_text(font, al_map_rgb(255,255,255), X_SCREEN/2, Y_SCREEN/2-20, ALLEGRO_ALIGN_CENTRE, "INICIAR");
                al_draw_text(font, al_map_rgb(255,255,255), X_SCREEN/2, Y_SCREEN/2+20, ALLEGRO_ALIGN_CENTRE, "SAIR");
                // Destaque a opção selecionada
                if (menu_option == 0)
                    al_draw_rectangle(X_SCREEN/2-40, Y_SCREEN/2-25, X_SCREEN/2+40, Y_SCREEN/2-5, al_map_rgb(255,255,0), 2);
                else
                    al_draw_rectangle(X_SCREEN/2-40, Y_SCREEN/2+15, X_SCREEN/2+40, Y_SCREEN/2+35, al_map_rgb(255,255,0), 2);

                al_flip_display();

                ALLEGRO_EVENT event;
                al_wait_for_event(queue, &event);
                if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                    if (event.keyboard.keycode == ALLEGRO_KEY_UP || event.keyboard.keycode == ALLEGRO_KEY_DOWN)
                        menu_option = 1 - menu_option; // alterna entre 0 e 1
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
            square* player = square_create(50, X_SCREEN/2, Y_SCREEN/2, X_SCREEN, Y_SCREEN);
            if (!player) {
                printf("Erro ao criar quadrado!\n");
                state = EXIT;
                continue;
            }

            bool jogando = true;
            while (jogando) {
                ALLEGRO_EVENT event;
                al_wait_for_event(queue, &event);

                if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
                    jogando = false;
                else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                    if (event.keyboard.keycode == ALLEGRO_KEY_LEFT)
                        square_move(player, 1, 0, X_SCREEN, Y_SCREEN);
                    else if (event.keyboard.keycode == ALLEGRO_KEY_RIGHT)
                        square_move(player, 1, 1, X_SCREEN, Y_SCREEN);
                    else if (event.keyboard.keycode == ALLEGRO_KEY_UP)
                        square_move(player, 1, 2, X_SCREEN, Y_SCREEN);
                    else if (event.keyboard.keycode == ALLEGRO_KEY_DOWN)
                        square_move(player, 1, 3, X_SCREEN, Y_SCREEN);
                    else if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                        jogando = false; // ESC para sair do jogo
                }

                // Desenhar o quadrado
                al_clear_to_color(al_map_rgb(30, 30, 30));
                al_draw_filled_rectangle(
                    player->x - player->side/2, player->y - player->side/2,
                    player->x + player->side/2, player->y + player->side/2,
                    al_map_rgb(255, 0, 0)
                );
                al_flip_display();
            }

            square_destroy(player);
            state = MENU; // Volta para o menu ao sair do jogo
        }
    }

    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_event_queue(queue);
    return 0;
}