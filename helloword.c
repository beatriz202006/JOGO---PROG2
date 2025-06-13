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

typedef enum { MENU, GAME, BOSS, EXIT } GameState;

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

struct Boss {
    float x, y;
    int estado; // 0=escudo, 1=parado, 2=atacando, 3=andando, 4=desfazendo, 5=dano, 6=morto
    int vida;
    int cooldown_ataque;
    int cooldown_escudo;
    int frame_atual;
};

struct BolaFogo {
    float x, y;
    float vx, vy;
    int ativa;
};

struct BolaFogo bolas_fogo[MAX_BOLAS_FOGO] = {0};

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
    ALLEGRO_BITMAP* sprite_sheet = al_load_bitmap("spritesagua.png");
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

    // Carrega a imagem da bala
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

    al_convert_mask_to_alpha(sprite_sheet, al_map_rgb(0,0,0));
    int BULLET_W = 22;
    int BULLET_H = 33;

    GameState state = MENU;
    int menu_option = 0; // 0 = Iniciar, 1 = Sair

    // Carrega a spritesheet do inimigo
    ALLEGRO_BITMAP* fogo_sprite = al_load_bitmap("spritesfogofull.png");
    al_convert_mask_to_alpha(fogo_sprite, al_map_rgb(0,0,0));
    if (!fogo_sprite) {
        printf("Erro ao carregar sprite do inimigo!\n");
    }
    int fogo_sprite_w = al_get_bitmap_width(fogo_sprite);
    int fogo_sprite_h = al_get_bitmap_height(fogo_sprite);
    int fogo_frame_w = fogo_sprite_w / 4; // 4 sprites na linha
    int fogo_frame_h = fogo_sprite_h;     // só 1 linha

    int plataforma_y = Y_SCREEN - 265; // chão
    int plat_x = 400;
    int plat_w = 400;
    int plat_y = 750;
    int plat_h = 20;

    // Inicializa os 6 inimigos
    float escala = 0.4;
    for (int i = 0; i < NUM_FOGOS; i++) {
        fogos[i].x = X_SCREEN + i * 1800; // 1800 pixels de distância entre cada fogo
        fogos[i].y = plat_y - fogo_frame_h * escala;
        fogos[i].direcao = 1;
        fogos[i].velocidade = 1;
        fogos[i].frame = 2;
        fogos_vida[i] = 2;
        fogos_respawn_timer[i] = i * 900;
        fogo_derrotado[i] = 0;
    }

    // Carrega a spritesheet da chama
    ALLEGRO_BITMAP* chama_sprite = al_load_bitmap("chama2.png");
    if (!chama_sprite) {
        printf("Erro ao carregar sprite da chama!\n");
    }
    int CHAMA_W = 46;
    int CHAMA_H = 33;

    // Carrega a spritesheet dos corações
    ALLEGRO_BITMAP* coracao_sprite = al_load_bitmap("vida.png");
    al_convert_mask_to_alpha(coracao_sprite, al_map_rgb(0,0,0));
    if (!coracao_sprite) {
        printf("Erro ao carregar sprite dos corações!\n");
    }
    int CORACAO_W = 100 / 3; // 3 sprites na horizontal
    int CORACAO_H = 35;

    int vida = 20;
    bool rodada_fogos_acabou = false;

    // Carrega a imagem de fundo do menu
    ALLEGRO_BITMAP* bg_menu = al_load_bitmap("menu.png");
    if (!bg_menu) {
        printf("Erro ao carregar background do menu!\n");
    }

    int dano_fogo_cooldown = 0;

    // Carrega a imagem de fundo do chefe
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

    // Carrega a imagem da bola de fogo
    ALLEGRO_BITMAP* bola_fogo_sprite = al_load_bitmap("boladefogo2.png"); // ou o nome do seu arquivo
    if (!bola_fogo_sprite) {
        printf("Erro ao carregar sprite da bola de fogo!\n");
    }
    int BOLA_FOGO_W = 77;
    int BOLA_FOGO_H = 78;

    while (state != EXIT) {
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
            }
        }

        if (state == GAME) {
            // Inicializa os 6 inimigos para cada nova rodada!
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
                fogo_morto_por_tiro[i] = 0; // <-- adicione aqui!
            }

            // REINICIE A VIDA AQUI!
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

                // Pulo
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

                if (key[ALLEGRO_KEY_ESCAPE]) {
                    jogando = false;
                    state = MENU; // Volta para o menu ao sair do loop
                }

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
                     altura_colisao = SPRITE_H * 0.5;
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
                        altura_colisao = SPRITE_H * 0.5;
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

                // Controle de cooldown para tiro contínuo
                static double last_shot_time = 0;
                double now = al_get_time();
                double shot_delay = 0.15; // segundos entre tiros (ajuste como quiser)

                if (key[ALLEGRO_KEY_Z] && now - last_shot_time > shot_delay) {
                    last_shot_time = now;
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (!bullets[i].ativa) {
                            bullets[i].ativa = 1;

                            // Tiro para cima
                            if (key[ALLEGRO_KEY_UP]) {
                                bullets[i].x = player->x + 10;
                                bullets[i].y = player->y + player->side/2 - SPRITE_UP_H;
                                bullets[i].vx = 0;
                                bullets[i].vy = -15;
                            }
                            // Tiro abaixado
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
                            // Tiro normal (horizontal)
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
                        al_draw_bitmap(bullet_img, bullets[i].x - BULLET_W/2, bullets[i].y, 0);
                    }
                }

                // Bounding box do player
                float player_left   = player->x - SPRITE_W/2;
                float player_right  = player->x + SPRITE_W/2;
                float player_top    = player->y + player->side/2 - SPRITE_H;
                float player_bottom = player->y + player->side/2;

                // --- Lógica, desenho e colisão dos inimigos ---
                passo++;
                for (int f = 0; f < NUM_FOGOS; f++) {
                    // Se o fogo saiu da tela e ainda não acabou a rodada, mata e inicia timer de respawn
                    if (fogos_vida[f] > 0 && fogos[f].x < -fogo_frame_w * escala && !rodada_fogos_acabou) {
                        fogos_vida[f] = 0;
                        fogos_respawn_timer[f] = 100 + f * 20;
                        fogos[f].x = -1000;
                        fogo_derrotado[f] = 0;         // <-- Adicione esta linha!
                        fogo_morto_por_tiro[f] = 0;    // <-- E esta!
                    }

                    // Se o fogo está morto e ainda não acabou a rodada, faz o respawn
                    if (fogos_vida[f] == 0 && fogos_respawn_timer[f] > 0 && !rodada_fogos_acabou && !fogo_morto_por_tiro[f]) {
                        fogos_respawn_timer[f]--;
                        if (fogos_respawn_timer[f] == 0) {
                            fogos[f].x = X_SCREEN + rand() % 200;
                            fogos_vida[f] = 2;
                            chama_timer[f] = -40;
                            fogo_derrotado[f] = 0;         // <-- Adicione esta linha!
                            fogo_morto_por_tiro[f] = 0;    // <-- E esta!
                        }
                        chama_timer[f] = 0;
                        continue;
                    }

                    // Se acabou a rodada, nunca mais respawna
                    if (fogos_vida[f] <= 0 && rodada_fogos_acabou) {
                        fogos[f].x = -1000;
                        chama_timer[f] = 0;
                        continue;
                    }

                    // Se está morto, não faz nada
                    if (fogos_vida[f] <= 0) {
                        chama_timer[f] = 0;
                        continue;
                    }

                    // Movimento e animação
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

                    // Colisão com player
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

                    // Colisão com balas
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

                    // Lançar chamas (só se o fogo está vivo)
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

                // --- Desenhar e atualizar as chamas ---
                for (int i = 0; i < MAX_CHAMAS; i++) {
                    if (chamas[i].ativa) {
                        chamas[i].x += chamas[i].vx;
                        chamas[i].y += chamas[i].vy;
                        al_draw_bitmap(chama_sprite, chamas[i].x, chamas[i].y, 0);

                        // Desativa se sair da tela
                        if (chamas[i].x < -CHAMA_W || chamas[i].x > X_SCREEN + CHAMA_W ||
                            chamas[i].y < -CHAMA_H || chamas[i].y > Y_SCREEN + CHAMA_H) {
                            chamas[i].ativa = 0;
                        }

                        // Colisão com o player
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
                            vida -= 1; // meio coração
                            chamas[i].ativa = 0; // desativa a chama após causar dano
                        }
                    }
                }

                // --- HUD e fim de jogo ---
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
                    jogando = false;
                }

                // Se todos os fogos estão mortos E todos foram mortos por tiro, avance para o chefe!
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
                    state = BOSS; // Troca para a fase do chefão
                    jogando = false;
                }

                if (dano_fogo_cooldown > 0) dano_fogo_cooldown--;

                al_flip_display();
                al_rest(0.01);
            }

            square_destroy(player);
        }

        if (state == BOSS) {
            // Tela de transição: "Chefão desbloqueado!"
            if (boss_unlocked_img) {
                al_draw_scaled_bitmap(boss_unlocked_img, 0, 0, 2048, 1536, 0, 0, X_SCREEN, Y_SCREEN, 0);
            } else {
                al_clear_to_color(al_map_rgb(0, 0, 0));
                al_draw_text(font, al_map_rgb(255, 255, 0), X_SCREEN/2, Y_SCREEN/2, ALLEGRO_ALIGN_CENTRE, "Chefão desbloqueado!");
            }
            al_flip_display();
            al_rest(2.0); // mostra por 2 segundos

            // Cria o player para a fase do boss
            square* player_boss = square_create(50, 100, Y_SCREEN - 100, X_SCREEN, Y_SCREEN);
            bool boss_running = true;
            bool key[ALLEGRO_KEY_MAX] = {false};
            float vel_y = 0;
            bool no_chao = false;
            int direcao = 0;
            int altura_colisao = SPRITE_H;
            int vida_boss = 20; // Exemplo de vida do player na fase do boss

            // Antes do loop do boss:
            struct Boss boss;
            boss.x = X_SCREEN - 350; // canto direito da tela
            boss.y = Y_SCREEN - 10; // alinhado ao chão do boss
            boss.estado = 1; // parado
            boss.vida = 20;
            boss.cooldown_ataque = 0;
            boss.cooldown_escudo = 0;
            boss.frame_atual = 0;

            while (boss_running) {
                // Eventos
                ALLEGRO_EVENT event;
                if (al_get_next_event(queue, &event)) {
                    if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
                        boss_running = false;
                    else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
                        key[event.keyboard.keycode] = true;
                    else if (event.type == ALLEGRO_EVENT_KEY_UP)
                        key[event.keyboard.keycode] = false;
                }

                // Movimento do player (horizontal)
                if (key[ALLEGRO_KEY_LEFT]) {
                    square_move(player_boss, 1, 0, X_SCREEN, Y_SCREEN);
                    direcao = 1;
                }
                if (key[ALLEGRO_KEY_RIGHT]) {
                    square_move(player_boss, 1, 1, X_SCREEN, Y_SCREEN);
                    direcao = 0;
                }

                // Pulo
                if ((key[ALLEGRO_KEY_SPACE] || key[ALLEGRO_KEY_UP]) && no_chao) {
                    vel_y = -20;
                    no_chao = false;
                }

                // Gravidade e chão do player (VOLTE PARA COMO ESTAVA ANTES)
                if (!no_chao) {
                    player_boss->y += vel_y;
                    vel_y += 1.5;
                    if (player_boss->y + player_boss->side/2 >= Y_SCREEN - 90) { // chão do boss
                        player_boss->y = Y_SCREEN - 90 - player_boss->side/2;
                        vel_y = 0;
                        no_chao = true;
                    }
                }

                if (key[ALLEGRO_KEY_ESCAPE]) {
                    boss_running = false;
                    state = MENU;
                }

                // Desenha o background do boss
                al_draw_scaled_bitmap(bg_boss, 0, 0, 2048, 1536, 0, -100, X_SCREEN, Y_SCREEN + 100, 0);

                // --- Desenhar o personagem (igual à fase normal) ---
                int sprite_row = 0, sprite_col = 0;
                if (key[ALLEGRO_KEY_UP] && key[ALLEGRO_KEY_Z]) {
                    int up_col = (direcao == 0) ? 1 : 0;
                    al_draw_bitmap_region(
                        sprite_up,
                        up_col * SPRITE_UP_W, 0,
                        SPRITE_UP_W, SPRITE_UP_H,
                        player_boss->x - SPRITE_UP_W/2,
                        player_boss->y + player_boss->side/2 - SPRITE_UP_H,
                        0
                    );
                } else if (key[ALLEGRO_KEY_DOWN] && key[ALLEGRO_KEY_Z] && no_chao) {
                    altura_colisao = SPRITE_H * 0.5;
                    int down_col = (direcao == 0) ? 1 : 0;
                    al_draw_bitmap_region(
                        sprite_down,
                        down_col * SPRITE_DOWN_W, 0,
                        SPRITE_DOWN_W, SPRITE_DOWN_H,
                        player_boss->x - SPRITE_DOWN_W/2,
                        player_boss->y + player_boss->side/2 - SPRITE_DOWN_H,
                        0
                    );
                } else {
                    if (key[ALLEGRO_KEY_DOWN] && no_chao) {
                        altura_colisao = SPRITE_H * 0.5;
                        sprite_row = 1;
                        sprite_col = (direcao == 0) ? 0 : 3;
                    } else if (key[ALLEGRO_KEY_Z]) {
                        sprite_row = 2;
                        sprite_col = (direcao == 0) ? 1 : 2;
                    } else if (!no_chao) {
                        sprite_row = 0;
                        sprite_col = (direcao == 0) ? 1 : 2;
                    } else if ((key[ALLEGRO_KEY_LEFT] || key[ALLEGRO_KEY_RIGHT]) && no_chao) {
                        sprite_row = 1;
                        sprite_col = (direcao == 0) ? 1 : 2;
                    } else if (no_chao) {
                        sprite_row = 0;
                        sprite_col = (direcao == 0) ? 0 : 3;
                    }
                    al_draw_bitmap_region(
                        sprite_sheet,
                        sprite_col * SPRITE_W, sprite_row * SPRITE_H,
                        SPRITE_W, SPRITE_H,
                        player_boss->x - SPRITE_W/2,
                        player_boss->y + player_boss->side/2 - SPRITE_H,
                        0
                    );
                }

                // Controle de cooldown para tiro contínuo
                static double last_shot_time_boss = 0;
                double now_boss = al_get_time();
                double shot_delay_boss = 0.15;
                if (key[ALLEGRO_KEY_Z] && now_boss - last_shot_time_boss > shot_delay_boss) {
                    last_shot_time_boss = now_boss;
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (!bullets[i].ativa) {
                            bullets[i].ativa = 1;
                            if (key[ALLEGRO_KEY_UP]) {
                                bullets[i].x = player_boss->x + 10;
                                bullets[i].y = player_boss->y + player_boss->side/2 - SPRITE_UP_H;
                                bullets[i].vx = 0;
                                bullets[i].vy = -15;
                            } else if (key[ALLEGRO_KEY_DOWN] && no_chao) {
                                if (direcao == 0) {
                                    bullets[i].x = player_boss->x + SPRITE_DOWN_W/2;
                                } else {
                                    bullets[i].x = player_boss->x - SPRITE_DOWN_W/2;
                                }
                                bullets[i].y = player_boss->y + player_boss->side/2 - SPRITE_DOWN_H/2 + 40;
                                bullets[i].vx = (direcao == 0) ? 15 : -15;
                                bullets[i].vy = 0;
                            } else {
                                if (direcao == 0) {
                                    bullets[i].x = player_boss->x + SPRITE_W/2;
                                } else {
                                    bullets[i].x = player_boss->x - SPRITE_W/2;
                                }
                                bullets[i].y = player_boss->y + player_boss->side/2 - SPRITE_H + 20;
                                bullets[i].vx = (direcao == 0) ? 15 : -15;
                                bullets[i].vy = 0;
                            }
                            break;
                        }
                    }
                }

                // Atualiza e desenha as balas
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (bullets[i].ativa) {
                        bullets[i].x += bullets[i].vx;
                        bullets[i].y += bullets[i].vy;
                        al_draw_bitmap(bullet_img, bullets[i].x - BULLET_W/2, bullets[i].y, 0);
                    }
                }

                // --- HUD do player ---
                char vida_str[32];
                sprintf(vida_str, "Vida: %d", vida_boss);
                al_draw_text(font, al_map_rgb(255,0,0), 20, 20, 0, vida_str);

                int vida_max = 20;
                int num_coracoes = vida_max / 2;
                for (int i = 0; i < num_coracoes; i++) {
                    int tipo;
                    if (vida_boss >= (i+1)*2) {
                        tipo = 0;
                    } else if (vida_boss == (i*2)+1) {
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

                int boss_sprite_col = 0, boss_sprite_row = 0;
                switch (boss.estado) {
                    case 0: boss_sprite_row = 0; boss_sprite_col = 0; break; // escudo
                    case 1: boss_sprite_row = 0; boss_sprite_col = 1; break; // parado
                    case 2: boss_sprite_row = 1; boss_sprite_col = 0; break; // atacando
                    case 3: boss_sprite_row = 1; boss_sprite_col = 1; break; // andando
                    case 4: boss_sprite_row = 2; boss_sprite_col = 0; break; // desfazendo
                    case 5: boss_sprite_row = 2; boss_sprite_col = 1; break; // dano
                    case 6: boss_sprite_row = 3; boss_sprite_col = 0; break; // morto
                }

                float boss_draw_y = boss.y - (BOSS_FRAME_H * 2);
                if (boss.estado == 2) { // atacando
                    boss_draw_y += 60; // ajuste este valor
                }

                al_draw_scaled_bitmap(
                    boss_sprite,
                    boss_sprite_col * BOSS_FRAME_W, boss_sprite_row * BOSS_FRAME_H,
                    BOSS_FRAME_W, BOSS_FRAME_H,
                    boss.x,
                    boss_draw_y, // ou simplesmente Y_SCREEN - 200
                    BOSS_FRAME_W * 2,
                    BOSS_FRAME_H * 2,
                    0
                );

                
                // Exemplo simples de alternância de estado
                if (boss.cooldown_ataque > 0) boss.cooldown_ataque--;
                if (boss.cooldown_escudo > 0) boss.cooldown_escudo--;

                if (boss.cooldown_ataque == 0 && boss.estado != 2) {
                    boss.estado = 2; // atacando
                    boss.cooldown_ataque = 120; // 2 segundos
                    
                    // Criar a bola de fogo
                    for (int i = 0; i < MAX_BOLAS_FOGO; i++) {
                        if (!bolas_fogo[i].ativa) {
                            bolas_fogo[i].ativa = 1;
                            bolas_fogo[i].x = boss.x; // posição da mão do boss
                            bolas_fogo[i].y = boss.y - 180; // ajuste para altura da mão
                            bolas_fogo[i].vx = -8; // velocidade para a esquerda
                            bolas_fogo[i].vy = 0;
                            break;
                        }
                    }
                } else if (boss.cooldown_ataque == 60) {
                    boss.estado = 1; // parado
                }

                if (boss.cooldown_escudo == 0 && boss.estado != 0) {
                    boss.estado = 0; // escudo
                    boss.cooldown_escudo = 180; // 3 segundos
                } else if (boss.cooldown_escudo == 90) {
                    boss.estado = 1; // parado
                }

                // Atualiza e desenha as bolas de fogo
                for (int i = 0; i < MAX_BOLAS_FOGO; i++) {
                    if (bolas_fogo[i].ativa) {
                        bolas_fogo[i].x += bolas_fogo[i].vx;
                        bolas_fogo[i].y += bolas_fogo[i].vy;
                        
                        al_draw_bitmap(bola_fogo_sprite, bolas_fogo[i].x, bolas_fogo[i].y, 0);
                        
                        // Desativa se sair da tela
                        if (bolas_fogo[i].x < -BOLA_FOGO_W || bolas_fogo[i].x > X_SCREEN + BOLA_FOGO_W) {
                            bolas_fogo[i].ativa = 0;
                        }
                    }
                }

                al_flip_display();
                al_rest(0.01);
            }
            square_destroy(player_boss);
            state = MENU; // Volta para o menu depois do boss
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
    return 0;
}