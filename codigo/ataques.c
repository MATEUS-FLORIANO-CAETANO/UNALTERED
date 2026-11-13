// padrões de ataque e sequência de ondas
// decide quando e onde as coisas nascem (via spawn_*), 
// movimento e colisão fica em entidades.c

// AINDA TEM QUE ORGANIZAR NO game.h float proxSpawn, Rectangle caixa

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

// chuva
static void padrao_chuva(Game *g, float t, float intervalo) {
    /*
    // ver se já é hora do próximo spawn
    if (t < g->proxSpawn) return;        // ainda não é hora
    g->proxSpawn += intervalo;           // agenda o próximo spawn
    */

    // spawn_bala(g, GetRandomValue() em lugar acima da caixa, pra baixo)
}

/*
static void padrao_X(Game *g, float t, float intervalo) {
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


/* TABELA
 * nova onda: linha aqui
 * novo ataque: nome em enum, função padrao, case no switch
 */

static Onda ondas[] = {
    //  tipo        duracao  intervalo
    { PAD_CHUVA,    8.0f,    0.25f },   // chuva leve
    { PAD_CHUVA,    8.0f,    0.10f },   // chuva pesada
};

// quantidade de ondas = tamanho total / tamanho de cada
#define NUM_ONDAS ((int)(sizeof(ondas) / sizeof(ondas[0])))


// Chamar 1x por frame no main.c. Retorna true quando acabar ondas (implementar vitoria)

bool att_ondas(Game *g, float dt) {
    // se já passou da última onda
    // if (g->onda >= NUM_ONDAS) return true;

    // tempoOnda soma dt todo frame
    // g->tempoOnda += dt;

    // chama exec_padrao com tipo, tempo e intervalo da onda atual

    // se passou da duração: próxima onda, zera tempoOnda e proxSpawn

    return false;   // só pra compilar
}