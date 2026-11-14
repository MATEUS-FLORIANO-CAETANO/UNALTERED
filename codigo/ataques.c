// padrões de ataque e sequência de ondas
// decide quando e onde as coisas nascem (via spawn_bala), 
// movimento e colisão fica em entidades.c


#include "game.h"

// um nome para cada padrão de ataque
typedef enum {
    PAD_CHUVA,
    /* ... */
} TipoAtaque;

// cada onda carrega o tipo do ataque, sua duracao (s), o intervalo entre ataques (s)
// e a escala das balas (multiplica hitbox e desenho do sprite)
typedef struct {
    TipoAtaque tipo;
    float duracao;
    float intervalo;
    float escala;
} Onda;

/*
 * PADRÕES
 * todos tem
 * g -> estado do jogo
 * t -> segundos desde o início da onda atual
 * intervalo -> tempo entre spawns, vindo da tabela de ondas
 * escala -> multiplicador do tamanho das balas, vindo da tabela de ondas
 */

// chuva: balas nascem acima da caixa em x aleatorio e caem reto pra baixo
static void padrao_chuva(Game *g, float t, float intervalo, float escala) {
    // 'static' guarda entre chamadas, e a janela (contexto GL) ja existe aqui.
    static Texture2D lagrima = { 0 };
    if (lagrima.id == 0) lagrima = LoadTexture("assets/lagrima.png");

    if (t < g->proxSpawn) return;        // ainda não é hora
    g->proxSpawn += intervalo;           // agenda o próximo spawn

    // hitbox base 10x10 (o png e 32x32, mas o desenho visivel e ~10x13),
    // tam multiplicado pela escala da onda
    Vector2 tam = { 10 * escala * GetRandomValue(1.1, 1.3), 10 * GetRandomValue(1.1, 1.3) };

    // faixa horizontal onde a hitbox pode nascer: interior da caixa (sem borda),
    // descontando a largura da hitbox pra não passar da parede direita
    int xMin = (int)(g->caixa.x + BORDA);
    int xMax = (int)(g->caixa.x + g->caixa.width - BORDA - tam.x);
    float x = (float)GetRandomValue(xMin, xMax);

    // nasce logo acima do topo da caixa e cai reto pra baixo
    float y = g->caixa.y - tam.y;
    // recorta = true: a chuva nasce acima da caixa, mas so aparece dentro dela
    spawn_bala(g, (Vector2){ x, y }, (Vector2){ 0, 120 }, tam, true, &lagrima, escala);
}

/*
static void padrao_circulo(Game *g, float t, float intervalo, float escala) {
    // ...
}
*/


// recebe o tipo do ataque e chama o padrão correspondente, repassando tempo, intervalo e escala
static void exec_padrao(Game *g, TipoAtaque tipo, float t, float intervalo, float escala) {
    switch (tipo) {
        case PAD_CHUVA:  padrao_chuva(g, t, intervalo, escala);  break;
        /* case PAD_X:   padrao_X(g, t, intervalo, escala);      break; */
        default:         break;
    }
}

/* 
 * nova onda: linha aqui
 * novo ataque: nome em enum, função padrao, case no switch
 */

static Onda ondas[] = {
    //  tipo        duracao  intervalo  escala
    { PAD_CHUVA,    8.0f,    0.25f,     1.0f },   // chuva leve
    { PAD_CHUVA,    8.0f,    0.40f,     2.0f },   // chuva pesada
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
    exec_padrao(g, atual.tipo, g->tempoOnda, atual.intervalo, atual.escala);

    // incrementa automaticamente indice em ondas[] assim que o ataque acabar
    if (g->tempoOnda > atual.duracao) {
        g->onda++;
        g->tempoOnda = 0;
        g->proxSpawn = 0;               
    }
    return g->onda >= NUM_ONDAS;
}