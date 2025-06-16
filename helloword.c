#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include "Square.h"
#include <stdio.h>
#include <stdbool.h>
#define X_SCREEN 1200
#define Y_SCREEN 1000
#define MAX_CHAMAS 500
#define MAX_BULLETS 2000
#define NUM_FOGOS 6
#define MAX_BOLAS_FOGO 50
#define GAMEOVER_TIME 4.0f

typedef enum { MENU, GAME, BOSS, FASE3, EXIT, PAUSE } GameState;

struct Bullet {
    float x, y;
    float vx, vy;
    int ativa;
};

struct Bullet bullets[MAX_BULLETS] = {0};

struct inimigo {
    float x, y;
    int frame;
    int direcao; // 0: direita, 1: esquerda
    float velocidade;
};

struct inimigo fogos[NUM_FOGOS];
int fogos_vida[NUM_FOGOS];
int fogos_respawn_timer[NUM_FOGOS] = {0};
int fogo_derrotado[NUM_FOGOS] = {0};
int fogo_morto_por_tiro[NUM_FOGOS] = {0};

struct Chama {
    float x, y;
    float vx, vy;
    int ativa;
};

struct Chama chamas[MAX_CHAMAS] = {0};

enum BossEstados {
    BOSS_ESCUDO = 0,
    BOSS_PARADO = 1,
    BOSS_ATACANDO = 2,
    BOSS_ANDANDO = 3,
    BOSS_DESFAZENDO = 4,
    BOSS_DANO = 5,
    BOSS_MORTO = 6
};

struct Boss {
    float x, y;
    int estado; // ver enum acima
    int vida;
};

struct BolaFogo {
    float x, y;
    float vx, vy;
    int ativa;
};

