// padrões de ataque e sequência de ondas
// decide quando e onde as coisas nascem (via spawn_bala), 
// movimento e colisão fica em entidades.c


#include "game.h"
#include <math.h>   // trig do circulo

// um nome para cada padrão
typedef enum {
    PAD_CHUVA,
    PAD_CIRCULO,
    PAD_BARREIRA,
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

 // -------------------------------------------------------------------------------

// chuva: balas nascem acima da caixa em x aleatorio e caem reto pra baixo
static void padrao_chuva(Game *g, float t, float intervalo, float escala) {
    // 'static' guarda entre chamadas pra evitar carregar a textura repetidamente.
    static Texture2D lagrima = { 0 };
    if (lagrima.id == 0) lagrima = LoadTexture("assets/lagrima.png");

    if (t < g->proxSpawn) return;        // ainda não é hora
    g->proxSpawn += intervalo;           // agenda o próximo spawn

    // hitbox base 10x10 (o png e 32x32, mas o desenho visivel e ~10x13),
    // tam multiplicado pela escala da onda
    Vector2 tam = { 10 * escala * GetRandomValue(110, 130) / 100.0f, 10 * escala * GetRandomValue(110, 130) / 100.0f };

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

// ---------------------------------------------------------------------------------


// circulo (jevil): a cada 'intervalo' nasce um anel de N balas em volta do centro
// da caixa. a bala k espera k*ATRASO segundos e entao acelera em linha reta pro
// centro. como cada uma sai um pouco depois da vizinha anti-horaria, a "frente"
// parece girar com velocidade angular (2*PI/N) / ATRASO.
static void padrao_circulo(Game *g, float t, float intervalo, float escala) {
    // textura das balas do anel. descomentar qnd existir e
    // trocar o NULL do spawn_bala por &jevil.
    // static Texture2D jevil = { 0 };
    // if (jevil.id == 0) jevil = LoadTexture("assets/jevil.png");

    if (t < g->proxSpawn) return;
    // qual anel e este (0, 1, 2...). roundf pq a soma de floats acumula erro
    int anel = (int)roundf(g->proxSpawn / intervalo);
    g->proxSpawn += intervalo;

    const int   N      = 8;
    const float R      = 110.0f;   // raio. caixa ta em y~270..410 e a tela vai ate 480:
    const float ACEL   = 300.0f;   // px/s ao quadradp
    const float ATRASO = 0.10f;    // s entre uma bala e a vizinha (volta inteira = N*ATRASO)

    Vector2 tam = { 8 * escala, 8 * escala };

    // centro da caixa
    Vector2 c = { g->caixa.x + g->caixa.width  / 2,
                  g->caixa.y + g->caixa.height / 2 };

    // aneis impares giram meio passo, pra nao nascerem em cima das balas
    // do anel anterior que ainda estao esperando
    float giro = (anel % 2) * (PI / N);

    int horario = GetRandomValue(0, 1);

    for (int k = 0; k < N; k++) {
        // k=0 em 0 rad (direita do centro), crescendo anti-horario
        float th = k * 2.0f * PI / N + giro;
        float cs = cosf(th), sn = sinf(th);

        // y da tela cresce pra baixo: o MENOS no seno mantem o anti-horario
        Vector2 centroBala = { c.x + R * cs, c.y - R * sn };

        // spawn_bala recebe o canto superior esquerdo, nao o centro
        Vector2 canto = { centroBala.x - tam.x / 2, centroBala.y - tam.y / 2 };

        Bala *b = spawn_bala(g, canto, (Vector2){ 0, 0 }, tam, false, NULL, escala);  // &jevil
        if (b == NULL) return;   // array cheio

        // (-cos, +sin) = vetor que aponta da bala pro centro, ja com tamanho 1
        b->acel   = (Vector2){ -ACEL * cs, ACEL * sn };
        b->espera = (horario ? (N - 1 - k) : k) * ATRASO;
    }
}

// ---------------------------------------------------------------------------------

// barreira (sans / "flappy bird"): a cada 'intervalo', duas balas finas e altas no
// mesmo x cobrem toda a altura interna da caixa, menos uma fresta. o par atravessa
// a caixa de lado. a fresta de cada par fica perto da anterior, pra sempre dar
// tempo da alma chegar nela.
static void padrao_barreira(Game *g, float t, float intervalo, float escala) {

    // textura das barreiras. descomentar quando o pixel art existir e 
    // trocar o NULL dos spawn_bala por &barreira.

    // static Texture2D barreira = { 0 };
    // if (barreira.id == 0) barreira = LoadTexture("assets/barreira.png");

    // topo da fresta do par anterior. 'static' guarda entre chamadas, e
    // proxSpawn == 0 so acontece na primeira chamada de cada onda
    static float frestaY;

    const float VEL    = 140.0f;               // px/s pro lado
    const int   DIR    = 1;                   // +1 esquerda->direita, -1 direita->esquerda
    const float LARG   = 12 * escala;          // largura da barreira
    const float FRESTA = 2.0f * TAM_ALMA;     // altura da fresta (24 px)

    float topo  = g->caixa.y + BORDA;                    // limites internos da caixa
    float fundo = g->caixa.y + g->caixa.height - BORDA;

    if (g->proxSpawn == 0) frestaY = (topo + fundo - FRESTA) / 2;  // 1o par: fresta no meio

    if (t < g->proxSpawn) return;
    g->proxSpawn += intervalo;

    // quanto a fresta pode mudar de um par pro proximo: o que a alma anda
    // em 'intervalo' segundos, com 30% de folga pra reacao do jogador
    int passo = (int)(VEL_JOGADOR * intervalo * 0.7f);
    frestaY += GetRandomValue(-passo, passo);

    // nao deixa a fresta sair da caixa
    if (frestaY < topo)           frestaY = topo;
    if (frestaY > fundo - FRESTA) frestaY = fundo - FRESTA;

    // nasce logo fora da parede de entrada; recorta = true esconde ate entrar
    float x = (DIR > 0) ? g->caixa.x + BORDA - LARG
                        : g->caixa.x + g->caixa.width - BORDA;
    Vector2 vel = { DIR * VEL, 0 };

    // de cima: do topo da caixa ate a fresta (pula se a fresta encosta no topo)
    float hCima = frestaY - topo;
    if (hCima > 0) {
        Bala *b = spawn_bala(g, (Vector2){ x, topo }, vel, (Vector2){ LARG, hCima }, true, NULL, escala);  // &barreira
        if (b) b->persiste = true;
    }

    // de baixo: do fim da fresta ate o fundo da caixa
    float yBaixo = frestaY + FRESTA;
    float hBaixo = fundo - yBaixo;
    if (hBaixo > 0) {
        Bala *b = spawn_bala(g, (Vector2){ x, yBaixo }, vel, (Vector2){ LARG, hBaixo }, true, NULL, escala);  // &barreira
        if (b) b->persiste = true;
    }
}


// recebe o tipo do ataque e chama o padrão correspondente, repassando tempo, intervalo e escala
static void exec_padrao(Game *g, TipoAtaque tipo, float t, float intervalo, float escala) {
    switch (tipo) {
        case PAD_CHUVA:    padrao_chuva(g, t, intervalo, escala);    break;
        case PAD_CIRCULO:  padrao_circulo(g, t, intervalo, escala);  break;
        case PAD_BARREIRA: padrao_barreira(g, t, intervalo, escala); break;
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
   // { PAD_CHUVA,    8.0f,    0.25f,     1.0f },   // chuva leve
   // { PAD_CHUVA,    8.0f,    0.40f,     2.0f },   // chuva pesada
   //  { PAD_CIRCULO,  8.0f,    2.0f,      1.0f },   // circulo do jevil
    { PAD_BARREIRA, 8.0f,    0.5f,      1.0f },   // barreiras do sans
};

// quantidade de ondss = tamanho total / tamanho de cada
#define NUM_ONDAS ((int)(sizeof(ondas) / sizeof(ondas[0])))


// chamadp 1x por frame no main.c. Retorna true quando acabar ondas (implementar vitoria)
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