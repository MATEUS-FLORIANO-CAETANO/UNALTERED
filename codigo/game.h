#ifndef GAME_H
#define GAME_H
#include "raylib.h"
#define MAX_BALAS 256
#define BORDA 5

// resolucao virtual em que todo o jogo e desenhado (antes do upscale da janela)
#define VIRTUAL_W 640
#define VIRTUAL_H 480

// guarda declaracoes das funcoes pro compilador. todo arquivo inclui game.h 
// (compilador copia e cola os conteudos de game.h nos arquivos)
  
typedef enum { ESTADO_JOGANDO, ESTADO_MORTE, ESTADO_VITORIA } estadoGame;

// recorta = true -> so e desenhada dentro da caixa (ex: chuva). o ataque decide.
typedef struct { Rectangle r; Vector2 vel; bool ativo; bool recorta; /*Texture2D textura;*/ } Bala;

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

    int onda; 
    float tempoOnda, proxSpawn;
} Game;

// declaracoes: existem em entidades.c
void spawn_bala(Game *g, Vector2 pos, Vector2 vel, bool recorta);
void att_entidades(Game *g, float dt);
void desenha_entidades(Game *g);

// declaracao: existe em ataques.c
bool att_ondas(Game *g, float dt);

#endif