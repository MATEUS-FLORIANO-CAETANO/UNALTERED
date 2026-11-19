#ifndef GAME_H
#define GAME_H
#include "raylib.h"
#include <stddef.h>   // NULL
#define MAX_BALAS 256
#define BORDA 5

// resolucao virtual em que todo o jogo e desenhado (antes do upscale da janela)
#define VIRTUAL_W 640
#define VIRTUAL_H 480

#define TAM_ALMA     16
#define HITBOX_ALMA  8
#define VEL_JOGADOR  120   // px/s

extern Sound dano;

// guarda declaracoes das funcoes pro compilador. todo arquivo inclui game.h 
// (compilador copia e cola os conteudos de game.h nos arquivos)
  
typedef enum { ESTADO_JOGANDO, ESTADO_MORTE, ESTADO_VITORIA } estadoGame;

// r = hitbox (posicao + tamanho). o sprite e desenhado centralizado nela, pois a
// imagem costuma ser maior que a parte visivel (ex: gota ~10x13 num png 32x32).

// campos que o ataque preenche. o que nao for preenchido vale 0 / false / NULL.
typedef struct {
    Rectangle r;          // hitbox: canto superior esquerdo + largura/altura
    Vector2 vel;          // 
    Vector2 acel;         //  somada na vel todo frame
    float segue;          // segs. acompanhando a alma, antes da espera
    float espera;         // segs. parada
    float vida;           // segs. ate sumir (0 = nao expira)

    float persegue;       // CLAUDE: rad/s que a vel gira em direcao a alma (0 = reta). mantem
                          // o modulo da vel; precisa de vel != 0. ex: 2.0 = curva suave

    bool persiste;        // true = nao some ao acertar
    bool recorta;         // true = so aparece dentro da caixa
    bool laser;           // true = linha de aviso roxa na direcao da acel enquanto
                          // espera ou recua (some quando comeca a ir pra frente)
    bool rebativel;       // true = da pra dar parry
    int rebates;          // vezes que o inimigo ainda devolve

    Texture2D *textura;   // NULL = quadrado branco
    float escala;         // multiplica tam do desenho

    bool ativo;           
    Vector2 desloc;       // distancia ate a alma guardada no spawn (usada por 'segue')
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
    bool parry;        // permitir parry?
    int contador_inimigo; // balas de parry ja lancadas
    float anim_inimigo;   // segs. de piscada restantes
    float janela_parry;   // segs. de hitbox de parry ativa restantes
} Game;

#define LINHA_INIMIGO 170   // base dos olhos (y=130, raio 40)
#define JANELA_PARRY 0.2f   // segs. de hitbox ativa por aperto
#define MULT_REBATE 1.3f    // vel. quando o inimigo devolve

// declaracoes: usei em entidades.c
void spawn_bala(Game *g, Bala nova);
void att_entidades(Game *g, float dt);
void desenha_entidades(Game *g);

// declaracao: usei em ataques.c
bool att_ondas(Game *g, float dt);

// declaracoes: parry.c
void att_parry(Game *g, bool apertou, float dt);
void desenha_parry(Game *g);

#endif