struct BolaFogo bolas_fogo[MAX_BOLAS_FOGO] = {0};
int boss_bar_col(struct Boss boss) {
    if (boss.estado == BOSS_MORTO) return 1; // última coluna
    if (boss.estado == BOSS_DESFAZENDO) return 1;
    if (boss.vida > 6) return 0; // cheia e um pouco de dano
    if (boss.vida > 3) return 0; // metade
    if (boss.vida > 0) return 1; // finzinho
    return 1;
}
int boss_bar_row(struct Boss boss) {
    if (boss.estado == BOSS_MORTO) return 2; // última linha
    if (boss.estado == BOSS_DESFAZENDO) return 1; // linha do finzinho
    if (boss.vida > 8) return 0; // cheia (9-10)
    if (boss.vida > 6) return 1; // tomou um pouco de dano (7-8)
    if (boss.vida > 3) return 2; // metade (4-6)
    if (boss.vida > 0) return 1; // finzinho (1-3)
    return 2; // vazia (0)
}
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

    ALLEGRO_BITMAP* sprite_sheet = al_load_bitmap("spritesagua.png");
    if (!sprite_sheet) {
        printf("Erro ao carregar sprite do personagem!\n");
        al_destroy_bitmap(bg);
        al_destroy_font(font);
        al_destroy_display(disp);
        al_destroy_event_queue(queue);
        return 1;
    }

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
    int SPRITE_DOWN_W = 182;
    int SPRITE_DOWN_H = 164;

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
    int SPRITE_UP_W = 106;
    int SPRITE_UP_H = 164;

    al_convert_mask_to_alpha(sprite_sheet, al_map_rgb(200, 200, 200));
    al_convert_mask_to_alpha(sprite_sheet, al_map_rgb(255,255,255));
    al_convert_mask_to_alpha(sprite_sheet, al_map_rgb(0,0,0));
    int SPRITE_W = 128;
    int SPRITE_H = 128;

    ALLEGRO_BITMAP* bullet_img = al_load_bitmap("bala.png");
    if (!bullet_img) {
        printf("Erro ao carregar sprite do projetil!\n");
        al_destroy_bitmap(sprite_up);
        al_destroy_bitmap(sprite_down);
        al_destroy_bitmap(sprite_sheet);
        al_destroy_bitmap(bg);
        al_destroy_font(font);
        al_destroy_display(disp);
        al_destroy_event_queue(queue);
        return 1;
    }
    int BULLET_W = 22;
    int BULLET_H = 33;

    ALLEGRO_BITMAP* bullet_boss_img = al_load_bitmap("projetilaguaboss.png");
    if (!bullet_boss_img) {
        printf("Erro ao carregar sprite da bala do boss!\n");
    }
    int BULLET_BOSS_W = 52;
    int BULLET_BOSS_H = 78;

    GameState state = MENU;
    int menu_option = 0;
    GameState state_anterior = MENU;

    ALLEGRO_BITMAP* fogo_sprite = al_load_bitmap("spritesfogofull.png");
    al_convert_mask_to_alpha(fogo_sprite, al_map_rgb(0,0,0));
    if (!fogo_sprite) {
        printf("Erro ao carregar sprite do inimigo!\n");
    }
    int fogo_sprite_w = al_get_bitmap_width(fogo_sprite);
    int fogo_sprite_h = al_get_bitmap_height(fogo_sprite);
    int fogo_frame_w = fogo_sprite_w / 4;
    int fogo_frame_h = fogo_sprite_h;

    int plataforma_y = Y_SCREEN - 265;
    int plat_x = 400;
    int plat_w = 400;
    int plat_y = 750;
    int plat_h = 20;

    float escala = 0.4;
    for (int i = 0; i < NUM_FOGOS; i++) {
        fogos[i].x = X_SCREEN + i * 1800;
        fogos[i].y = plat_y - fogo_frame_h * escala;
        fogos[i].direcao = 1;
        fogos[i].velocidade = 1;
        fogos[i].frame = 2;
        fogos_vida[i] = 2;
        fogos_respawn_timer[i] = i * 900;
        fogo_derrotado[i] = 0;
    }

    ALLEGRO_BITMAP* chama_sprite = al_load_bitmap("chama2.png");
    if (!chama_sprite) {
        printf("Erro ao carregar sprite da chama!\n");
    }
    int CHAMA_W = 46;
    int CHAMA_H = 33;

    ALLEGRO_BITMAP* coracao_sprite = al_load_bitmap("vida.png");
    al_convert_mask_to_alpha(coracao_sprite, al_map_rgb(0,0,0));
    if (!coracao_sprite) {
        printf("Erro ao carregar sprite dos corações!\n");
    }
    int CORACAO_W = 100 / 3;
    int CORACAO_H = 35;

    int vida = 20;
    bool rodada_fogos_acabou = false;
    int stamina_max = 100;
    int stamina = 100;
    int stamina_recupera_tick = 0; // para controlar o tempo de recarga, se quiser
    int stamina_fadiga_tick = 0;   // <-- NOVA VARIÁVEL
    bool cansado = false;    

    ALLEGRO_BITMAP* bg_menu = al_load_bitmap("menu.png");
    if (!bg_menu) {
        printf("Erro ao carregar background do menu!\n");
    }

    int dano_fogo_cooldown = 0;

    ALLEGRO_BITMAP* gameover_img = al_load_bitmap("gameoverlose.png");
    if (!gameover_img) {
        printf("Erro ao carregar imagem de Game Over!\n");
    }

    ALLEGRO_BITMAP* victory_img = al_load_bitmap("gameoverwin.png"); // ou "win.png", etc
    if (!victory_img) {
        printf("Erro ao carregar imagem de Vitória!\n");
    }

    double tempo_gameover = 0;
    int gameover_rodando = 0;
    ALLEGRO_BITMAP* pause_img = al_load_bitmap("teladepausa.png");
    if (!pause_img) {
        printf("Erro ao carregar imagem de pausa!\n");
    }

    ALLEGRO_BITMAP* bg_boss = al_load_bitmap("backgroundchefefull2.png");
    if (!bg_boss) {
        printf("Erro ao carregar background do chefão!\n");
    }

    ALLEGRO_BITMAP* boss_unlocked_img = al_load_bitmap("chefedesbloqueado.png");
    if (!boss_unlocked_img) {
        printf("Erro ao carregar imagem de transição do chefão!\n");
    }

    ALLEGRO_BITMAP* boss_sprite = al_load_bitmap("chefe2.png");
    if (!boss_sprite) {
        printf("Erro ao carregar sprite do chefão!\n");
    }

    int BOSS_FRAME_W = 150;
    int BOSS_FRAME_H = 150;

    ALLEGRO_BITMAP* bola_fogo_sprite = al_load_bitmap("boladefogo2.png");
    if (!bola_fogo_sprite) {
        printf("Erro ao carregar sprite da bola de fogo!\n");
    }
    int BOLA_FOGO_W = 77;
    int BOLA_FOGO_H = 78;

    ALLEGRO_BITMAP* boss_vida_sprite = al_load_bitmap("vidachefe.png");
    if (!boss_vida_sprite) {
        printf("Erro ao carregar sprite das barras de vida do boss!\n");
    }
    int BOSS_VIDA_W = 150;
    int BOSS_VIDA_H = 68;

    ALLEGRO_BITMAP* boss_dano_sprite = al_load_bitmap("chefedano.png");
    if (!boss_dano_sprite) {
        printf("Erro ao carregar sprite de dano do boss!\n");
    }
    int BOSS_DANO_W = 129;
    int BOSS_DANO_H = 105;
    al_convert_mask_to_alpha(boss_dano_sprite, al_map_rgb(0,0,0));

    ALLEGRO_BITMAP* boss_atacando_sprite = al_load_bitmap("chefeatacando.png");
    ALLEGRO_BITMAP* boss_andando_sprite = al_load_bitmap("chefeandando.png");
    ALLEGRO_BITMAP* boss_desfazendo_sprite = al_load_bitmap("chefedesfazendo.png");
    ALLEGRO_BITMAP* boss_morto_sprite = al_load_bitmap("chefemorto.png");

    al_convert_mask_to_alpha(boss_atacando_sprite, al_map_rgb(0,0,0));
    int BOSS_ATACANDO_W = 160, BOSS_ATACANDO_H = 119;
    al_convert_mask_to_alpha(boss_andando_sprite, al_map_rgb(0,0,0));
    int BOSS_ANDANDO_W = 137, BOSS_ANDANDO_H = 113;
    int BOSS_DESFAZENDO_W = 164, BOSS_DESFAZENDO_H = 105;
    al_convert_mask_to_alpha(boss_desfazendo_sprite, al_map_rgb(0,0,0));
    int BOSS_MORTO_W = 146, BOSS_MORTO_H = 64;
    al_convert_mask_to_alpha(boss_morto_sprite, al_map_rgb(0,0,0));

    ALLEGRO_BITMAP* bg_night = al_load_bitmap("backgroundfase3full.png");
    if (!bg_night) {
        printf("Erro ao carregar background da fase noturna!\n");
    }

    while (state != EXIT) {
        // MENU
        if (state == MENU) {
            rodada_fogos_acabou = false;
            for (int i = 0; i < NUM_FOGOS; i++) fogo_derrotado[i] = 0;
            bool menu_running = true;
            while (menu_running) {
                if (bg_menu) {
                    al_draw_bitmap(bg_menu, 0, -300, 0);
                } else {
                    al_clear_to_color(al_map_rgb(10, 10, 80));
                }

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
                continue;
            }
        }

        // GAME
        if (state == GAME) {
            float escala = 0.4;
            for (int i = 0; i < NUM_FOGOS; i++) {
                fogos[i].x = X_SCREEN + i * 1800;
                fogos[i].y = plat_y - fogo_frame_h * escala;
                fogos[i].direcao = 1;
                fogos[i].velocidade = 1;
                fogos[i].frame = 2;
                fogos_vida[i] = 2;
                fogos_respawn_timer[i] = i * 1200;
                fogo_derrotado[i] = 0;
                fogo_morto_por_tiro[i] = 0;
            }
            vida = 20;
            rodada_fogos_acabou = false;
            for (int i = 0; i < MAX_BULLETS; i++) bullets[i].ativa = 0;
            for (int i = 0; i < MAX_CHAMAS; i++) chamas[i].ativa = 0;
            int plataforma_y = Y_SCREEN - 265;
            int plat_x = 400;
            int plat_w = 400;
            int plat_y = 750;
            int plat_h = 20;

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

            float vel_y = 0;
            bool no_chao = false;
            int direcao = 0;
            int travamento_x = X_SCREEN / 2;
            int chama_timer[NUM_FOGOS] = {0};
            int passo = 0;
            int altura_colisao = SPRITE_H;

            stamina = stamina_max;
            cansado = false;
            stamina_fadiga_tick = 0;
            stamina_recupera_tick = 0;

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

                if (key[ALLEGRO_KEY_LEFT]) direcao = 1;
                if (key[ALLEGRO_KEY_RIGHT]) direcao = 0;

                bool andando = false, correndo = false;
                static int frame_counter = 0;
                frame_counter++;

                if (key[ALLEGRO_KEY_RIGHT]) {
                    if (player->x < travamento_x) {
                        square_move(player, 1, 1, X_SCREEN, Y_SCREEN);
                    } else {
                        bg_offset_x += player_speed;
                        for (int f = 0; f < NUM_FOGOS; f++) fogos[f].x -= player_speed;
                        for (int i = 0; i < MAX_BULLETS; i++) if (bullets[i].ativa) bullets[i].x -= player_speed;
                        for (int i = 0; i < MAX_CHAMAS; i++) if (chamas[i].ativa) chamas[i].x -= player_speed;
                        plat_x -= player_speed;
                    }
                    andando = true;
                    if (key[ALLEGRO_KEY_RIGHT] && frame_counter % 3 == 0) correndo = true;
                }

                if (key[ALLEGRO_KEY_LEFT]) {
                    if (player->x > travamento_x) {
                        square_move(player, 1, 0, X_SCREEN, Y_SCREEN);
                    } else if (bg_offset_x > 0) {
                        bg_offset_x -= player_speed;
                        for (int f = 0; f < NUM_FOGOS; f++) fogos[f].x += player_speed;
                        for (int i = 0; i < MAX_BULLETS; i++) if (bullets[i].ativa) bullets[i].x += player_speed;
                        for (int i = 0; i < MAX_CHAMAS; i++) if (chamas[i].ativa) chamas[i].x += player_speed;
                        plat_x += player_speed;
                    } else {
                        square_move(player, 1, 0, X_SCREEN, Y_SCREEN);
                    }
                    andando = true;
                    if (key[ALLEGRO_KEY_LEFT] && frame_counter % 3 == 0) correndo = true;
                }

                if ((key[ALLEGRO_KEY_SPACE] || key[ALLEGRO_KEY_UP]) && no_chao) {
                    vel_y = -20;
                    no_chao = false;
                }

                if (!no_chao) {
                    player->y += vel_y;
                    vel_y += 1.5;
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
                    else if (player->y + player->side/2 >= plataforma_y) {
                        player->y = plataforma_y - player->side/2;
                        vel_y = 0;
                        no_chao = true;
                    }
                }

                if (key[ALLEGRO_KEY_ESCAPE]) {
                    jogando = false;
                    state = MENU;
                }

                int sprite_row = 0, sprite_col = 0;
                int start_x = -(bg_offset_x % bg_width);
                for (int x = start_x; x <= X_SCREEN; x += bg_width) {
                    for (int y = 0; y <= Y_SCREEN; y += bg_height) {
                        al_draw_bitmap(bg, x, y, 0);
                    }
                }

                //Tecla de pausa
                if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                    key[event.keyboard.keycode] = true;
                    if (event.keyboard.keycode == ALLEGRO_KEY_P) {
                        // PAUSE IN-GAME
                        tela_pausa(disp, pause_img);

                        // Limpa eventos antigos
                        ALLEGRO_EVENT temp_event;
                        while (al_get_next_event(queue, &temp_event)) {}

                        // Espera soltar P
                        bool esperando_soltar = true;
                        while (esperando_soltar) {
                            al_wait_for_event(queue, &temp_event);
                            if (temp_event.type == ALLEGRO_EVENT_KEY_UP && temp_event.keyboard.keycode == ALLEGRO_KEY_P)
                                esperando_soltar = false;
                            if (temp_event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                                jogando = false;
                                state = EXIT;
                                return 0;
                            }
                        }

                        // Espera apertar P de novo
                        bool esperando = true;
                        while (esperando) {
                            al_wait_for_event(queue, &temp_event);
                            if (temp_event.type == ALLEGRO_EVENT_KEY_DOWN && temp_event.keyboard.keycode == ALLEGRO_KEY_P)
                                esperando = false;
                            if (temp_event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                                esperando = false;
                                jogando = false;
                                state = EXIT;
                                return 0;
                            }
                        }
                        // Ao sair, apenas continua o loop do jogo normalmente!
                    }
                }

                if (key[ALLEGRO_KEY_UP] && key[ALLEGRO_KEY_Z]) {
                    int up_col = (direcao == 0) ? 1 : 0;
                    al_draw_bitmap_region(
                        sprite_up,
                        up_col * SPRITE_UP_W, 0,
                        SPRITE_UP_W, SPRITE_UP_H,
                        player->x - SPRITE_UP_W/2,
                        player->y + player->side/2 - SPRITE_UP_H,
                        0
                    );
                }
                else if (key[ALLEGRO_KEY_DOWN] && key[ALLEGRO_KEY_Z] && no_chao) {
                    altura_colisao = SPRITE_H * 0.5;
                    int down_col = (direcao == 0) ? 1 : 0;
                    al_draw_bitmap_region(
                        sprite_down,
                        down_col * SPRITE_DOWN_W, 0,
                        SPRITE_DOWN_W, SPRITE_DOWN_H,
                        player->x - SPRITE_DOWN_W/2,
                        player->y + player->side/2 - SPRITE_DOWN_H,
                        0
                    );
                } else {
                    if (key[ALLEGRO_KEY_DOWN] && no_chao) {
                        altura_colisao = SPRITE_H * 0.5;
                        sprite_row = 1;
                        sprite_col = (direcao == 0) ? 0 : 3;
                    }
                    else if (key[ALLEGRO_KEY_Z]) {
                        sprite_row = 2;
                        sprite_col = (direcao == 0) ? 1 : 2;
                    }
                    else if (!no_chao) {
                        sprite_row = 0;
                        sprite_col = (direcao == 0) ? 1 : 2;
                    }
                    else if ((key[ALLEGRO_KEY_LEFT] || key[ALLEGRO_KEY_RIGHT]) && no_chao && (key[ALLEGRO_KEY_LEFT] || key[ALLEGRO_KEY_RIGHT])) {
                        sprite_row = 1;
                        sprite_col = (direcao == 0) ? 1 : 2;
                    }
                    else if (andando && no_chao) {
                        sprite_row = 2;
                        sprite_col = (direcao == 0) ? 0 : 3;
                    }
                    else if (no_chao) {
                        sprite_row = 0;
                        sprite_col = (direcao == 0) ? 0 : 3;
                    }
                    al_draw_bitmap_region(
                        sprite_sheet,
                        sprite_col * SPRITE_W, sprite_row * SPRITE_H,
                        SPRITE_W, SPRITE_H,
                        player->x - SPRITE_W/2,
                        player->y + player->side/2 - SPRITE_H,
                        0
                    );
                }

                static double last_shot_time = 0;
                double now = al_get_time();
                double shot_delay = 0.15;
                if (key[ALLEGRO_KEY_Z] && now - last_shot_time > shot_delay  && stamina >= 10) {
                    last_shot_time = now;
                    stamina -= 10; // Consome estamina no ataque
                    if(stamina < 10) {
                        stamina = 0;
                        cansado = true;
                        stamina_fadiga_tick = 0; // começa a contar o tempo de descanso
                    }

                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (!bullets[i].ativa) {
                            bullets[i].ativa = 1;

                            if (key[ALLEGRO_KEY_UP]) {
                                bullets[i].x = player->x + 10;
                                bullets[i].y = player->y + player->side/2 - SPRITE_UP_H;
                                bullets[i].vx = 0;
                                bullets[i].vy = -15;
                            }
                            else if (key[ALLEGRO_KEY_DOWN] && no_chao) {
                                if (direcao == 0) {
                                    bullets[i].x = player->x + SPRITE_DOWN_W/2;
                                } else {
                                    bullets[i].x = player->x - SPRITE_DOWN_W/2;
                                }
                                bullets[i].y = player->y + player->side/2 - SPRITE_DOWN_H/2 + 40;
                                bullets[i].vx = (direcao == 0) ? 15 : -15;
                                bullets[i].vy = 0;
                            }
                            else {
                                if (direcao == 0) {
                                    bullets[i].x = player->x + SPRITE_W/2;
                                } else {
                                    bullets[i].x = player->x - SPRITE_W/2;
                                }
                                bullets[i].y = player->y + player->side/2 - SPRITE_H + 20;
                                bullets[i].vx = (direcao == 0) ? 15 : -15;
                                bullets[i].vy = 0;
                            }
                            break;
                        }
                    }
                }

                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (bullets[i].ativa) {
                        bullets[i].x += bullets[i].vx;
                        bullets[i].y += bullets[i].vy;
                        al_draw_bitmap(bullet_img, bullets[i].x - BULLET_BOSS_W/2, bullets[i].y, 0);
                    }
                }

                float player_left   = player->x - SPRITE_W/2;
                float player_right  = player->x + SPRITE_W/2;
                float player_top    = player->y + player->side/2 - SPRITE_H;
                float player_bottom = player->y + player->side/2;

                passo++;
                for (int f = 0; f < NUM_FOGOS; f++) {
                    if (fogos_vida[f] > 0 && fogos[f].x < -fogo_frame_w * escala && !rodada_fogos_acabou) {
                        fogos_vida[f] = 0;
                        fogos_respawn_timer[f] = 100 + f * 20;
                        fogos[f].x = -1000;
                        fogo_derrotado[f] = 0;
                        fogo_morto_por_tiro[f] = 0;
                    }
                    if (fogos_vida[f] == 0 && fogos_respawn_timer[f] > 0 && !rodada_fogos_acabou && !fogo_morto_por_tiro[f]) {
                        fogos_respawn_timer[f]--;
                        if (fogos_respawn_timer[f] == 0) {
                            fogos[f].x = X_SCREEN + rand() % 200;
                            fogos_vida[f] = 2;
                            chama_timer[f] = -40;
                            fogo_derrotado[f] = 0;
                            fogo_morto_por_tiro[f] = 0;
                        }
                        chama_timer[f] = 0;
                        continue;
                    }
                    if (fogos_vida[f] <= 0 && rodada_fogos_acabou) {
                        fogos[f].x = -1000;
                        chama_timer[f] = 0;
                        continue;
                    }
                    if (fogos_vida[f] <= 0) {
                        chama_timer[f] = 0;
                        continue;
                    }
                    fogos[f].x -= fogos[f].velocidade;
                    if (passo % 240 < 120) {
                        fogos[f].velocidade = 3;
                        fogos[f].frame = 2;
                    } else {
                        fogos[f].velocidade = 7;
                        fogos[f].frame = 3;
                    }
                    al_draw_scaled_bitmap(
                        fogo_sprite,
                        fogos[f].frame * fogo_frame_w, 0,
                        fogo_frame_w, fogo_frame_h,
                        fogos[f].x, fogos[f].y,
                        fogo_frame_w * escala,
                        fogo_frame_h * escala,
                        0
                    );
                    float fogo_w = fogo_frame_w * escala;
                    float fogo_h = fogo_frame_h * escala;
                    float fogo_left   = fogos[f].x;
                    float fogo_right  = fogos[f].x + fogo_w;
                    float fogo_top    = fogos[f].y;
                    float fogo_bottom = fogos[f].y + fogo_h;
                    if (
                        fogos_vida[f] > 0 &&
                        fogo_right > player_left &&
                        fogo_left < player_right &&
                        fogo_bottom > player_top &&
                        fogo_top < player_bottom
                    ) {
                        if (dano_fogo_cooldown == 0) {
                            vida -= 4;
                            dano_fogo_cooldown = 60;
                        }
                    }
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (bullets[i].ativa && fogos_vida[f] > 0 && fogos[f].x > 0 && fogos[f].x < X_SCREEN - fogo_frame_w * escala) {
                            float bullet_left   = bullets[i].x - BULLET_W/2;
                            float bullet_right  = bullets[i].x + BULLET_W/2;
                            float bullet_top    = bullets[i].y;
                            float bullet_bottom = bullets[i].y + BULLET_H;
                            if (
                                bullet_right > fogo_left &&
                                bullet_left < fogo_right &&
                                bullet_bottom > fogo_top &&
                                bullet_top < fogo_bottom
                            ) {
                                fogos_vida[f]--;
                                if (fogos_vida[f] < 0) fogos_vida[f] = 0;
                                bullets[i].ativa = 0;
                                if (fogos_vida[f] == 0 && !fogo_derrotado[f]) {
                                    fogo_derrotado[f] = 1;
                                    fogo_morto_por_tiro[f] = 1;
                                }
                            }
                        }
                    }
                    chama_timer[f]++;
                    if (chama_timer[f] > 70) {
                        chama_timer[f] = 0;
                        for (int i = 0; i < MAX_CHAMAS; i++) {
                            if (!chamas[i].ativa) {
                                chamas[i].ativa = 1;
                                chamas[i].x = fogos[f].x + (fogo_frame_w * escala) / 4;
                                chamas[i].y = fogos[f].y + (fogo_frame_h * escala) / 2;
                                chamas[i].vx = -10;
                                chamas[i].vy = 0;
                                break;
                            }
                        }
                    }
                }
                for (int i = 0; i < MAX_CHAMAS; i++) {
                    if (chamas[i].ativa) {
                        chamas[i].x += chamas[i].vx;
                        chamas[i].y += chamas[i].vy;
                        al_draw_bitmap(chama_sprite, chamas[i].x, chamas[i].y, 0);
                        if (chamas[i].x < -CHAMA_W || chamas[i].x > X_SCREEN + CHAMA_W ||
                            chamas[i].y < -CHAMA_H || chamas[i].y > Y_SCREEN + CHAMA_H) {
                            chamas[i].ativa = 0;
                        }
                        float chama_left   = chamas[i].x;
                        float chama_right  = chamas[i].x + CHAMA_W;
                        float chama_top    = chamas[i].y;
                        float chama_bottom = chamas[i].y + CHAMA_H;
                        if (
                            chama_right > player_left &&
                            chama_left < player_right &&
                            chama_bottom > player_top &&
                            chama_top < player_bottom
                        ) {
                            vida -= 1;
                            chamas[i].ativa = 0;
                        }
                    }
                }

                stamina_recupera_tick++;
                if (stamina_recupera_tick > 5) { // a cada 6 iterações do loop, recupera 1 ponto de stamina
                    stamina_recupera_tick = 0;
                    if (stamina < stamina_max) stamina++;
                }
                char vida_str[32];
                sprintf(vida_str, "Vida: %d", vida);
                al_draw_text(font, al_map_rgb(255,0,0), 20, 20, 0, vida_str);
                int vida_max = 20;
                int num_coracoes = vida_max / 2;
                for (int i = 0; i < num_coracoes; i++) {
                    int tipo;
                    if (vida >= (i+1)*2) {
                        tipo = 0;
                    } else if (vida == (i*2)+1) {
                        tipo = 2;
                    } else {
                        tipo = 1;
                    }
                    al_draw_bitmap_region(
                        coracao_sprite,
                        tipo * CORACAO_W, 0,
                        CORACAO_W, CORACAO_H,
                        20 + i * (CORACAO_W + 5), 20,
                        0
                    );
                }
                if (vida <= 0) {
                    tela_gameover(disp, gameover_img);
                    jogando = false;
                    state = MENU; // já volta para o menu depois
                }

                bool todos_derrotados = true;
                for (int f = 0; f < NUM_FOGOS; f++) {
                    if (fogos_vida[f] > 0 || !fogo_morto_por_tiro[f]) todos_derrotados = false;
                }
                if (todos_derrotados && !rodada_fogos_acabou) {
                    rodada_fogos_acabou = true;
                    for (int f = 0; f < NUM_FOGOS; f++) {
                        fogos_vida[f] = 0;
                        fogos_respawn_timer[f] = 0;
                        fogos[f].x = -1000;
                    }
                    state = BOSS;
                    jogando = false;
                }

                 // BLOCO DE RECUPERAÇÃO/CANSAÇO DA ESTAMINA - coloque aqui:
                if (cansado) {
                    stamina_fadiga_tick++;
                    if (stamina_fadiga_tick > 60) { // espera 60 frames (~1 segundo)
                        cansado = false;
                        stamina = 0;
                        stamina_recupera_tick = 0;
                    }
                } else {
                    stamina_recupera_tick++;
                    if (stamina_recupera_tick > 5) {
                        stamina_recupera_tick = 0;
                        if (stamina < stamina_max) stamina++;
                    }
                }

                if (dano_fogo_cooldown > 0) dano_fogo_cooldown--;
                // DESENHA BARRA DE ESTAMINA
                int bar_w = 200;
                int bar_h = 20;
                float perc = (float)stamina / stamina_max;
                al_draw_filled_rectangle(20, 60, 20 + bar_w * perc, 60 + bar_h, al_map_rgb(0,200,0));
                al_draw_rectangle(20, 60, 20 + bar_w, 60 + bar_h, al_map_rgb(0,0,0), 2);

                al_flip_display();
                al_rest(0.01);
            }
            square_destroy(player);
            continue;
        }

       // ----- BOSS -----
        if (state == BOSS) {
            // Tela de transição
            if (boss_unlocked_img) {
                al_draw_scaled_bitmap(boss_unlocked_img, 0, 0, 2048, 1536, 0, 0, X_SCREEN, Y_SCREEN, 0);
            } else {
                al_clear_to_color(al_map_rgb(0, 0, 0));
                al_draw_text(font, al_map_rgb(255, 255, 0), X_SCREEN/2, Y_SCREEN/2, ALLEGRO_ALIGN_CENTRE, "Chefão desbloqueado!");
            }
            al_flip_display();
            al_rest(2.0);

            
            // Inicialização do boss
            square* player_boss = square_create(50, 100, Y_SCREEN - 100, X_SCREEN, Y_SCREEN);
            bool boss_running = true;
            bool key[ALLEGRO_KEY_MAX] = {false};
            float vel_y = 0;
            bool no_chao = false;
            int direcao = 0;
            int altura_colisao = SPRITE_H;
            int vida_boss = 20;

            struct Boss boss;
            boss.x = X_SCREEN - 350;
            boss.y = Y_SCREEN - 10;
            boss.estado = BOSS_PARADO;
            boss.vida = 10;

            int boss_state_timer = 0;
            int boss_dano_timer = 0;
            int boss_desfazendo_timer = 0;
            int passos_dados = 0;
            int boss_andando_bola_timer = 0;
            int boss_hit_counter = 0;

            stamina = stamina_max;
            cansado = false;
            stamina_fadiga_tick = 0;
            stamina_recupera_tick = 0;

            // --- BOSS LOOP ---
           while (boss_running) {
            // --- CONTROLE DO PLAYER ---
            ALLEGRO_EVENT event;
            if (al_get_next_event(queue, &event)) {
                if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
                    boss_running = false;
                else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
                    key[event.keyboard.keycode] = true;
                else if (event.type == ALLEGRO_EVENT_KEY_UP)
                    key[event.keyboard.keycode] = false;
            }

            if (key[ALLEGRO_KEY_LEFT]) { square_move(player_boss, 1, 0, X_SCREEN, Y_SCREEN); direcao = 1; }
            if (key[ALLEGRO_KEY_RIGHT]) { square_move(player_boss, 1, 1, X_SCREEN, Y_SCREEN); direcao = 0; }
            if ((key[ALLEGRO_KEY_SPACE] || key[ALLEGRO_KEY_UP]) && no_chao) { vel_y = -20; no_chao = false; }
            if (!no_chao) {
                player_boss->y += vel_y;
                vel_y += 1.5;
                if (player_boss->y + player_boss->side/2 >= Y_SCREEN - 90) {
                    player_boss->y = Y_SCREEN - 90 - player_boss->side/2;
                    vel_y = 0; no_chao = true;
                }
            }
            if (key[ALLEGRO_KEY_ESCAPE]) { boss_running = false; state = MENU; }

            //Tecla de pausa
            if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                key[event.keyboard.keycode] = true;
                if (event.keyboard.keycode == ALLEGRO_KEY_P) {
                    // PAUSE IN-BOSS
                    tela_pausa(disp, pause_img);

                    // Limpa eventos antigos
                    ALLEGRO_EVENT temp_event;
                    while (al_get_next_event(queue, &temp_event)) {}

                    // Espera soltar P
                    bool esperando_soltar = true;
                    while (esperando_soltar) {
                        al_wait_for_event(queue, &temp_event);
                        if (temp_event.type == ALLEGRO_EVENT_KEY_UP && temp_event.keyboard.keycode == ALLEGRO_KEY_P)
                            esperando_soltar = false;
                        if (temp_event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                            boss_running = false;
                            state = EXIT;
                            return 0;
                        }
                    }

                    // Espera apertar P de novo
                    bool esperando = true;
                    while (esperando) {
                        al_wait_for_event(queue, &temp_event);
                        if (temp_event.type == ALLEGRO_EVENT_KEY_DOWN && temp_event.keyboard.keycode == ALLEGRO_KEY_P)
                            esperando = false;
                        if (temp_event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                            esperando = false;
                            boss_running = false;
                            state = EXIT;
                            return 0;
                        }
                    }
                    // Ao sair, apenas continua o loop do boss normalmente!
                }
            }

            al_draw_scaled_bitmap(bg_boss, 0, 0, 2048, 1536, 0, -100, X_SCREEN, Y_SCREEN + 100, 0);

            // --- DESENHO DO PLAYER ---
            int sprite_row = 0, sprite_col = 0;
            if (key[ALLEGRO_KEY_UP] && key[ALLEGRO_KEY_Z]) {
                int up_col = (direcao == 0) ? 1 : 0;
                al_draw_bitmap_region(sprite_up, up_col * SPRITE_UP_W, 0, SPRITE_UP_W, SPRITE_UP_H,
                    player_boss->x - SPRITE_UP_W/2,
                    player_boss->y + player_boss->side/2 - SPRITE_UP_H, 0);
            } else if (key[ALLEGRO_KEY_DOWN] && key[ALLEGRO_KEY_Z] && no_chao) {
                altura_colisao = SPRITE_H * 0.5;
                int down_col = (direcao == 0) ? 1 : 0;
                al_draw_bitmap_region(sprite_down, down_col * SPRITE_DOWN_W, 0, SPRITE_DOWN_W, SPRITE_DOWN_H,
                    player_boss->x - SPRITE_DOWN_W/2,
                    player_boss->y + player_boss->side/2 - SPRITE_DOWN_H, 0);
            } else {
                if (key[ALLEGRO_KEY_DOWN] && no_chao) { altura_colisao = SPRITE_H * 0.5; sprite_row = 1; sprite_col = (direcao == 0) ? 0 : 3; }
                else if (key[ALLEGRO_KEY_Z]) { sprite_row = 2; sprite_col = (direcao == 0) ? 1 : 2; }
                else if (!no_chao) { sprite_row = 0; sprite_col = (direcao == 0) ? 1 : 2; }
                else if ((key[ALLEGRO_KEY_LEFT] || key[ALLEGRO_KEY_RIGHT]) && no_chao) { sprite_row = 1; sprite_col = (direcao == 0) ? 1 : 2; }
                else if (no_chao) { sprite_row = 0; sprite_col = (direcao == 0) ? 0 : 3; }
                al_draw_bitmap_region(sprite_sheet, sprite_col * SPRITE_W, sprite_row * SPRITE_H, SPRITE_W, SPRITE_H,
                    player_boss->x - SPRITE_W/2,
                    player_boss->y + player_boss->side/2 - SPRITE_H, 0);
            }

            static double last_shot_time_boss = 0;
            double now_boss = al_get_time();
            double shot_delay_boss = 0.15;
            if (key[ALLEGRO_KEY_Z] && now_boss - last_shot_time_boss > shot_delay_boss  && stamina >= 10 && !cansado) {
                last_shot_time_boss = now_boss;
                stamina -= 10;
                if(stamina < 10) {
                    stamina = 0;
                    cansado = true;
                    stamina_fadiga_tick = 0; // começa a contar o tempo de descanso
                }

                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (!bullets[i].ativa) {
                        bullets[i].ativa = 1;
                        if (key[ALLEGRO_KEY_UP]) {
                            bullets[i].x = player_boss->x + 10;
                            bullets[i].y = player_boss->y + player_boss->side/2 - SPRITE_UP_H;
                            bullets[i].vx = 0;
                            bullets[i].vy = -15;
                        } else if (key[ALLEGRO_KEY_DOWN] && no_chao) {
                            if (direcao == 0) bullets[i].x = player_boss->x + SPRITE_DOWN_W/2;
                            else bullets[i].x = player_boss->x - SPRITE_DOWN_W/2;
                            bullets[i].y = player_boss->y + player_boss->side/2 - SPRITE_DOWN_H/2 + 40;
                            bullets[i].vx = (direcao == 0) ? 15 : -15; bullets[i].vy = 0;
                        } else {
                            if (direcao == 0) bullets[i].x = player_boss->x + SPRITE_W/2;
                            else bullets[i].x = player_boss->x - SPRITE_W/2;
                            bullets[i].y = player_boss->y + player_boss->side/2 - SPRITE_H -5;
                            bullets[i].vx = (direcao == 0) ? 15 : -15; bullets[i].vy = 0;
                        }
                        break;
                    }
                }
            }
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (bullets[i].ativa) {
                    bullets[i].x += bullets[i].vx;
                    bullets[i].y += bullets[i].vy;
                    al_draw_bitmap(bullet_boss_img, bullets[i].x - BULLET_BOSS_W/2, bullets[i].y, 0);
                }
            }

            stamina_recupera_tick++;
            if (stamina_recupera_tick > 5) { // a cada 6 iterações do loop, recupera 1 ponto de stamina
                stamina_recupera_tick = 0;
                if (stamina < stamina_max) stamina++;
            }
            // HUD do player
            int vida_max = 20, num_coracoes = vida_max / 2;
            for (int i = 0; i < num_coracoes; i++) {
                int tipo = (vida_boss >= (i+1)*2) ? 0 : (vida_boss == (i*2)+1) ? 2 : 1;
                al_draw_bitmap_region(coracao_sprite, tipo * CORACAO_W, 0, CORACAO_W, CORACAO_H,
                    20 + i * (CORACAO_W + 5), 20, 0);
            }

            if (vida_boss <= 0) {
                tela_gameover(disp, gameover_img);
                boss_running = false;
                state = MENU;
                continue;
            }

            // --------- LÓGICA DO BOSS ---------
            // --- BLOQUEIO ABSOLUTO DO DESFAZENDO/MORTO ---
            if (boss.estado == BOSS_DESFAZENDO) {
                boss.vida = 3;
                boss_desfazendo_timer--;
                al_draw_scaled_bitmap(boss_desfazendo_sprite, 0, 0, BOSS_DESFAZENDO_W, BOSS_DESFAZENDO_H,
                    boss.x, boss.y - BOSS_DESFAZENDO_H - 80, BOSS_DESFAZENDO_W * 2, BOSS_DESFAZENDO_H * 2, 0);
                int vida_col = boss_bar_col(boss);
                int vida_row = boss_bar_row(boss);
                al_draw_bitmap_region(boss_vida_sprite, vida_col * BOSS_VIDA_W, vida_row * BOSS_VIDA_H, BOSS_VIDA_W, BOSS_VIDA_H,
                    X_SCREEN - BOSS_VIDA_W - 20, 20, 0);
                al_flip_display();
                al_rest(0.01);
                if (boss_desfazendo_timer <= 0) {
                    boss.estado = BOSS_MORTO;
                    boss.vida = 0;
                }
                continue;
            }
            if (boss.estado == BOSS_MORTO) {
                al_draw_scaled_bitmap(boss_morto_sprite, 0, 0, BOSS_MORTO_W, BOSS_MORTO_H,
                    boss.x, boss.y - BOSS_MORTO_H - 80, BOSS_MORTO_W * 2, BOSS_MORTO_H * 2, 0);
                int vida_col = boss_bar_col(boss);
                int vida_row = boss_bar_row(boss);
                al_draw_bitmap_region(boss_vida_sprite, vida_col * BOSS_VIDA_W, vida_row * BOSS_VIDA_H, BOSS_VIDA_W, BOSS_VIDA_H,
                    X_SCREEN - BOSS_VIDA_W - 20, 20, 0);
                al_flip_display();
                al_rest(2.0);

                // Tela de vitória
                tela_vitoria(disp, victory_img);

                boss_running = false;
                state = FASE3;
                continue;
            }

            // --- TRANSIÇÃO PARA DESFAZENDO ---
            if (boss.vida <= 3 && boss.estado != BOSS_DESFAZENDO && boss.estado != BOSS_MORTO) {
                boss.estado = BOSS_DESFAZENDO;
                boss.vida = 3;
                boss_desfazendo_timer = 120;
                continue;
            }

            // --- CICLO NORMAL DO BOSS ---
            if (boss.estado == BOSS_PARADO) {
                if (boss_state_timer == 0) boss_state_timer = 120;
                boss_state_timer--;
                if (boss_state_timer <= 0) { boss.estado = BOSS_ESCUDO; boss_state_timer = 0; }
            } else if (boss.estado == BOSS_ESCUDO) {
                if (boss_state_timer == 0) boss_state_timer = 120;
                boss_state_timer--;
                if (boss_state_timer <= 0) { boss.estado = BOSS_ANDANDO; boss_state_timer = 0; passos_dados = 0; boss_andando_bola_timer = 0; }
            } else if (boss.estado == BOSS_ANDANDO) {
                boss_andando_bola_timer++;
                if (boss_andando_bola_timer >= 32) {
                    for (int i = 0; i < MAX_BOLAS_FOGO; i++) {
                        if (!bolas_fogo[i].ativa) {
                            bolas_fogo[i].ativa = 1;
                            bolas_fogo[i].x = boss.x - 20;
                            bolas_fogo[i].y = boss.y - 180;
                            bolas_fogo[i].vx = -8;
                            bolas_fogo[i].vy = 0;
                            break;
                        }
                    }
                    boss_andando_bola_timer = 0;
                }
                if (passos_dados < 2) {
                    if (boss_state_timer == 0) boss_state_timer = 30;
                    boss_state_timer--;
                    if (boss_state_timer <= 0) {
                        boss.x -= 50;
                        passos_dados++;
                        boss_state_timer = 30;
                    }
                } else {
                    boss.estado = BOSS_PARADO;
                    boss_state_timer = 0;
                }
            } else if (boss.estado == BOSS_DANO) {
                if (boss_dano_timer == 0) boss_dano_timer = 240;
                boss_dano_timer--;
                if (boss_dano_timer <= 0) {
                    boss.estado = BOSS_ESCUDO;
                    boss_state_timer = 0;
                    boss_dano_timer = 0;
                }
            }

            // --- DESENHO DOS SPRITES DO BOSS NORMAL ---
            float boss_draw_y = boss.y - (BOSS_FRAME_H * 2);
            if (boss.estado == BOSS_ATACANDO) {
                al_draw_scaled_bitmap(boss_atacando_sprite, 0, 0, BOSS_ATACANDO_W, BOSS_ATACANDO_H,
                    boss.x, boss_draw_y + 70, BOSS_ATACANDO_W * 2, BOSS_ATACANDO_H * 2, 0);
            } else if (boss.estado == BOSS_ANDANDO) {
                al_draw_scaled_bitmap(boss_andando_sprite, 0, 0, BOSS_ANDANDO_W, BOSS_ANDANDO_H,
                    boss.x, boss_draw_y + 70, BOSS_ANDANDO_W * 2, BOSS_ANDANDO_H * 2, 0);
            } else if (boss.estado == BOSS_DANO) {
                al_draw_scaled_bitmap(boss_dano_sprite, 0, 0, BOSS_DANO_W, BOSS_DANO_H,
                    boss.x, boss_draw_y + 70, BOSS_DANO_W * 2, BOSS_DANO_H * 2, 0);
            } else if (boss.estado == BOSS_PARADO || boss.estado == BOSS_ESCUDO) {
                int boss_sprite_col = boss.estado == BOSS_ESCUDO ? 0 : 1;
                int boss_sprite_row = 0;
                al_draw_scaled_bitmap(boss_sprite, boss_sprite_col * BOSS_FRAME_W, boss_sprite_row * BOSS_FRAME_H, BOSS_FRAME_W, BOSS_FRAME_H,
                    boss.x, boss_draw_y, BOSS_FRAME_W * 2, BOSS_FRAME_H * 2, 0);
            }

            // --- BARRA DE VIDA CORRETA ---
            int vida_col = boss_bar_col(boss);
            int vida_row = boss_bar_row(boss);
            al_draw_bitmap_region(boss_vida_sprite, vida_col * BOSS_VIDA_W, vida_row * BOSS_VIDA_H, BOSS_VIDA_W, BOSS_VIDA_H,
                X_SCREEN - BOSS_VIDA_W - 20, 20, 0);

            // --- COLISÃO DOS TIROS ---
            if (boss.estado == BOSS_ANDANDO || boss.estado == BOSS_DANO) {
                float boss_left   = boss.x;
                float boss_right  = boss.x + (BOSS_FRAME_W * 2);
                float boss_top    = boss.y - (BOSS_FRAME_H * 2);
                float boss_bottom = boss.y;
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (bullets[i].ativa) {
                        float bullet_left   = bullets[i].x - BULLET_BOSS_W/2;
                        float bullet_right  = bullets[i].x + BULLET_BOSS_W/2;
                        float bullet_top    = bullets[i].y;
                        float bullet_bottom = bullets[i].y + BULLET_BOSS_H;
                        if (bullet_right > boss_left && bullet_left < boss_right &&
                            bullet_bottom > boss_top && bullet_top < boss_bottom) {

                            // Se NÃO está em BOSS_DANO, conte hits e aplique dano
                            if (boss.estado != BOSS_DANO) {
                                boss_hit_counter++;
                                if (boss_hit_counter == 1) {
                                    boss_hit_counter = 0;
                                    if (boss.vida > 3) {
                                        boss.vida--;
                                        boss.estado = BOSS_DANO;
                                        boss_dano_timer = 0;
                                        boss_state_timer = 0;
                                    }
                                }
                            }
                            // Se está em BOSS_DANO, não faz nada (ignora)
                            bullets[i].ativa = 0; // sempre desativa o tiro
                        }
                    }
                }
            }
            // Fora do loop: quando está em BOSS_DANO, zere o contador para não acumular
            if (boss.estado == BOSS_DANO) {
                boss_hit_counter = 0;
            }

            // --- BOLAS DE FOGO DO BOSS ---
            for (int i = 0; i < MAX_BOLAS_FOGO; i++) {
                if (bolas_fogo[i].ativa) {
                    bolas_fogo[i].x += bolas_fogo[i].vx;
                    bolas_fogo[i].y += bolas_fogo[i].vy;
                    al_draw_bitmap(bola_fogo_sprite, bolas_fogo[i].x, bolas_fogo[i].y, 0);
                    if (bolas_fogo[i].x < -BOLA_FOGO_W || bolas_fogo[i].x > X_SCREEN + BOLA_FOGO_W) {
                        bolas_fogo[i].ativa = 0;
                    }
                }
            }
            // --- COLISÃO DAS BOLAS DE FOGO COM O PLAYER ---
            float player_left   = player_boss->x - SPRITE_W/2;
            float player_right  = player_boss->x + SPRITE_W/2;
            float player_top    = player_boss->y + player_boss->side/2 - SPRITE_H;
            float player_bottom = player_boss->y + player_boss->side/2;
            for (int i = 0; i < MAX_BOLAS_FOGO; i++) {
                if (bolas_fogo[i].ativa) {
                    float bola_left   = bolas_fogo[i].x;
                    float bola_right  = bolas_fogo[i].x + BOLA_FOGO_W;
                    float bola_top    = bolas_fogo[i].y;
                    float bola_bottom = bolas_fogo[i].y + BOLA_FOGO_H;
                    if (bola_right > player_left && bola_left < player_right &&
                        bola_bottom > player_top && bola_top < player_bottom) {
                        vida_boss -= 4;
                        bolas_fogo[i].ativa = 0;
                    }
                }
            }

             // BLOCO DE RECUPERAÇÃO/CANSAÇO DA ESTAMINA - coloque aqui:
            if (cansado) {
                stamina_fadiga_tick++;
                if (stamina_fadiga_tick > 60) { // espera 60 frames (~1 segundo)
                    cansado = false;
                    stamina = 0;
                    stamina_recupera_tick = 0;
                }
            } else {
                stamina_recupera_tick++;
                if (stamina_recupera_tick > 5) {
                    stamina_recupera_tick = 0;
                    if (stamina < stamina_max) stamina++;
                }
            }
            // DESENHA BARRA DE ESTAMINA
            int bar_w = 200;
            int bar_h = 20;
            float perc = (float)stamina / stamina_max;
            al_draw_filled_rectangle(20, 60, 20 + bar_w * perc, 60 + bar_h, al_map_rgb(0,200,0));
            al_draw_rectangle(20, 60, 20 + bar_w, 60 + bar_h, al_map_rgb(0,0,0), 2);
            al_flip_display();
            al_rest(0.01);
        }
            square_destroy(player_boss);
            continue;
        }
        // ----- FASE 3 -----
        // ----- FASE 3 -----
        if (state == FASE3) {
            // Inicialização da fase 3
            int vida_fase3 = 20; // vida do chefe 2 (ajuste conforme seu chefe)
            bool fase3_running = true;
            bool key[ALLEGRO_KEY_MAX] = {false};
            int vida_player_fase3 = 20;

            // Player na posição inicial
            square* player_fase3 = square_create(50, 50, Y_SCREEN-40, X_SCREEN, Y_SCREEN);
            float vel_y_fase3 = 0;
            bool no_chao_fase3 = false;
            int direcao_fase3 = 0;
            int altura_colisao_fase3 = SPRITE_H;

            while (fase3_running) {
                // Eventos
                ALLEGRO_EVENT event;
                if (al_get_next_event(queue, &event)) {
                    if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                        fase3_running = false;
                        state = EXIT;
                    } else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                        key[event.keyboard.keycode] = true;
                    } else if (event.type == ALLEGRO_EVENT_KEY_UP) {
                        key[event.keyboard.keycode] = false;
                    }
                }

                // --- Desenha o background noturno ---
                al_draw_scaled_bitmap(bg_night, 0, 0, 2048, 1536, 0, 0, X_SCREEN, Y_SCREEN, 0);

                // --- MOVIMENTO DO PLAYER (igual outras fases) ---
                if (key[ALLEGRO_KEY_LEFT]) { square_move(player_fase3, 1, 0, X_SCREEN, Y_SCREEN); direcao_fase3 = 1; }
                if (key[ALLEGRO_KEY_RIGHT]) { square_move(player_fase3, 1, 1, X_SCREEN, Y_SCREEN); direcao_fase3 = 0; }
                if ((key[ALLEGRO_KEY_SPACE] || key[ALLEGRO_KEY_UP]) && no_chao_fase3) { vel_y_fase3 = -20; no_chao_fase3 = false; }
                if (!no_chao_fase3) {
                    player_fase3->y += vel_y_fase3;
                    vel_y_fase3 += 1.5;
                    if (player_fase3->y + player_fase3->side/2 >= Y_SCREEN - 90) {
                        player_fase3->y = Y_SCREEN - 90 - player_fase3->side/2;
                        vel_y_fase3 = 0; no_chao_fase3 = true;
                    }
                }

                // --- SPRITE DO PLAYER (igual outras fases) ---
                int sprite_row = 0, sprite_col = 0;
                if (key[ALLEGRO_KEY_UP] && key[ALLEGRO_KEY_Z]) {
                    int up_col = (direcao_fase3 == 0) ? 1 : 0;
                    al_draw_bitmap_region(sprite_up, up_col * SPRITE_UP_W, 0, SPRITE_UP_W, SPRITE_UP_H,
                        player_fase3->x - SPRITE_UP_W/2,
                        player_fase3->y + player_fase3->side/2 - SPRITE_UP_H, 0);
                } else if (key[ALLEGRO_KEY_DOWN] && key[ALLEGRO_KEY_Z] && no_chao_fase3) {
                    altura_colisao_fase3 = SPRITE_H * 0.5;
                    int down_col = (direcao_fase3 == 0) ? 1 : 0;
                    al_draw_bitmap_region(sprite_down, down_col * SPRITE_DOWN_W, 0, SPRITE_DOWN_W, SPRITE_DOWN_H,
                        player_fase3->x - SPRITE_DOWN_W/2,
                        player_fase3->y + player_fase3->side/2 - SPRITE_DOWN_H, 0);
                } else {
                    if (key[ALLEGRO_KEY_DOWN] && no_chao_fase3) { altura_colisao_fase3 = SPRITE_H * 0.5; sprite_row = 1; sprite_col = (direcao_fase3 == 0) ? 0 : 3; }
                    else if (key[ALLEGRO_KEY_Z]) { sprite_row = 2; sprite_col = (direcao_fase3 == 0) ? 1 : 2; }
                    else if (!no_chao_fase3) { sprite_row = 0; sprite_col = (direcao_fase3 == 0) ? 1 : 2; }
                    else if ((key[ALLEGRO_KEY_LEFT] || key[ALLEGRO_KEY_RIGHT]) && no_chao_fase3) { sprite_row = 1; sprite_col = (direcao_fase3 == 0) ? 1 : 2; }
                    else if (no_chao_fase3) { sprite_row = 0; sprite_col = (direcao_fase3 == 0) ? 0 : 3; }
                    al_draw_bitmap_region(sprite_sheet, sprite_col * SPRITE_W, sprite_row * SPRITE_H, SPRITE_W, SPRITE_H,
                        player_fase3->x - SPRITE_W/2,
                        player_fase3->y + player_fase3->side/2 - SPRITE_H, 0);
                }

                static double last_shot_time_fase3 = 0;
                double now_fase3 = al_get_time();
                double shot_delay_fase3 = 0.15; // igual chefão

                // Disparo do projetil (igual chefão)
                if (key[ALLEGRO_KEY_Z] && now_fase3 - last_shot_time_fase3 > shot_delay_fase3) {
                    last_shot_time_fase3 = now_fase3;

                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (!bullets[i].ativa) {
                            bullets[i].ativa = 1;
                            // Para cima
                            if (key[ALLEGRO_KEY_UP]) {
                                bullets[i].x = player_fase3->x + 10;
                                bullets[i].y = player_fase3->y + player_fase3->side/2 - SPRITE_UP_H;
                                bullets[i].vx = 0;
                                bullets[i].vy = -15;
                            }
                            // Para baixo
                            else if (key[ALLEGRO_KEY_DOWN] && no_chao_fase3) {
                                if (direcao_fase3 == 0) bullets[i].x = player_fase3->x + SPRITE_DOWN_W/2;
                                else bullets[i].x = player_fase3->x - SPRITE_DOWN_W/2;
                                bullets[i].y = player_fase3->y + player_fase3->side/2 - SPRITE_DOWN_H/2 + 40;
                                bullets[i].vx = (direcao_fase3 == 0) ? 15 : -15;
                                bullets[i].vy = 0;
                            }
                            // Para frente
                            else {
                                if (direcao_fase3 == 0) bullets[i].x = player_fase3->x + SPRITE_W/2;
                                else bullets[i].x = player_fase3->x - SPRITE_W/2;
                                bullets[i].y = player_fase3->y + player_fase3->side/2 - SPRITE_H + 20;
                                bullets[i].vx = (direcao_fase3 == 0) ? 15 : -15;
                                bullets[i].vy = 0;
                            }
                            break;
                        }
                    }
                }

                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (bullets[i].ativa) {
                        bullets[i].x += bullets[i].vx;
                        bullets[i].y += bullets[i].vy;
                        al_draw_bitmap(bullet_boss_img, bullets[i].x - BULLET_BOSS_W/2, bullets[i].y, 0);

                        // Desativa se sair da tela
                        if (bullets[i].x < -BULLET_BOSS_W || bullets[i].x > X_SCREEN + BULLET_BOSS_W ||
                            bullets[i].y < -BULLET_BOSS_H || bullets[i].y > Y_SCREEN + BULLET_BOSS_H) {
                            bullets[i].ativa = 0;
                        }
                    }
                }
                // --- CONTROLE DE PAUSA/SAIR ---
                if (key[ALLEGRO_KEY_ESCAPE]) {
                    fase3_running = false;
                    state = MENU;
                }

                // --- HUD (exemplo de barra de vida do chefe 2 e do player) ---
                char vida_str[32];
                sprintf(vida_str, "Vida Chefe 2: %d", vida_fase3);
                al_draw_text(font, al_map_rgb(128,128,255), 20, 20, 0, vida_str);

                char vida_player_str[32];
                sprintf(vida_player_str, "Vida: %d", vida_player_fase3);
                al_draw_text(font, al_map_rgb(255,0,0), 20, 50, 0, vida_player_str);

                al_flip_display();
                al_rest(0.01);
            }
            square_destroy(player_fase3);
            continue;
        }
    }
    

    al_destroy_bitmap(fogo_sprite);
    al_destroy_bitmap(bg_menu);
    al_destroy_bitmap(bullet_img);
    al_destroy_bitmap(sprite_up);
    al_destroy_bitmap(sprite_down);
    al_destroy_bitmap(sprite_sheet);
    al_destroy_bitmap(bg);
    al_destroy_font(font);
    al_destroy_display(disp);
    al_destroy_event_queue(queue);
    al_destroy_bitmap(chama_sprite);
    al_destroy_bitmap(coracao_sprite);
    al_destroy_bitmap(bg_boss);
    al_destroy_bitmap(boss_unlocked_img);
    al_destroy_bitmap(bullet_boss_img);
    al_destroy_bitmap(boss_vida_sprite);
    al_destroy_bitmap(boss_dano_sprite);
    al_destroy_bitmap(boss_atacando_sprite);
    al_destroy_bitmap(boss_andando_sprite);
    al_destroy_bitmap(boss_desfazendo_sprite);
    al_destroy_bitmap(boss_morto_sprite);
    al_destroy_bitmap(gameover_img);
    al_destroy_bitmap(victory_img);
    al_destroy_bitmap(pause_img);
    return 0;
}