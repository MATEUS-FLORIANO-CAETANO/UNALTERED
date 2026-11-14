// padrões de ataque e sequência de ondas
// decide quando e onde as coisas nascem (via spawn_bala), 
// movimento e colisão fica em entidades.c


#include "game.h"

// um nome para cada padrão de ataque
typedef enum {
    PAD_CHUVA,
    /* ... */
} TipoAtaque;

// cada onda carrega o tipo do ataque, sua duracao (s) e intervalo entre ataques (s)
typedef struct {
    TipoAtaque tipo;
    float duracao;    
    float intervalo;   
} Onda;

/*
 * PADRÕES 
 * todos tem
 * g -> estado do jogo
 * t -> segundos desde o início da onda atual
 * intervalo -> tempo entre spawns, vindo da tabela de ondas
 */

// chuva: balas nascem acima da caixa em x aleatorio e caem reto pra baixo
static void padrao_chuva(Game *g, float t, float intervalo) {
    // carrega o sprite da gota uma vez; fica na memoria ate o jogo fechar.
    // 'static' guarda entre chamadas, e a janela (contexto GL) ja existe aqui.
    static Texture2D lagrima = { 0 };
    if (lagrima.id == 0) lagrima = LoadTexture("assets/lagrima.png");

    // ver se já é hora do próximo spawn
    if (t < g->proxSpawn) return;        // ainda não é hora
    g->proxSpawn += intervalo;           // agenda o próximo spawn

    // hitbox da gota menor que a imagem: o png e 32x32, o desenho visivel e ~10x13.
    // dobrada (20x20) junto com o desenho (escala 2) pra deixar as gotas maiores.
    Vector2 tam = { 20, 20 };

    // faixa horizontal onde a hitbox pode nascer: interior da caixa (sem borda),
    // descontando a largura da hitbox pra não passar da parede direita
    int xMin = (int)(g->caixa.x + BORDA);
    int xMax = (int)(g->caixa.x + g->caixa.width - BORDA - tam.x);
    float x = (float)GetRandomValue(xMin, xMax);

    // nasce logo acima do topo da caixa e cai reto pra baixo
    float y = g->caixa.y - tam.y;
    // recorta = true: a chuva nasce acima da caixa, mas so aparece dentro dela
    spawn_bala(g, (Vector2){ x, y }, (Vector2){ 0, 120 }, tam, true, &lagrima, 2.0f);
}

/*
static void padrao_circulo(Game *g, float t, float intervalo) {
    // ...
}
*/


// recebe o tipo do ataque e chama o padrão correspondente, repassando o mesmo tempo e intervalo
static void exec_padrao(Game *g, TipoAtaque tipo, float t, float intervalo) {
    switch (tipo) {
        case PAD_CHUVA:  padrao_chuva(g, t, intervalo);  break;
        /* case PAD_X:   padrao_X(g, t, intervalo);      break; */
        default:         break;
    }
}


/* 
 * nova onda: linha aqui
 * novo ataque: nome em enum, função padrao, case no switch
 */

static Onda ondas[] = {
    //  tipo        duracao  intervalo
    { PAD_CHUVA,    8.0f,    0.50f },   // chuva leve
    { PAD_CHUVA,    8.0f,    0.25f },   // chuva pesada
};

// quantidade de ondas = tamanho total / tamanho de cada
#define NUM_ONDAS ((int)(sizeof(ondas) / sizeof(ondas[0])))


// Chamadp 1x por frame no main.c. Retorna true quando acabar ondas (implementar vitoria)
bool att_ondas(Game *g, float dt) {
    // se já passou da última onda
    if (g->onda >= NUM_ONDAS) return true;
    // tempoOnda soma dt todo frame
    g->tempoOnda += dt;

    Onda atual = ondas[g->onda];
    exec_padrao(g, atual.tipo, g->tempoOnda, atual.intervalo);

    // incrementa automaticamente indice em ondas[] assim que o ataque acabar
    if (g->tempoOnda > atual.duracao) {
        g->onda++;
        g->tempoOnda = 0;
        g->proxSpawn = 0;               
    }
    return g->onda >= NUM_ONDAS;
}