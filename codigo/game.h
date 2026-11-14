#ifndef GAME_H
#define GAME_H
#include "raylib.h"
#include <stddef.h>   // NULL
#define MAX_BALAS 256
#define BORDA 5

// resolucao virtual em que todo o jogo e desenhado (antes do upscale da janela)
#define VIRTUAL_W 640
#define VIRTUAL_H 480

// alma: sprite 16x16 (assets/alma.png), hitbox 8x8 no centro.
// ficam aqui pra os ataques poderem calcular frestas alcancaveis.
#define TAM_ALMA     16
#define HITBOX_ALMA  8
#define VEL_JOGADOR  120   // px/s

// guarda declaracoes das funcoes pro compilador. todo arquivo inclui game.h 
// (compilador copia e cola os conteudos de game.h nos arquivos)
  
typedef enum { ESTADO_JOGANDO, ESTADO_MORTE, ESTADO_VITORIA } estadoGame;

// r = hitbox (posicao + tamanho). o sprite e desenhado centralizado nela, pois a
// imagem costuma ser maior que a parte visivel (ex: gota ~10x13 num png 32x32).

// campos que o ataque preenche. o que nao for preenchido vale 0 / false / NULL.
typedef struct {
    Rectangle r;          // hitbox: canto superior esquerdo + largura/altura
    Vector2 vel;          // px/s
    Vector2 acel;         // px/s², somada na vel todo frame
    float espera;         // segundos parada antes de comecar a se mover
    bool persiste;        // true = nao some ao acertar a alma (barreiras, ossos)
    bool recorta;         // true = so aparece dentro da caixa
    Texture2D *textura;   // NULL = quadrado branco
    float escala;         // tamanho do desenho do sprite (0 = 1)
    bool ativo;           // controlado por spawn_bala/att_entidades, o ataque nao mexe
} Bala;

// prototipo: todas as informacoes que cada frame precisa saber. 
// declarado aqui, inicializado em main.c, passa referencia por todas funcoes
typedef struct {
    estadoGame estado; 
    Rectangle caixa; 
    Vector2 jogador; 
    int hp; 
    float iframes;
    
    // instancia 256 balas, cada uma com estado ativo/inativo
    // assim, nao precisa alocar dinamicamente
    Bala balas[MAX_BALAS];

    int onda;          // indice em ondas[] (ataques.c)
    float tempoOnda;   // segundos desde o inicio da onda atual
    int disparo;       // quantas vezes o padrao da onda atual ja disparou
} Game;

// declaracoes: existem em entidades.c
// copia 'nova' pra um slot livre. uso:
//   spawn_bala(g, (Bala){ .r = {x, y, w, h}, .vel = {0, 120}, .recorta = true });
// bala sem tamanho (w ou h <= 0) e ignorada; array cheio tambem.
void spawn_bala(Game *g, Bala nova);
void att_entidades(Game *g, float dt);
void desenha_entidades(Game *g);

// declaracao: existe em ataques.c
bool att_ondas(Game *g, float dt);

#endif