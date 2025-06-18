#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <stdio.h>
#include <stdbool.h>

#include "Square.h"
#include "config.h"
#include "utils.h"
#include "player.h"
#include "enemy.h"
#include "boss.h"
#include "projetil.h"

int main() {

    // Inicializa a biblioteca principal Allegro
    al_init();
    // Inicializa o addon de primitivas gráficas (para desenhar formas simples, como retângulos)
    al_init_primitives_addon();
    // Inicializa o addon para carregamento de imagens (bitmaps, png, etc)
    al_init_image_addon();
    // Inicializa o módulo de teclado (para capturar eventos de teclado)
    al_install_keyboard();

    // Cria a janela do jogo com as dimensões definidas em X_SCREEN e Y_SCREEN
    ALLEGRO_DISPLAY* disp = al_create_display(X_SCREEN, Y_SCREEN);
    // Cria uma fonte padrão do Allegro
    ALLEGRO_FONT* font = al_create_builtin_font();
    // Cria a fila de eventos, responsável por receber eventos do teclado e da janela
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();

    // Registra as fontes dos eventos de teclado e de fechamento da janela na fila de eventos
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));

    // Carrega o background principal do jogo
    ALLEGRO_BITMAP* bg = al_load_bitmap("backgroundfull.png");
    if (!bg) {
        // Se não conseguir carregar, imprime erro e encerra liberando recursos
        printf("Erro ao carregar background!\n");
        al_destroy_font(font);
        al_destroy_display(disp);
        al_destroy_event_queue(queue);
        return 1;
    }

    // Pega as dimensões do background carregado
    int bg_width = al_get_bitmap_width(bg);
    int bg_height = al_get_bitmap_height(bg);

    // --- CARREGA A IMAGEM DO SPRITESHEET ---
    // Carrega o spritesheet principal do personagem
    ALLEGRO_BITMAP* sprite_sheet = al_load_bitmap("spritesagua.png");
    if (!sprite_sheet) {
        printf("Erro ao carregar spritesagua.png\n");
        return 1;
    }
    // Define função de conversão de máscara para transparência
    al_convert_mask_to_alpha(sprite_sheet, al_map_rgb(0,0,0));

    // Carrega sprite do projétil padrão da 1 fase
    ALLEGRO_BITMAP* bullet_img = al_load_bitmap("bala.png");
    if (!bullet_img) {
        printf("Erro ao carregar sprite do projetil!\n");
        al_destroy_bitmap(sprite_sheet);
        al_destroy_bitmap(bg);
        al_destroy_font(font);
        al_destroy_display(disp);
        al_destroy_event_queue(queue);
        return 1;
    }
    int BULLET_W = 22;    // largura do projétil
    int BULLET_H = 33;    // altura do projétil

    // Carrega sprite do projétil do player da fase 2
    ALLEGRO_BITMAP* bullet_boss_img = al_load_bitmap("projetilaguaboss.png");
    if (!bullet_boss_img) {
        printf("Erro ao carregar sprite da bala do player da fase 2!\n");
    }
    int BULLET_BOSS_W = 52;
    int BULLET_BOSS_H = 78;

    // Estado inicial do jogo (menu), opção selecionada no menu e controle do estado anterior
    GameState state = MENU;
    int menu_option = 0;
    GameState state_anterior = MENU;

    // Carrega sprite do inimigo de fogo (foguinhos da 1 fase)
    ALLEGRO_BITMAP* fogo_sprite = al_load_bitmap("spritesfogofull.png");
    al_convert_mask_to_alpha(fogo_sprite, al_map_rgb(0,0,0));
    if (!fogo_sprite) {
        printf("Erro ao carregar sprite do inimigo!\n");
    }
    // Calcula tamanho dos frames do inimigo (sprite sheet dividido em 4 colunas)
    int fogo_sprite_w = al_get_bitmap_width(fogo_sprite);
    int fogo_sprite_h = al_get_bitmap_height(fogo_sprite);
    int fogo_frame_w = fogo_sprite_w / 4;
    int fogo_frame_h = fogo_sprite_h;

    // Parâmetros das plataformas do cenário
    int plataforma_y = Y_SCREEN - 265;
    int plat_x = 400;
    int plat_w = 400;
    int plat_y = 750;
    int plat_h = 20;

    // Escala dos inimigos de fogo
    float escala = 0.4;
    // Inicializa posição, estado e timers de cada inimigo de fogo
    for (int i = 0; i < NUM_FOGOS; i++) {
        fogos[i].x = X_SCREEN + i * 1800; // cada fogo começa fora da tela e vai se movendo para a esquerda
        fogos[i].y = plat_y - fogo_frame_h * escala;
        fogos[i].direcao = 1;
        fogos[i].velocidade = 1;
        fogos[i].frame = 2;
        fogos_vida[i] = 2;
        fogos_respawn_timer[i] = i * 900; // timers de respawn diferentes
        fogo_derrotado[i] = 0;            // nenhum fogo derrotado no início
    }

    // Carrega sprite da chama (projetil dos fogos)
    ALLEGRO_BITMAP* chama_sprite = al_load_bitmap("chama2.png");
    if (!chama_sprite) {
        printf("Erro ao carregar sprite da chama!\n");
    }
    int CHAMA_W = 46;
    int CHAMA_H = 33;

    // Carrega sprite dos corações (vida do player)
    ALLEGRO_BITMAP* coracao_sprite = al_load_bitmap("vida.png");
    al_convert_mask_to_alpha(coracao_sprite, al_map_rgb(0,0,0));
    if (!coracao_sprite) {
        printf("Erro ao carregar sprite dos corações!\n");
    }
    int CORACAO_W = 100 / 3;
    int CORACAO_H = 35;

    // Variáveis do jogador: vida, stamina, estado de fadiga
    int vida = 20;
    bool rodada_fogos_acabou = false;
    int stamina_max = 100;
    int stamina = 100;
    int stamina_recupera_tick = 0; // controla tempo de recarga da stamina
    int stamina_fadiga_tick = 0;   // controla tempo de fadiga
    bool cansado = false;    

    // Estrutura para as chamas dos inimigos: cada linha é uma chama ativa/inativa
    float fires[MAX_FIRES][5] = {0};
    // Índices: 0: x, 1: y, 2: vx, 3: vy, 4: ativa

    // Carrega o background do menu
    ALLEGRO_BITMAP* bg_menu = al_load_bitmap("menu.png");
    if (!bg_menu) {
        printf("Erro ao carregar background do menu!\n");
    }

    // Timer para dano do fogo (para não tirar vida muito rápido)
    int dano_fogo_cooldown = 0;

    // Carrega imagens de game over e vitória
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

    // Carrega imagem de pausa
    ALLEGRO_BITMAP* pause_img = al_load_bitmap("teladepausa.png");
    if (!pause_img) {
        printf("Erro ao carregar imagem de pausa!\n");
    }

    // Carrega background do chefão (fase do boss)
    ALLEGRO_BITMAP* bg_boss = al_load_bitmap("backgroundchefefull2.png");
    if (!bg_boss) {
        printf("Erro ao carregar background do chefão!\n");
    }

    // Carrega imagem de transição para a luta contra o chefão
    ALLEGRO_BITMAP* boss_unlocked_img = al_load_bitmap("chefedesbloqueado.png");
    if (!boss_unlocked_img) {
        printf("Erro ao carregar imagem de transição do chefão!\n");
    }

    // Carrega sprites de animação do boss (chefão) em várias situações
    ALLEGRO_BITMAP* boss_sprite = al_load_bitmap("chefe2.png");
    if (!boss_sprite) {
        printf("Erro ao carregar sprite do chefão!\n");
    }
    int BOSS_FRAME_W = 150;
    int BOSS_FRAME_H = 150;

    // Carrega sprite da bola de fogo do chefão
    ALLEGRO_BITMAP* bola_fogo_sprite = al_load_bitmap("boladefogo2.png");
    if (!bola_fogo_sprite) {
        printf("Erro ao carregar sprite da bola de fogo!\n");
    }
    int BOLA_FOGO_W = 77;
    int BOLA_FOGO_H = 78;

    // Carrega sprite de barra de vida do boss
    ALLEGRO_BITMAP* boss_vida_sprite = al_load_bitmap("vidachefe.png");
    if (!boss_vida_sprite) {
        printf("Erro ao carregar sprite das barras de vida do boss!\n");
    }
    int BOSS_VIDA_W = 150;
    int BOSS_VIDA_H = 68;

    // Carrega sprite de dano do boss e ajusta transparência
    ALLEGRO_BITMAP* boss_dano_sprite = al_load_bitmap("chefedano.png");
    if (!boss_dano_sprite) {
        printf("Erro ao carregar sprite de dano do boss!\n");
    }
    int BOSS_DANO_W = 129;
    int BOSS_DANO_H = 105;
    al_convert_mask_to_alpha(boss_dano_sprite, al_map_rgb(0,0,0));

    // Carrega as outras animações do boss (cada uma para uma ação diferente)
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

    // Carrega background da fase 3 (fase do dragão)
    ALLEGRO_BITMAP* bg_night = al_load_bitmap("backgroundfase3full.png");
    if (!bg_night) {
        printf("Erro ao carregar background da fase noturna!\n");
    }

    // Carrega sprites do dragão (chefão da fase 3) em diferentes estados
    ALLEGRO_BITMAP* dragon_idle = al_load_bitmap("dragaoparado.png"); // 200x300
    ALLEGRO_BITMAP* dragon_attack = al_load_bitmap("dragaofogo2.png"); // 200x144
    ALLEGRO_BITMAP* dragon_almost_dead = al_load_bitmap("dragaoquasemorto.png"); // 200x184
    ALLEGRO_BITMAP* dragon_dead = al_load_bitmap("dragaomorto.png"); // 200x156

    // Checa se todas as sprites do dragão foram carregadas corretamente
    if (!dragon_idle || !dragon_attack || !dragon_almost_dead || !dragon_dead) {
        printf("Erro ao carregar sprites do chefão da fase 3!\n");
    }
    // Ajusta a transparência das sprites do dragão
    al_convert_mask_to_alpha(dragon_idle, al_map_rgb(0,0,0));
    al_convert_mask_to_alpha(dragon_attack, al_map_rgb(0,0,0));
    al_convert_mask_to_alpha(dragon_almost_dead, al_map_rgb(0,0,0));
    al_convert_mask_to_alpha(dragon_dead, al_map_rgb(0,0,0));

    // Carrega sprite da chama disparada pelo dragão
    ALLEGRO_BITMAP* dragon_fire = al_load_bitmap("spritefogodragao.png");
    if (!dragon_fire) { printf("Erro ao carregar sprite da chama!\n"); }
    al_convert_mask_to_alpha(dragon_fire, al_map_rgb(0,0,0));

    // Cria a estrutura do player e ponteiro para o quadrado do player (usado para colisão/movimentação)
    struct Player player;
    square* player_sq = NULL;

    while (state != EXIT) {
    // ----------------------------
    // BLOCO DO MENU PRINCIPAL
    // ----------------------------
    if (state == MENU) {
        rodada_fogos_acabou = false;
        // Reseta o estado dos inimigos de fogo ao entrar no menu
        for (int i = 0; i < NUM_FOGOS; i++) fogo_derrotado[i] = 0;
        bool menu_running = true;
        while (menu_running) {
            // Desenha o fundo do menu
            if (bg_menu) {
                al_draw_bitmap(bg_menu, 0, -300, 0);
            }
            // Desenha as opções do menu ("INICIAR" e "SAIR")
            al_draw_text(font, al_map_rgb(255,255,255), X_SCREEN/2, Y_SCREEN/2-20, ALLEGRO_ALIGN_CENTRE, "INICIAR");
            al_draw_text(font, al_map_rgb(255,255,255), X_SCREEN/2, Y_SCREEN/2+20, ALLEGRO_ALIGN_CENTRE, "SAIR");

            // Destaca a opção selecionada com um retângulo amarelo
            if (menu_option == 0)
                al_draw_rectangle(X_SCREEN/2-40, Y_SCREEN/2-25, X_SCREEN/2+40, Y_SCREEN/2-5, al_map_rgb(255,255,0), 2);
            else
                al_draw_rectangle(X_SCREEN/2-40, Y_SCREEN/2+15, X_SCREEN/2+40, Y_SCREEN/2+35, al_map_rgb(255,255,0), 2);

            al_flip_display();

            // Espera e processa evento do teclado ou fechamento da janela
            ALLEGRO_EVENT event;
            al_wait_for_event(queue, &event);
            if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                // Alterna opção selecionada com seta para cima/baixo
                if (event.keyboard.keycode == ALLEGRO_KEY_UP || event.keyboard.keycode == ALLEGRO_KEY_DOWN)
                    menu_option = 1 - menu_option;
                // Seleciona opção com ENTER
                else if (event.keyboard.keycode == ALLEGRO_KEY_ENTER) {
                    if (menu_option == 0) { state = GAME; menu_running = false; }
                    else { state = EXIT; menu_running = false; }
                }
            }
            // Se o usuário fechar a janela, sai do jogo
            else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                state = EXIT;
                menu_running = false;
            }
            continue;
        }
    }

    // ----------------------------
    // BLOCO DA  1 FASE (GAME)
    // ----------------------------
    if (state == GAME) {
        // Inicializa os inimigos de fogo (posição, vida, timers)
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
        // Inicializa vida do player e variáveis auxiliares
        vida = 20;
        rodada_fogos_acabou = false;
        for (int i = 0; i < MAX_BULLETS; i++) bullets[i].ativa = 0;
        for (int i = 0; i < MAX_CHAMAS; i++) chamas[i].ativa = 0;
        int plataforma_y = Y_SCREEN - 265;
        int plat_x = 400;
        int plat_w = 400;
        int plat_y = 750;
        int plat_h = 20;

        // Cria o "quadrado" do player para controle de colisão/movimento
        player_sq = square_create(50, plat_x + plat_w/2, Y_SCREEN/2 - 200, X_SCREEN, Y_SCREEN);
        if (!player_sq) {
            printf("Erro ao criar quadrado!\n");
            state = EXIT;
            continue;
        }

        // Inicializa dados do player para animação e lógica
        player.x = plat_x + plat_w / 2;
        player.y = Y_SCREEN / 2 - 200;
        player.direcao = 0;
        player.no_chao = true;
        player.pulando = false;
        player.abaixado = false;
        player.atirando = false;
        player.atirando_cima = false;
        player.atirando_diag = false;

        // Variáveis de controle do cenário e player
        int bg_offset_x = 0;
        int player_speed = 10;
        bool jogando = true;
        bool key[ALLEGRO_KEY_MAX] = {false}; // Armazena teclas pressionadas

        float vel_y = 0;
        bool no_chao = false;
        int direcao = 0;
        int travamento_x = X_SCREEN / 2;
        int chama_timer[NUM_FOGOS] = {0};
        int passo = 0;
        int altura_colisao = SPRITE_H;

        // Inicializa estamina do player
        stamina = stamina_max;
        cansado = false;
        stamina_fadiga_tick = 0;
        stamina_recupera_tick = 0;

        // Loop de gameplay (fase principal)
        while (jogando) {
            // Processa eventos do Allegro (teclado, fechar janela)
            ALLEGRO_EVENT event;
            if (al_get_next_event(queue, &event)) {
                if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
                    jogando = false;
                else if (event.type == ALLEGRO_EVENT_KEY_DOWN)
                    key[event.keyboard.keycode] = true;
                else if (event.type == ALLEGRO_EVENT_KEY_UP)
                    key[event.keyboard.keycode] = false;
            }

            // Atualiza direção do player conforme as teclas pressionadas
            if (key[ALLEGRO_KEY_LEFT]) direcao = 1;
            if (key[ALLEGRO_KEY_RIGHT]) direcao = 0;

            bool andando = false, correndo = false;
            static int frame_counter = 0;
            frame_counter++;

            // Movimento para a direita: move o player ou o cenário
            if (key[ALLEGRO_KEY_RIGHT]) {
                if (player_sq->x < travamento_x) {
                    square_move(player_sq, 1, 1, X_SCREEN, Y_SCREEN);
                } else {
                    bg_offset_x += player_speed;
                    for (int f = 0; f < NUM_FOGOS; f++) fogos[f].x -= player_speed;
                    for (int i = 0; i < MAX_BULLETS; i++) if (bullets[i].ativa) bullets[i].x -= player_speed;
                    for (int i = 0; i < MAX_CHAMAS; i++) if (chamas[i].ativa) chamas[i].x -= player_speed;
                    plat_x -= player_speed;
                }
                andando = true;
                if (frame_counter % 3 == 0) correndo = true;
            }

            // Movimento para a esquerda: move o player ou o cenário
            if (key[ALLEGRO_KEY_LEFT]) {
                if (player_sq->x > travamento_x) {
                    square_move(player_sq, 1, 0, X_SCREEN, Y_SCREEN);
                } else if (bg_offset_x > 0) {
                    bg_offset_x -= player_speed;
                    for (int f = 0; f < NUM_FOGOS; f++) fogos[f].x += player_speed;
                    for (int i = 0; i < MAX_BULLETS; i++) if (bullets[i].ativa) bullets[i].x += player_speed;
                    for (int i = 0; i < MAX_CHAMAS; i++) if (chamas[i].ativa) chamas[i].x += player_speed;
                    plat_x += player_speed;
                } else {
                    square_move(player_sq, 1, 0, X_SCREEN, Y_SCREEN);
                }
                andando = true;
                if (frame_counter % 3 == 0) correndo = true;
            }

            // Lógica de pulo: se pressionar espaço ou seta para cima, aplica velocidade vertical
            if ((key[ALLEGRO_KEY_SPACE] || key[ALLEGRO_KEY_UP]) && no_chao) {
                vel_y = -20;
                no_chao = false;
            }

            // Gravidade: atualiza posição vertical do player e verifica colisão com chão/plataforma
            if (!no_chao) {
                player_sq->y += vel_y;
                vel_y += 1.5;
                if (
                    vel_y > 0 &&
                    player_sq->y + player_sq->side/2 >= plat_y &&
                    player_sq->y + player_sq->side/2 - vel_y < plat_y &&
                    player_sq->x + player_sq->side/2 > plat_x &&
                    player_sq->x - player_sq->side/2 < plat_x + plat_w
                ) {
                    player_sq->y = plat_y - player_sq->side/2;
                    vel_y = 0;
                    no_chao = true;
                }
                else if (player_sq->y + player_sq->side/2 >= plataforma_y) {
                    player_sq->y = plataforma_y - player_sq->side/2;
                    vel_y = 0;
                    no_chao = true;
                }
            }

            // Se apertar ESC volta para o menu
            if (key[ALLEGRO_KEY_ESCAPE]) {
                jogando = false;
                state = MENU;
            }

            // Atualiza a struct Player para animação correta do sprite
            player.x = player_sq->x;
            player.y = player_sq->y;
            player.direcao = direcao;
            player.no_chao = no_chao;
            player.abaixado = (key[ALLEGRO_KEY_DOWN] && no_chao);
            player.atirando = key[ALLEGRO_KEY_Z];
            player.atirando_cima = (key[ALLEGRO_KEY_UP] && key[ALLEGRO_KEY_Z]);
            player.atirando_diag =
                ((key[ALLEGRO_KEY_UP] && key[ALLEGRO_KEY_RIGHT] && key[ALLEGRO_KEY_Z]) ||
                (key[ALLEGRO_KEY_UP] && key[ALLEGRO_KEY_LEFT]  && key[ALLEGRO_KEY_Z]) ||
                (key[ALLEGRO_KEY_X])); // X atira na diagonal na direção atual
            player.pulando = (!no_chao);

            // --- DESENHO DO FUNDO ---
            // Desenha o fundo repetidamente para cobrir toda a tela
            int start_x = -(bg_offset_x % bg_width);
            for (int x = start_x; x <= X_SCREEN; x += bg_width) {
                for (int y = 0; y <= Y_SCREEN; y += bg_height) {
                    al_draw_bitmap(bg, x, y, 0);
                }
            }

            // --- PAUSA IN-GAME ---
            // Se apertar P, chama a tela de pausa e espera sair da pausa
            if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                key[event.keyboard.keycode] = true;
                if (event.keyboard.keycode == ALLEGRO_KEY_P) {
                    tela_pausa(disp, pause_img);

                    // Limpa eventos antigos do buffer
                    ALLEGRO_EVENT temp_event;
                    while (al_get_next_event(queue, &temp_event)) {}

                    // Espera soltar e apertar P de novo para retornar ao jogo
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
                }
            }

            // --- DESENHO DO PLAYER ---
            // Calcula qual frame da animação do player desenhar
            int sprite_indice = player_get_sprite_index(&player);
            struct SpriteFrame frame = player_frames[sprite_indice];
            al_draw_bitmap_region(
                sprite_sheet,
                frame.x, frame.y, frame.w, frame.h,
                player.x - frame.w/2,
                player.y + player_sq->side/2 - frame.h,
                0
            );

            // --- LÓGICA DOS TIROS DO PLAYER ---
            static double last_shot_time = 0;
            double now = al_get_time();
            double shot_delay = 0.15;
            // Só atira se não estiver cansado, e respeitando delay entre tiros
            if (key[ALLEGRO_KEY_Z] && now - last_shot_time > shot_delay  && stamina >= 10 && !cansado) {
                last_shot_time = now;
                stamina -= 10; // Consome estamina no ataque
                if(stamina < 10) {
                    stamina = 0;
                    cansado = true;
                    stamina_fadiga_tick = 0; // começa a contar o tempo de descanso
                }

                // Procura uma bala livre para ativar e define sua direção
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (!bullets[i].ativa) {
                        bullets[i].ativa = 1;
                        // Tiro diagonal
                        if (player.atirando_diag) {
                            if (direcao == 0) { // direita
                                bullets[i].x = player.x + frame.w/2;
                                bullets[i].vx = 10;
                            } else { // esquerda
                                bullets[i].x = player.x - frame.w/2;
                                bullets[i].vx = -10;
                            }
                            bullets[i].y = player.y + player_sq->side/2 - frame.h - 10;
                            bullets[i].vy = -10;
                        }
                        // Tiro para cima
                        else if (key[ALLEGRO_KEY_UP]) {
                            bullets[i].x = player.x + 10;
                            bullets[i].y = player.y + player_sq->side/2 - frame.h;
                            bullets[i].vx = 0;
                            bullets[i].vy = -15;
                        }
                        // Tiro abaixado
                        else if (key[ALLEGRO_KEY_DOWN] && no_chao) {
                            if (direcao == 0) {
                                bullets[i].x = player.x + frame.w/2;
                            } else {
                                bullets[i].x = player.x - frame.w/2;
                            }
                            bullets[i].y = player.y + player_sq->side/2 - frame.h/2 - 5;
                            bullets[i].vx = (direcao == 0) ? 15 : -15;
                            bullets[i].vy = 0;
                        }
                        // Tiro horizontal
                        else {
                            if (direcao == 0) {
                                bullets[i].x = player.x + frame.w/2;
                            } else {
                                bullets[i].x = player.x - frame.w/2;
                            }
                            bullets[i].y = player.y + player_sq->side/2 - frame.h + 20;
                            bullets[i].vx = (direcao == 0) ? 15 : -15;
                            bullets[i].vy = 0;
                        }
                        break;
                    }
                }
            }

            // Atualiza e desenha os tiros ativos
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (bullets[i].ativa) {
                    bullets[i].x += bullets[i].vx;
                    bullets[i].y += bullets[i].vy;
                    al_draw_bitmap(bullet_img, bullets[i].x - BULLET_W/2, bullets[i].y, 0);
                }
            }

            // Calcula as bordas do player para detecção de colisão
            float player_left   = player.x - frame.w/2;
            float player_right  = player.x + frame.w/2;
            float player_top    = player.y + player_sq->side/2 - frame.h;
            float player_bottom = player.y + player_sq->side/2;

            // --- LÓGICA DOS INIMIGOS DE FOGO ---
            passo++;
            for (int f = 0; f < NUM_FOGOS; f++) {
                // Se o fogo saiu da tela, marca como morto/fora de jogo
                if (fogos_vida[f] > 0 && fogos[f].x < -fogo_frame_w * escala && !rodada_fogos_acabou) {
                    fogos_vida[f] = 0;
                    fogos_respawn_timer[f] = 100 + f * 20;
                    fogos[f].x = -1000;
                    fogo_derrotado[f] = 0;
                    fogo_morto_por_tiro[f] = 0;
                }
                // Lógica de respawn dos fogos
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
                // Remove fogos derrotados após o fim da rodada
                if (fogos_vida[f] <= 0 && rodada_fogos_acabou) {
                    fogos[f].x = -1000;
                    chama_timer[f] = 0;
                    continue;
                }
                // Se morto, ignora
                if (fogos_vida[f] <= 0) {
                    chama_timer[f] = 0;
                    continue;
                }
                // Move o fogo pela tela, alternando velocidade e frame para animação
                fogos[f].x -= fogos[f].velocidade;
                if (passo % 240 < 120) {
                    fogos[f].velocidade = 3;
                    fogos[f].frame = 2;
                } else {
                    fogos[f].velocidade = 7;
                    fogos[f].frame = 3;
                }
                // Desenha o inimigo de fogo
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
                // Colisão entre fogo e player: dano ao player
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
                // Colisão dos tiros do player com fogo
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
                // Timer para disparo da chama (ataque do fogo)
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
            // Atualiza e desenha todas as chamas ativas
            for (int i = 0; i < MAX_CHAMAS; i++) {
                if (chamas[i].ativa) {
                    chamas[i].x += chamas[i].vx;
                    chamas[i].y += chamas[i].vy;
                    al_draw_bitmap(chama_sprite, chamas[i].x, chamas[i].y, 0);
                    // Desativa chamas que saíram da tela
                    if (chamas[i].x < -CHAMA_W || chamas[i].x > X_SCREEN + CHAMA_W ||
                        chamas[i].y < -CHAMA_H || chamas[i].y > Y_SCREEN + CHAMA_H) {
                        chamas[i].ativa = 0;
                    }
                    // Colisão entre chama e player: dano ao player
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

            // Lógica de recuperação da estamina
            stamina_recupera_tick++;
            if (stamina_recupera_tick > 5) { // a cada 6 iterações, recupera 1 ponto
                stamina_recupera_tick = 0;
                if (stamina < stamina_max) stamina++;
            }

            // Desenha a vida do player (corações)
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

            // Game Over: se a vida zerar, mostra a tela de game over da derrota e volta para o menu
            if (vida <= 0) {
                tela_gameover(disp, gameover_img);
                jogando = false;
                state = MENU;
            }

            // Verifica se todos os  6 fogos foram derrotados para avançar para o boss
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

            // BLOCO DE RECUPERAÇÃO/CANSAÇO DA ESTAMINA
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

            // Controla o tempo de invulnerabilidade após dano do fogo
            if (dano_fogo_cooldown > 0) dano_fogo_cooldown--;

            // Desenha barra de estamina
            int bar_w = 200;
            int bar_h = 20;
            float perc = (float)stamina / stamina_max;
            al_draw_filled_rectangle(20, 60, 20 + bar_w * perc, 60 + bar_h, al_map_rgb(0,200,0));
            al_draw_rectangle(20, 60, 20 + bar_w, 60 + bar_h, al_map_rgb(0,0,0), 2);

            al_flip_display();
            al_rest(0.01); // pequena pausa para controle do FPS
        }
        // Libera a memória do quadrado do player ao sair da fase
        square_destroy(player_sq);
        continue;
    }

    // ----- BOSS -----
        if (state == BOSS) {
            // Mostra uma tela de transição informando que o chefão foi desbloqueado
            if (boss_unlocked_img) {
                al_draw_scaled_bitmap(boss_unlocked_img, 0, 0, 2048, 1536, 0, 0, X_SCREEN, Y_SCREEN, 0);
            } else {
                // Se não tiver imagem, mostra texto no fundo preto
                al_clear_to_color(al_map_rgb(0, 0, 0));
                al_draw_text(font, al_map_rgb(255, 255, 0), X_SCREEN/2, Y_SCREEN/2, ALLEGRO_ALIGN_CENTRE, "Chefão desbloqueado!");
            }
            al_flip_display();
            al_rest(2.0);

            // Inicializa a estrutura do player para a fase do boss
            square* player_boss_sq = square_create(50, 100, Y_SCREEN - 100, X_SCREEN, Y_SCREEN);
            if (!player_boss_sq) {
                printf("Erro ao criar quadrado!\n");
                state = EXIT;
                continue;
            }

            // Inicializa campos do player (posição, estado, animação)
            struct Player player_boss;
            player_boss.x = 100;
            player_boss.y = Y_SCREEN - 100;
            player_boss.direcao = 0;
            player_boss.no_chao = true;
            player_boss.pulando = false;
            player_boss.abaixado = false;
            player_boss.atirando = false;
            player_boss.atirando_cima = false;
            player_boss.atirando_diag = false;

            // Variáveis de controle do loop do boss
            bool boss_running = true;
            bool key[ALLEGRO_KEY_MAX] = {false};
            float vel_y = 0;
            bool no_chao = true;
            int direcao = 0;
            int altura_colisao = SPRITE_H;
            int vida_boss = 20; // Vida do jogador nessa fase

            // Inicialização da struct do chefão (boss)
            struct Boss boss;
            boss.x = X_SCREEN - 350;
            boss.y = Y_SCREEN - 10;
            boss.estado = BOSS_PARADO;
            boss.vida = 10;

            // Timers e variáveis auxiliares para os estados do boss
            int boss_state_timer = 0;
            int boss_dano_timer = 0;
            int boss_desfazendo_timer = 0;
            int passos_dados = 0;
            int boss_andando_bola_timer = 0;
            int boss_hit_counter = 0;

            // Estamina do player
            stamina = stamina_max;
            cansado = false;
            stamina_fadiga_tick = 0;
            stamina_recupera_tick = 0;

            // --- LOOP PRINCIPAL DA FASE DO BOSS ---
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

                    // Pausa durante a luta contra o boss
                    if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                        key[event.keyboard.keycode] = true;
                        if (event.keyboard.keycode == ALLEGRO_KEY_P) {
                            // Mostra tela de pausa
                            tela_pausa(disp, pause_img);
                            // Limpa eventos antigos do buffer
                            ALLEGRO_EVENT temp_event;
                            while (al_get_next_event(queue, &temp_event)) {}

                            // Espera o jogador soltar e apertar P de novo para sair da pausa
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
                        }
                    }
                }

                // Movimento lateral do player
                if (key[ALLEGRO_KEY_LEFT])  { square_move(player_boss_sq, 1, 0, X_SCREEN, Y_SCREEN); direcao = 1; }
                if (key[ALLEGRO_KEY_RIGHT]) { square_move(player_boss_sq, 1, 1, X_SCREEN, Y_SCREEN); direcao = 0; }

                // Pulo do player
                if ((key[ALLEGRO_KEY_SPACE] || key[ALLEGRO_KEY_UP]) && no_chao) { vel_y = -20; no_chao = false; }

                // Gravidade do player
                if (!no_chao) {
                    player_boss_sq->y += vel_y;
                    vel_y += 1.5;
                    if (player_boss_sq->y + player_boss_sq->side/2 >= Y_SCREEN - 90) {
                        player_boss_sq->y = Y_SCREEN - 90 - player_boss_sq->side/2;
                        vel_y = 0; no_chao = true;
                    }
                }

                // ESC retorna ao menu
                if (key[ALLEGRO_KEY_ESCAPE]) { boss_running = false; state = MENU; }

                // Desenha o background da fase do boss
                al_draw_scaled_bitmap(bg_boss, 0, 0, 2048, 1536, 0, -100, X_SCREEN, Y_SCREEN + 100, 0);

                // Atualiza informações do player para animação
                player_boss.x = player_boss_sq->x;
                player_boss.y = player_boss_sq->y;
                player_boss.direcao = direcao;
                player_boss.no_chao = no_chao;
                player_boss.abaixado = (key[ALLEGRO_KEY_DOWN] && no_chao);
                player_boss.atirando = key[ALLEGRO_KEY_Z];
                player_boss.atirando_cima = (key[ALLEGRO_KEY_UP] && key[ALLEGRO_KEY_Z]);
                player_boss.atirando_diag =
                    ((key[ALLEGRO_KEY_UP] && key[ALLEGRO_KEY_RIGHT] && key[ALLEGRO_KEY_Z]) ||
                    (key[ALLEGRO_KEY_UP] && key[ALLEGRO_KEY_LEFT]  && key[ALLEGRO_KEY_Z]) ||
                    (key[ALLEGRO_KEY_X])); // X atira na diagonal
                player_boss.pulando = (!no_chao);

                // Desenha o sprite do player usando as informações de animação
                int sprite_indice = player_get_sprite_index(&player_boss);
                struct SpriteFrame frame = player_frames[sprite_indice];
                al_draw_bitmap_region(
                    sprite_sheet,
                    frame.x, frame.y, frame.w, frame.h,
                    player_boss.x - frame.w/2,
                    player_boss.y + player_boss_sq->side/2 - frame.h,
                    0
                );

                // --- TIROS DO PLAYER NA FASE DO BOSS ---
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
                            // Definição da direção e tipo do tiro (diagonal, cima, abaixado, normal)
                            if (player_boss.atirando_diag) {
                                if (direcao == 0) {
                                    bullets[i].x = player_boss.x + frame.w/2;
                                    bullets[i].vx = 10;
                                } else {
                                    bullets[i].x = player_boss.x - frame.w/2;
                                    bullets[i].vx = -10;
                                }
                                bullets[i].y = player_boss.y + player_boss_sq->side/2 - frame.h - 10;
                                bullets[i].vy = -10;
                            } else if (key[ALLEGRO_KEY_UP]) {
                                bullets[i].x = player_boss.x + 10;
                                bullets[i].y = player_boss.y + player_boss_sq->side/2 - frame.h;
                                bullets[i].vx = 0;
                                bullets[i].vy = -15;
                            } else if (key[ALLEGRO_KEY_DOWN] && no_chao) {
                                if (direcao == 0) bullets[i].x = player_boss.x + frame.w/2;
                                else bullets[i].x = player_boss.x - frame.w/2;
                                bullets[i].y = player_boss.y + player_boss_sq->side/2 - frame.h/2 - 5;
                                bullets[i].vx = (direcao == 0) ? 15 : -15; bullets[i].vy = 0;
                            } else {
                                if (direcao == 0) bullets[i].x = player_boss.x + frame.w/2;
                                else bullets[i].x = player_boss.x - frame.w/2;
                                bullets[i].y = player_boss.y + player_boss_sq->side/2 - frame.h -5;
                                bullets[i].vx = (direcao == 0) ? 15 : -15; bullets[i].vy = 0;
                            }
                            break;
                        }
                    }
                }
                // Atualiza e desenha tiros ativos
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (bullets[i].ativa) {
                        bullets[i].x += bullets[i].vx;
                        bullets[i].y += bullets[i].vy;
                        al_draw_bitmap(bullet_boss_img, bullets[i].x - BULLET_BOSS_W/2, bullets[i].y, 0);
                    }
                }

                // Recuperação de estamina do player
                stamina_recupera_tick++;
                if (stamina_recupera_tick > 5) {
                    stamina_recupera_tick = 0;
                    if (stamina < stamina_max) stamina++;
                }

                // HUD da vida do player (desenha corações)
                int vida_max = 20, num_coracoes = vida_max / 2;
                for (int i = 0; i < num_coracoes; i++) {
                    int tipo = (vida_boss >= (i+1)*2) ? 0 : (vida_boss == (i*2)+1) ? 2 : 1;
                    al_draw_bitmap_region(coracao_sprite, tipo * CORACAO_W, 0, CORACAO_W, CORACAO_H,
                        20 + i * (CORACAO_W + 5), 20, 0);
                }

                // Se o player morrer, mostra tela de gameover e volta ao menu
                if (vida_boss <= 0) {
                    tela_gameover(disp, gameover_img);
                    boss_running = false;
                    state = MENU;
                    continue;
                }

                // --------- LÓGICA DO BOSS (CHEFÃO) ---------
                // Transições especiais de estado: desfazendo/morto
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

                    // Mostra tela de vitória e avança para próxima fase
                    tela_vitoria(disp, victory_img);

                    boss_running = false;
                    state = FASE3;
                    continue;
                }

                // Quando o boss fica com pouca vida, ele entra no estado desfazendo
                if (boss.vida <= 3 && boss.estado != BOSS_DESFAZENDO && boss.estado != BOSS_MORTO) {
                    boss.estado = BOSS_DESFAZENDO;
                    boss.vida = 3;
                    boss_desfazendo_timer = 120;
                    continue;
                }

                // --- CICLO DE ESTADOS NORMAIS DO BOSS ---
                if (boss.estado == BOSS_PARADO) {
                    if (boss_state_timer == 0) boss_state_timer = 120;
                    boss_state_timer--;
                    if (boss_state_timer <= 0) { boss.estado = BOSS_ESCUDO; boss_state_timer = 0; }
                } else if (boss.estado == BOSS_ESCUDO) {
                    if (boss_state_timer == 0) boss_state_timer = 120;
                    boss_state_timer--;
                    if (boss_state_timer <= 0) { boss.estado = BOSS_ANDANDO; boss_state_timer = 0; passos_dados = 0; boss_andando_bola_timer = 0; }
                } else if (boss.estado == BOSS_ANDANDO) {
                    // Boss dispara bolas de fogo periodicamente enquanto anda
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
                    // Boss anda para a esquerda por alguns passos
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

                // --- DESENHO DO SPRITE DO BOSS CONFORME O ESTADO ---
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

                // Desenha a barra de vida do boss
                int vida_col = boss_bar_col(boss);
                int vida_row = boss_bar_row(boss);
                al_draw_bitmap_region(boss_vida_sprite, vida_col * BOSS_VIDA_W, vida_row * BOSS_VIDA_H, BOSS_VIDA_W, BOSS_VIDA_H,
                    X_SCREEN - BOSS_VIDA_W - 20, 20, 0);

                // --- COLISÃO DOS TIROS DO PLAYER COM O BOSS ---
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

                                // Aplica dano ao boss se ele não estiver no modo invulnerável
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
                                // Sempre desativa o tiro ao colidir
                                bullets[i].ativa = 0;
                            }
                        }
                    }
                }
                // Quando em estado de dano, zera o contador de hits
                if (boss.estado == BOSS_DANO) {
                    boss_hit_counter = 0;
                }

                // --- BOLAS DE FOGO DISPARADAS PELO BOSS ---
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
                float player_left   = player_boss.x - frame.w/2;
                float player_right  = player_boss.x + frame.w/2;
                float player_top    = player_boss.y + player_boss_sq->side/2 - frame.h;
                float player_bottom = player_boss.y + player_boss_sq->side/2;
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

                // Recuperação e cansaço da estamina do player
                if (cansado) {
                    stamina_fadiga_tick++;
                    if (stamina_fadiga_tick > 60) {
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

                // Desenha barra de estamina do player
                int bar_w = 200;
                int bar_h = 20;
                float perc = (float)stamina / stamina_max;
                al_draw_filled_rectangle(20, 60, 20 + bar_w * perc, 60 + bar_h, al_map_rgb(0,200,0));
                al_draw_rectangle(20, 60, 20 + bar_w, 60 + bar_h, al_map_rgb(0,0,0), 2);

                al_flip_display();
                al_rest(0.01);
            }
            // Libera a memória do quadrado do player ao sair da fase do boss
            square_destroy(player_boss_sq);
            continue;
        }

        // Fator de escala para sprites do dragão
        float dragon_scale = 2.0f;

        if (state == FASE3) {
            // --- Inicialização das variáveis de vida e estamina ---
            int vida_max_fase3 = 20;
            int vida_player_fase3 = vida_max_fase3;      // Vida do jogador nesta fase
            int boss3_vida_max = 15;
            int vida_boss3 = boss3_vida_max;             // Vida do chefão dragão
            bool fase3_running = true;
            bool key[ALLEGRO_KEY_MAX] = {false};         // Teclas pressionadas
            int stamina_fase3 = stamina_max;             // Estamina do jogador
            int stamina_recupera_tick_fase3 = 0;         // Timer de recuperação de estamina
            int stamina_fadiga_tick_fase3 = 0;           // Timer de fadiga de estamina
            bool cansado_fase3 = false;                  // Controle de fadiga

            // --- Inicializa o player (quadrado de colisão e struct para animação) ---
            square* player_fase3 = square_create(50, 50, Y_SCREEN-40, X_SCREEN, Y_SCREEN);
            struct Player player;
            player.x = 50;
            player.y = Y_SCREEN-40;
            player.direcao = 0;
            player.no_chao = true;
            player.pulando = false;
            player.abaixado = false;
            player.atirando = false;
            player.atirando_cima = false;
            player.atirando_diag = false;

            float vel_y_fase3 = 0;             // Velocidade vertical do player
            bool no_chao_fase3 = true;         // Controle se o player está no chão
            int direcao_fase3 = 0;             // Direção do player (0=dir, 1=esq)

            // --- Estado e variáveis do dragão chefão ---
            enum Boss3State { BOSS3_IDLE, BOSS3_ATTACK, BOSS3_ALMOST_DEAD, BOSS3_DEAD };
            int boss3_state = BOSS3_IDLE;      // Estado do dragão (parado, atacando, quase morto, morto)
            int boss3_timer = 0;               // Timer para controlar as transições de estado
            int boss3_x = X_SCREEN - DRAGON_IDLE_W * dragon_scale - 40;  // Posição do chefão
            int boss3_base_y = Y_SCREEN - (DRAGON_IDLE_H * dragon_scale) - 120; // Base Y do chefão

            // --- Chamas disparadas pelo dragão ---
            float fires[MAX_FIRES][5] = {0};   // Cada chama: x, y, vx, vy, ativa
            static int fire_timer = 0;         // Timer para disparo das chamas

            // --- LOOP PRINCIPAL DA FASE 3 ---
            while (fase3_running) {
                // ========== EVENTOS DE TECLADO E FECHAR JANELA ==========
                ALLEGRO_EVENT event;
                while (al_get_next_event(queue, &event)) {
                    if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                        fase3_running = false;
                        state = EXIT;
                    } else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                        key[event.keyboard.keycode] = true;
                    } else if (event.type == ALLEGRO_EVENT_KEY_UP) {
                        key[event.keyboard.keycode] = false;
                    }
                }

                // ========== TELA DE PAUSA ==========
                if (key[ALLEGRO_KEY_P]) {
                    // Desenha overlay de pausa
                    al_draw_filled_rectangle(0, 0, X_SCREEN, Y_SCREEN, al_map_rgba(0,0,0,180));
                    al_draw_text(font, al_map_rgb(255,255,255), X_SCREEN/2, Y_SCREEN/2-40, ALLEGRO_ALIGN_CENTER, "PAUSADO");
                    al_draw_text(font, al_map_rgb(200,200,200), X_SCREEN/2, Y_SCREEN/2+10, ALLEGRO_ALIGN_CENTER, "Pressione P para voltar");
                    al_flip_display();
                    // Espera soltar e apertar P de novo (padrão do jogo)
                    ALLEGRO_EVENT ev;
                    bool esperando_soltar = true;
                    while (esperando_soltar) {
                        al_wait_for_event(queue, &ev);
                        if (ev.type == ALLEGRO_EVENT_KEY_UP && ev.keyboard.keycode == ALLEGRO_KEY_P)
                            esperando_soltar = false;
                        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                            fase3_running = false;
                            state = EXIT;
                            break;
                        }
                    }
                    bool esperando = true;
                    while (esperando) {
                        al_wait_for_event(queue, &ev);
                        if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_P)
                            esperando = false;
                        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
                            fase3_running = false;
                            state = EXIT;
                            break;
                        }
                    }
                    key[ALLEGRO_KEY_P] = false;
                    continue;
                }

                // ========== DESENHA FUNDO DA FASE NOTURNA ==========
                al_draw_scaled_bitmap(bg_night, 0, 0, 2048, 1536, 0, 0, X_SCREEN, Y_SCREEN, 0);

                // ========== MOVIMENTO DO PLAYER ==========
                if (key[ALLEGRO_KEY_LEFT])  { square_move(player_fase3, 1, 0, X_SCREEN, Y_SCREEN); direcao_fase3 = 1; }
                if (key[ALLEGRO_KEY_RIGHT]) { square_move(player_fase3, 1, 1, X_SCREEN, Y_SCREEN); direcao_fase3 = 0; }
                if ((key[ALLEGRO_KEY_SPACE] || key[ALLEGRO_KEY_UP]) && no_chao_fase3) { vel_y_fase3 = -20; no_chao_fase3 = false; }
                // Gravidade do player
                if (!no_chao_fase3) {
                    player_fase3->y += vel_y_fase3;
                    vel_y_fase3 += 1.5;
                    if (player_fase3->y + player_fase3->side/2 >= Y_SCREEN - 90) {
                        player_fase3->y = Y_SCREEN - 90 - player_fase3->side/2;
                        vel_y_fase3 = 0; no_chao_fase3 = true;
                    }
                }

                // Atualiza struct Player para animação/sprite
                player.x = player_fase3->x;
                player.y = player_fase3->y;
                player.direcao = direcao_fase3;
                player.no_chao = no_chao_fase3;
                player.abaixado = (key[ALLEGRO_KEY_DOWN] && no_chao_fase3);
                player.atirando = key[ALLEGRO_KEY_Z];
                player.atirando_cima = (key[ALLEGRO_KEY_UP] && key[ALLEGRO_KEY_Z]);
                player.atirando_diag =
                    ((key[ALLEGRO_KEY_UP] && key[ALLEGRO_KEY_RIGHT] && key[ALLEGRO_KEY_Z]) ||
                    (key[ALLEGRO_KEY_UP] && key[ALLEGRO_KEY_LEFT]  && key[ALLEGRO_KEY_Z]) ||
                    (key[ALLEGRO_KEY_X])); // X atira na diagonal
                player.pulando = (!no_chao_fase3);

                // ========== DESENHA PLAYER ==========
                int sprite_indice = player_get_sprite_index(&player);
                struct SpriteFrame frame = player_frames[sprite_indice];
                al_draw_bitmap_region(
                    sprite_sheet,
                    frame.x, frame.y, frame.w, frame.h,
                    player.x - frame.w/2,
                    player.y + player_fase3->side/2 - frame.h,
                    0
                );

                // ========== TIRO DO PLAYER ==========
                static double last_shot_time_fase3 = 0;
                double now_fase3 = al_get_time();
                double shot_delay_fase3 = 0.15;
                if (key[ALLEGRO_KEY_Z] && now_fase3 - last_shot_time_fase3 > shot_delay_fase3 && stamina_fase3 >= 10 && !cansado_fase3) {
                    last_shot_time_fase3 = now_fase3;
                    stamina_fase3 -= 10;
                    if (stamina_fase3 < 10) {
                        stamina_fase3 = 0;
                        cansado_fase3 = true;
                        stamina_fadiga_tick_fase3 = 0;
                    }
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (!bullets[i].ativa) {
                            bullets[i].ativa = 1;
                            // Tiro diagonal
                            if (player.atirando_diag) {
                                if (direcao_fase3 == 0) { // direita
                                    bullets[i].x = player.x + frame.w/2;
                                    bullets[i].vx = 10;
                                } else {
                                    bullets[i].x = player.x - frame.w/2;
                                    bullets[i].vx = -10;
                                }
                                bullets[i].y = player.y + player_fase3->side/2 - frame.h - 10;
                                bullets[i].vy = -10;
                            }
                            // Tiro para cima
                            else if (key[ALLEGRO_KEY_UP]) {
                                bullets[i].x = player.x + 10;
                                bullets[i].y = player.y + player_fase3->side/2 - frame.h;
                                bullets[i].vx = 0;
                                bullets[i].vy = -15;
                            }
                            // Tiro abaixado
                            else if (key[ALLEGRO_KEY_DOWN] && no_chao_fase3) {
                                if (direcao_fase3 == 0) bullets[i].x = player.x + frame.w/2;
                                else bullets[i].x = player.x - frame.w/2;
                                bullets[i].y = player.y + player_fase3->side/2 - frame.h/2 - 5;
                                bullets[i].vx = (direcao_fase3 == 0) ? 15 : -15;
                                bullets[i].vy = 0;
                            }
                            // Tiro horizontal normal
                            else {
                                if (direcao_fase3 == 0) bullets[i].x = player.x + frame.w/2;
                                else bullets[i].x = player.x - frame.w/2;
                                bullets[i].y = player.y + player_fase3->side/2 - frame.h + 20;
                                bullets[i].vx = (direcao_fase3 == 0) ? 15 : -15;
                                bullets[i].vy = 0;
                            }
                            break;
                        }
                    }
                }

                // ========== ATUALIZA E DESENHA PROJÉTEIS ==========
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (bullets[i].ativa) {
                        bullets[i].x += bullets[i].vx;
                        bullets[i].y += bullets[i].vy;
                        al_draw_bitmap(bullet_boss_img, bullets[i].x - BULLET_BOSS_W/2, bullets[i].y, 0);
                        // Desativa projétil se sair da tela
                        if (bullets[i].x < -BULLET_BOSS_W || bullets[i].x > X_SCREEN + BULLET_BOSS_W ||
                            bullets[i].y < -BULLET_BOSS_H || bullets[i].y > Y_SCREEN + BULLET_BOSS_H) {
                            bullets[i].ativa = 0;
                        }
                    }
                }

                // ========== RECUPERAÇÃO DA ESTAMINA ==========
                if (!cansado_fase3) {
                    if (stamina_fase3 < stamina_max) {
                        stamina_recupera_tick_fase3++;
                        if (stamina_recupera_tick_fase3 >= 6) {
                            stamina_fase3 += 1;
                            if (stamina_fase3 > stamina_max) stamina_fase3 = stamina_max;
                            stamina_recupera_tick_fase3 = 0;
                        }
                    }
                } else {
                    stamina_fadiga_tick_fase3++;
                    if (stamina_fadiga_tick_fase3 >= 60) {
                        cansado_fase3 = false;
                        stamina_fadiga_tick_fase3 = 0;
                    }
                }

                // ========== LÓGICA DOS ESTADOS DO CHEFÃO DRAGÃO ==========
                // Quase morto: mudou sprite e comportamento
                if (vida_boss3 <= 2 && boss3_state != BOSS3_ALMOST_DEAD && boss3_state != BOSS3_DEAD) {
                    boss3_state = BOSS3_ALMOST_DEAD;
                    boss3_timer = 120;
                }
                // Estado parado: espera um tempo, depois começa ataque
                if (boss3_state == BOSS3_IDLE) {
                    if (boss3_timer == 0) boss3_timer = 180;
                    boss3_timer--;
                    if (boss3_timer <= 0 && boss3_state != BOSS3_ALMOST_DEAD && boss3_state != BOSS3_DEAD) {
                        boss3_state = BOSS3_ATTACK;
                        boss3_timer = 120;
                    }
                }
                // Estado de ataque: dispara chamas e verifica colisão com tiros do player
                else if (boss3_state == BOSS3_ATTACK) {
                    if (boss3_timer == 0) boss3_timer = 120;
                    boss3_timer--;

                    // Colisão dos tiros do player com dragão
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (bullets[i].ativa) {
                            float bullet_left = bullets[i].x - BULLET_BOSS_W/2;
                            float bullet_right = bullets[i].x + BULLET_BOSS_W/2;
                            float bullet_top = bullets[i].y;
                            float bullet_bottom = bullets[i].y + BULLET_BOSS_H;
                            float boss_left = boss3_x;
                            float boss_right = boss3_x + DRAGON_ATTACK_W * dragon_scale;
                            float boss_top = boss3_base_y + (DRAGON_IDLE_H - DRAGON_ATTACK_H) * dragon_scale;
                            float boss_bottom = boss_top + DRAGON_ATTACK_H * dragon_scale;
                            if (bullet_right > boss_left && bullet_left < boss_right &&
                                bullet_bottom > boss_top && bullet_top < boss_bottom) {
                                vida_boss3--;
                                bullets[i].ativa = 0;
                                if (vida_boss3 <= 0) {
                                    vida_boss3 = 0;
                                    boss3_state = BOSS3_DEAD;
                                    boss3_timer = 120;
                                }
                            }
                        }
                    }

                    // Dispara uma chama a cada 60 frames
                    fire_timer++;
                    if (fire_timer >= 60) {
                        fire_timer = 0;
                        for (int i = 0; i < MAX_FIRES; i++) {
                            if (fires[i][4] == 0) {
                                fires[i][4] = 1;
                                fires[i][2] = -14; // vx
                                fires[i][3] = 0;   // vy
                                fires[i][0] = boss3_x + DRAGON_ATTACK_W * dragon_scale * 0.3;
                                fires[i][1] = boss3_base_y + (DRAGON_IDLE_H - DRAGON_ATTACK_H) * dragon_scale + DRAGON_ATTACK_H * dragon_scale - FIRE_H * dragon_scale;
                                break;
                            }
                        }
                    }

                    if (boss3_timer <= 0 && boss3_state != BOSS3_ALMOST_DEAD && boss3_state != BOSS3_DEAD) {
                        boss3_state = BOSS3_IDLE;
                        boss3_timer = 180;
                    }
                }
                // Estado quase morto: aceita danos finais
                else if (boss3_state == BOSS3_ALMOST_DEAD) {
                    boss3_timer--;
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        if (bullets[i].ativa) {
                            float bullet_left = bullets[i].x - BULLET_BOSS_W/2;
                            float bullet_right = bullets[i].x + BULLET_BOSS_W/2;
                            float bullet_top = bullets[i].y;
                            float bullet_bottom = bullets[i].y + BULLET_BOSS_H;
                            float boss_left = boss3_x;
                            float boss_right = boss3_x + DRAGON_ALMOST_DEAD_W * dragon_scale;
                            float boss_top = boss3_base_y + (DRAGON_IDLE_H - DRAGON_ALMOST_DEAD_H) * dragon_scale;
                            float boss_bottom = boss_top + DRAGON_ALMOST_DEAD_H * dragon_scale;
                            if (bullet_right > boss_left && bullet_left < boss_right &&
                                bullet_bottom > boss_top && bullet_top < boss_bottom) {
                                vida_boss3--;
                                bullets[i].ativa = 0;
                                if (vida_boss3 <= 0) {
                                    vida_boss3 = 0;
                                    boss3_state = BOSS3_DEAD;
                                    boss3_timer = 120;
                                }
                            }
                        }
                    }
                    if (boss3_timer <= 0) {
                        boss3_state = BOSS3_DEAD;
                        boss3_timer = 120;
                    }
                }
                // Estado morto: mostra sprite final e tela de vitória
                else if (boss3_state == BOSS3_DEAD) {
                    boss3_timer--;
                    if (boss3_timer <= 0) {
                        tela_vitoria(disp, victory_img);
                        al_rest(1.5);
                        fase3_running = false;
                        state = MENU;
                        continue;
                    }
                }

                // ========== DESENHA O CHEFÃO CONFORME O ESTADO ==========
                if (boss3_state == BOSS3_IDLE) {
                    if (dragon_idle) al_draw_scaled_bitmap(
                        dragon_idle, 0, 0, DRAGON_IDLE_W, DRAGON_IDLE_H,
                        boss3_x, boss3_base_y, DRAGON_IDLE_W * dragon_scale, DRAGON_IDLE_H * dragon_scale, 0);
                } else if (boss3_state == BOSS3_ATTACK) {
                    if (dragon_attack) al_draw_scaled_bitmap(
                        dragon_attack, 0, 0, DRAGON_ATTACK_W, DRAGON_ATTACK_H,
                        boss3_x, boss3_base_y + (DRAGON_IDLE_H - DRAGON_ATTACK_H) * dragon_scale,
                        DRAGON_ATTACK_W * dragon_scale, DRAGON_ATTACK_H * dragon_scale, 0);
                } else if (boss3_state == BOSS3_ALMOST_DEAD) {
                    if (dragon_almost_dead) al_draw_scaled_bitmap(
                        dragon_almost_dead, 0, 0, DRAGON_ALMOST_DEAD_W, DRAGON_ALMOST_DEAD_H,
                        boss3_x, boss3_base_y + (DRAGON_IDLE_H - DRAGON_ALMOST_DEAD_H) * dragon_scale,
                        DRAGON_ALMOST_DEAD_W * dragon_scale, DRAGON_ALMOST_DEAD_H * dragon_scale, 0);
                } else if (boss3_state == BOSS3_DEAD) {
                    if (dragon_dead) al_draw_scaled_bitmap(
                        dragon_dead, 0, 0, DRAGON_DEAD_W, DRAGON_DEAD_H,
                        boss3_x, boss3_base_y + (DRAGON_IDLE_H - DRAGON_DEAD_H) * dragon_scale,
                        DRAGON_DEAD_W * dragon_scale, DRAGON_DEAD_H * dragon_scale, 0);
                }

                // ========== ATUALIZA/DESENHA CHAMAS DO DRAGÃO ==========
                for (int i = 0; i < MAX_FIRES; i++) {
                    if (fires[i][4]) {
                        fires[i][0] += fires[i][2];
                        fires[i][1] += fires[i][3];
                        al_draw_scaled_bitmap(
                            dragon_fire, 0, 0, FIRE_W, FIRE_H,
                            fires[i][0], fires[i][1], FIRE_W * dragon_scale, FIRE_H * dragon_scale, 0);

                        // Colisão chama x player
                        float player_left = player.x - frame.w/2;
                        float player_right = player.x + frame.w/2;
                        float player_top = player.y + player_fase3->side/2 - frame.h;
                        float player_bottom = player.y + player_fase3->side/2;

                        if (fires[i][0] < player_right &&
                            fires[i][0] + FIRE_W * dragon_scale > player_left &&
                            fires[i][1] < player_bottom &&
                            fires[i][1] + FIRE_H * dragon_scale > player_top) {
                            vida_player_fase3--;
                            fires[i][4] = 0;
                        }
                        // Desativa chama se sair da tela
                        if (fires[i][0] < -FIRE_W * dragon_scale)
                            fires[i][4] = 0;
                    }
                }

                // ========== GAME OVER: VIDA DO PLAYER ZEROU ==========
                if (vida_player_fase3 <= 0) {
                    tela_gameover(disp, gameover_img);
                    al_rest(1.5);
                    fase3_running = false;
                    state = MENU;
                    continue;
                }

                // ESC retorna ao menu
                if (key[ALLEGRO_KEY_ESCAPE]) {
                    fase3_running = false;
                    state = MENU;
                }

                // ========== HUD VIDA DO PLAYER (CORAÇÕES) ==========
                int vida_max = 20, num_coracoes = vida_max / 2;
                for (int i = 0; i < num_coracoes; i++) {
                    int tipo = (vida_player_fase3 >= (i+1)*2) ? 0 : (vida_player_fase3 == (i*2)+1) ? 2 : 1;
                    al_draw_bitmap_region(coracao_sprite, tipo * CORACAO_W, 0, CORACAO_W, CORACAO_H,
                        20 + i * (CORACAO_W + 5), 20, 0);
                }

                // ========== HUD DA VIDA DO CHEFÃO ==========
                struct Boss fake_boss3;
                memset(&fake_boss3, 0, sizeof(struct Boss));
                fake_boss3.vida = vida_boss3;
                int vida_col = boss_bar_col(fake_boss3);
                int vida_row = boss_bar_row(fake_boss3);
                al_draw_bitmap_region(boss_vida_sprite, vida_col * BOSS_VIDA_W, vida_row * BOSS_VIDA_H, BOSS_VIDA_W, BOSS_VIDA_H,
                    X_SCREEN - BOSS_VIDA_W - 20, 20, 0);

                // ========== HUD DA ESTAMINA ==========
                int bar_w = 200, bar_h = 20;
                float perc = (float)stamina_fase3 / stamina_max;
                al_draw_filled_rectangle(20, 80, 20 + bar_w * perc, 80 + bar_h, al_map_rgb(0,200,0));
                al_draw_rectangle(20, 80, 20 + bar_w, 80 + bar_h, al_map_rgb(0,0,0), 2);

                al_flip_display();
                al_rest(0.01); // Pequena pausa para controle do FPS
            }
            // Libera memória do quadrado do player ao sair da fase
            square_destroy(player_fase3);
            continue;
        }
    }

    // LIMPEZA DE MEMÓRIA
    al_destroy_bitmap(fogo_sprite);
    al_destroy_bitmap(bg_menu);
    al_destroy_bitmap(bullet_img);
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
    

   
