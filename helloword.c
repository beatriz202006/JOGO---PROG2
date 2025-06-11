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

typedef enum { MENU, GAME, EXIT } GameState;

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

struct Chama {
    float x, y;
    float vx, vy;
    int ativa;
};


struct Chama chamas[MAX_CHAMAS] = {0};

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
        // Libere recursos e retorne!
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
        // Libere recursos e retorne!
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

    // Agora sim, inicialize o inimigo usando plat_y:
    struct inimigo fogo;
    int fogo_vida = 2; //vida do inimigo (morre após 6 balas)
    fogo.x = X_SCREEN; // começa na direita da tela
    float escala = 0.4; // mesma escala usada no desenho
    fogo.y = plat_y - fogo_frame_h * escala; // alinhado com o topo da plataforma
    fogo.direcao = 1; // sempre para a esquerda
    fogo.velocidade = 3;
    fogo.frame = 2; // frame 2: andando para a esquerda

    // Carrega a spritesheet da chama
    ALLEGRO_BITMAP* chama_sprite = al_load_bitmap("chama2.png");
    if (!chama_sprite) {
        printf("Erro ao carregar sprite da chama!\n");
        // Libere recursos e retorne!
    }
    int CHAMA_W = 46;
    int CHAMA_H = 33;

    // Carrega a spritesheet dos corações
    ALLEGRO_BITMAP* coracao_sprite = al_load_bitmap("vida.png");
    al_convert_mask_to_alpha(coracao_sprite, al_map_rgb(0,0,0));
    if (!coracao_sprite) {
        printf("Erro ao carregar sprite dos corações!\n");
        // Libere recursos e retorne!
    }
    int CORACAO_W = 100 / 3; // 3 sprites na horizontal
    int CORACAO_H = 35;

    int vida = 50;//ou outro valor inicial

    while (state != EXIT) {
        if (state == MENU) {
            bool menu_running = true;
            while (menu_running) {
                al_clear_to_color(al_map_rgb(10, 10, 80)); // Fundo roxo/azul escuro

                al_draw_text(font, al_map_rgb(255,255,255), X_SCREEN/2, Y_SCREEN/2-20, ALLEGRO_ALIGN_CENTRE, "INICIAR");
                al_draw_text(font, al_map_rgb(255,255,255), X_SCREEN/2, Y_SCREEN/2+20, ALLEGRO_ALIGN_CENTRE, "SAIR");

                // Destaque da opção selecionada
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

            // Defina um ponto de travamento (centro da tela)
            int travamento_x = X_SCREEN / 2;

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
                        // Mova todos os objetos do cenário para a esquerda
                        fogo.x -= player_speed;
                        for (int i = 0; i < MAX_BULLETS; i++) {
                            if (bullets[i].ativa) bullets[i].x -= player_speed;
                        }
                        for (int i = 0; i < MAX_CHAMAS; i++) {
                            if (chamas[i].ativa) chamas[i].x -= player_speed;
                        }
                        plat_x -= player_speed; // se quiser mover a plataforma suspensa também
                    }
                    andando = true;
                    if (key[ALLEGRO_KEY_RIGHT] && frame_counter % 3 == 0) correndo = true;
                }

                if (key[ALLEGRO_KEY_LEFT]) {
                    if (player->x > travamento_x) {
                        square_move(player, 1, 0, X_SCREEN, Y_SCREEN);
                    } else if (bg_offset_x > 0) {
                        bg_offset_x -= player_speed;
                        fogo.x += player_speed;
                        for (int i = 0; i < MAX_BULLETS; i++) {
                            if (bullets[i].ativa) bullets[i].x += player_speed;
                        }
                        for (int i = 0; i < MAX_CHAMAS; i++) {
                            if (chamas[i].ativa) chamas[i].x += player_speed;
                        }
                        plat_x += player_speed;
                    } else {
                        square_move(player, 1, 0, X_SCREEN, Y_SCREEN);
                    }
                    andando = true;
                    if (key[ALLEGRO_KEY_LEFT] && frame_counter % 3 == 0) correndo = true;
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

                // Atualiza e desenha as balas
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (bullets[i].ativa) {
                        bullets[i].x += bullets[i].vx;
                        bullets[i].y += bullets[i].vy;
                        al_draw_bitmap(bullet_img, bullets[i].x - BULLET_W/2, bullets[i].y, 0);

                        // Colisão do tiro com o fogo (bounding box igual ao desenho do fogo)
                        float fogo_w = fogo_frame_w * escala;
                        float fogo_h = fogo_frame_h * escala;
                        float fogo_left   = fogo.x;
                        float fogo_right  = fogo.x + fogo_w;
                        float fogo_top    = fogo.y;
                        float fogo_bottom = fogo.y + fogo_h;

                        float bullet_left   = bullets[i].x - BULLET_W/2;
                        float bullet_right  = bullets[i].x + BULLET_W/2;
                        float bullet_top    = bullets[i].y;
                        float bullet_bottom = bullets[i].y + BULLET_H;

                        if (
                            fogo_vida > 0 && // só toma dano se estiver vivo
                            bullet_right > fogo_left &&
                            bullet_left < fogo_right &&
                            bullet_bottom > fogo_top &&
                            bullet_top < fogo_bottom
                        ) {
                            fogo_vida--;
                            bullets[i].ativa = 0; // desativa o tiro ao colidir

                            // Se a vida do fogo acabou, "mata" o inimigo (tira da tela)
                            if (fogo_vida <= 0) {
                                fogo.x = X_SCREEN + 200; // manda para fora da tela
                                // Opcional: pode parar de desenhar/mover o fogo
                            }
                        }
                    }
                }

                // --- Lógica e desenho do inimigo ---
                static int passo = 0;
                passo++;

                // Sempre para a esquerda
                fogo.x -= fogo.velocidade;

                // Alterna entre andar e correr a cada 120 frames (~2 segundos)
                if (passo % 240 < 120) {
                    fogo.velocidade = 3; // andando
                    fogo.frame = 2;      // frame 2: andando para a esquerda
                } else {
                    fogo.velocidade = 7; // correndo
                    fogo.frame = 3;      // frame 3: correndo para a esquerda
                }

                // Se saiu da tela à esquerda, volta para a direita
                if (fogo.x < -fogo_frame_w * 0.5) {
                    fogo.x = X_SCREEN + fogo_frame_w * 0.5;
                    passo = 0; // reinicia o ciclo de andar/correr
                }

                // Desenha o inimigo
                float escala = 0.4;// 0.5 = metade do tamanho 
                al_draw_scaled_bitmap(
                    fogo_sprite,
                    fogo.frame * fogo_frame_w, 0, // origem do recorte
                    fogo_frame_w, fogo_frame_h,   // tamanho do recorte
                    fogo.x, fogo.y,               // posição na tela
                    fogo_frame_w * escala,        // nova largura
                    fogo_frame_h * escala,        // nova altura
                    0
                );

                // Bounding box do fogo
                float fogo_w = fogo_frame_w * escala;
                float fogo_h = fogo_frame_h * escala;
                float fogo_left   = fogo.x;
                float fogo_right  = fogo.x + fogo_w;
                float fogo_top    = fogo.y;
                float fogo_bottom = fogo.y + fogo_h;

                // Bounding box do player
                float player_left   = player->x - SPRITE_W/2;
                float player_right  = player->x + SPRITE_W/2;
                float player_top    = player->y + player->side/2 - SPRITE_H;
                float player_bottom = player->y + player->side/2;

                if (
                    fogo_right > player_left &&
                    fogo_left < player_right &&
                    fogo_bottom > player_top &&
                    fogo_top < player_bottom
                ) {
                    vida--;
                    // Opcional: invencibilidade temporária
                }

                static int chama_timer = 0;
                chama_timer++;
                if (chama_timer > 70){//lança a cada 70 frames
                    chama_timer = 0;
                    for (int i = 0; i < MAX_CHAMAS; i++) {
                        if (!chamas[i].ativa) {
                            chamas[i].ativa = 1;
                            // Boca do fogo: ajuste para a posição certa!
                            chamas[i].x = fogo.x + (fogo_frame_w * escala) / 4; // ajuste para a boca
                            chamas[i].y = fogo.y + (fogo_frame_h * escala) / 2;
                            chamas[i].vx = -10; // velocidade para a esquerda
                            chamas[i].vy = 0;
                            break;
                        }
                    }
                }

                // Atualiza e desenha as chamas
                for (int i = 0; i < MAX_CHAMAS; i++) {
                    if (chamas[i].ativa) {
                        chamas[i].x += chamas[i].vx;
                        chamas[i].y += chamas[i].vy;
                        al_draw_bitmap(chama_sprite, chamas[i].x, chamas[i].y, 0);

                        // Aqui vai a colisão correta:
                        float chama_left   = chamas[i].x;
                        float chama_right  = chamas[i].x + CHAMA_W;
                        float chama_top    = chamas[i].y;
                        float chama_bottom = chamas[i].y + CHAMA_H;

                        // Use as mesmas variáveis player_left, player_right, player_top, player_bottom já calculadas antes!
                        if (
                            chama_right > player_left &&
                            chama_left < player_right &&
                            chama_bottom > player_top &&
                            chama_top < player_bottom
                        ) {
                            vida--;
                            chamas[i].ativa = 0; // desativa a chama ao colidir
                        }

                        // Desativa se sair da tela
                        if (chamas[i].x < -CHAMA_W) {
                            chamas[i].ativa = 0;
                        }
                    }
                }

                char vida_str[32];
                sprintf(vida_str, "Vida: %d", vida);
                al_draw_text(font, al_map_rgb(255,0,0), 20, 20, 0, vida_str);

                int vida_max = 20; // exemplo
                int num_coracoes = vida_max / 2;
                for (int i = 0; i < num_coracoes; i++) {
                    int tipo;
                    if (vida >= (i+1)*2) {
                        tipo = 0; // cheio
                    } else if (vida == (i*2)+1) {
                        tipo = 2; // meio
                    } else {
                        tipo = 1; // vazio
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
                    // Exibe mensagem, volta ao menu, etc.
                    jogando = false;
                }

                al_flip_display();
                al_rest(0.01);
            }

            square_destroy(player);
            state = MENU;
        }
    }

   al_destroy_bitmap(fogo_sprite);
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
    return 0;
}