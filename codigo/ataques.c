/*
 * IDEIAS DE ATAQUE, faça o que achar mais legal
 *
 * variacoes do que ja tem, ex.:
 * - chuva lateral
 * - o mesmo das facas, mas com segue diferente (1.0, 1.3, 1.6...), com varias facas ao msm tempo
 * - facas em cruz (uma horiz. e outra vert.), congelam e fecham ao msm tempo
 *
 *eu implementei um campo pra vida (duracao que a bala fica na tela)
 *e um campo pra persegue (que faz a bala girar na direcao do player e acompanhar)
 *nao usei nenhum desses 2 em ataques ainda, mas com eles da pra fazer:
 *
 *- uma "gaiola", circulo grandao com centro em canto aleatorio da arena que fecha até quase largura do player, dps some
 *- enxame, acompanha pouco mas sao muitas
 *- bala seguindo atras da alma bem rapido, nao pode ficar parado
 *
 *
 *em geral:
 *- golpe que cai do ceu com pouca vel pra baixo e muita vel lateral, quando chega na borda da caixa inverte, como se tivesse quicando
 *- chuva que cai tudo de uma vez, mas marca com laser antes pra encontrar a "folga"
 *- balas que deixam rastro
 *- n balas no centro, velocidade pra fora "explosao"
 *
*/
// padrões de ataque e sequência de ondas
// decide quando e onde as coisas nascem (via spawn_bala),
// movimento e colisão fica em entidades.c

#include "game.h"
#include "raymath.h"   // Vector2Subtract, Vector2Normalize, Vector2Scale, Clamp
#include <math.h>      // cosf, sinf

/*
 * COMO ESCREVER UM ATAQUE
 *
 * um padrao e uma funcao  static void padrao_x(Game *g, Onda o, int n)
 * que o att_ondas chama a cada o.intervalo segundos. ela so cria balas:
 *
 *   g -> estado do jogo (caixa, jogador...)
 *   o -> a linha da tabela ondas[] (o.escala, o.intervalo, o.duracao)
 *   n -> numero do disparo nessa onda: 0 no primeiro, 1 no segundo...
 *        (n == 0 serve pra resetar variaveis 'static' no comeco da onda)
 *
 * pra criar uma bala, preenche so os campos que importam (o resto vale 0):
 *
 *   spawn_bala(g, (Bala){
 *       .r        = { x, y, largura, altura },   // ou ret_centrado(centro, w, h)
 *       .vel      = { 0, 120 },                  // px/s
 *       .acel     = { 0, 0 },                    // px/s²
 *       .espera   = 0.5f,                        // s parada antes de sair
 *       .persiste = true,                        // nao some ao acertar
 *       .recorta  = true,                        // so aparece dentro da caixa
 *       .textura  = &sprite, .escala = o.escala,
 *   });
 *
 * depois: nome no enum, case no switch, linha na tabela ondas[].
 */

// um nome para cada padrão
typedef enum {
    PAD_CHUVA,
    PAD_CIRCULO,
    PAD_BARREIRA,
    PAD_ESPINGARDA,
    PAD_FACA,
    /* ... */
} TipoAtaque;

// cada onda carrega o tipo do ataque, sua duracao (s), o intervalo entre disparos (s)
// e a escala das balas
typedef struct {
    TipoAtaque tipo;
    float duracao;
    float intervalo;
    float escala;
} Onda;


// ---------------------------------------------------------------------------------
// AJUDANTES: contas de geometria que todo padrao precisa

// area jogavel: a caixa sem a borda
static Rectangle interior(Game *g) {
    return (Rectangle){ g->caixa.x + BORDA, g->caixa.y + BORDA,
                        g->caixa.width - 2 * BORDA, g->caixa.height - 2 * BORDA };
}

// centro de um retangulo (ex: centro(g->caixa), centro(interior(g)))
static Vector2 centro(Rectangle r) {
    return (Vector2){ r.x + r.width / 2, r.y + r.height / 2 };
}

// hitbox de tamanho w x h com o centro em c (Rectangle usa o canto, nao o centro)
static Rectangle ret_centrado(Vector2 c, float w, float h) {
    return (Rectangle){ c.x - w / 2, c.y - h / 2, w, h };
}

// ponto num circulo de centro c e raio r. ang em radianos:
// 0 = direita, PI/2 = cima, PI = esquerda... (anti-horario na tela)
static Vector2 no_circulo(Vector2 c, float r, float ang) {
    // y da tela cresce pra baixo, por isso o MENOS no seno
    return (Vector2){ c.x + r * cosf(ang), c.y - r * sinf(ang) };
}

// vetor de tamanho 'modulo' apontando de 'de' para 'para'.
// serve pra vel (modulo em px/s) ou acel (px/s²) mirando em algo.
static Vector2 rumo(Vector2 de, Vector2 para, float modulo) {
    return Vector2Scale(Vector2Normalize(Vector2Subtract(para, de)), modulo);
}


// ---------------------------------------------------------------------------------
// PADROES

// chuva: balas nascem acima da caixa em x aleatorio e caem reto pra baixo
static void padrao_chuva(Game *g, Onda o, int n) {
    
    (void)n;   // a chuva nao depende de qual disparo esta
    static Texture2D lagrima = { 0 };   // static: carrega a textura uma vez so
    if (lagrima.id == 0) lagrima = LoadTexture("assets/lagrima.png");

    // tamanho aleatorio: +10% a +100%. um sorteio so, usado na hitbox E no sprite,
    // pra o desenho crescer junto (o sprite tem escala uniforme, nao da pra esticar w e h separados)
    float fator = GetRandomValue(110, 200) / 100.0f;
    float escala = o.escala * fator;

    // hitbox base 10x10 (o png e 32x32, mas o desenho visivel e ~10x13)
    float w = 10 * escala;
    float h = 10 * escala;

    // x aleatorio dentro da caixa, sem passar da parede direita
    Rectangle in = interior(g);
    float x = (float)GetRandomValue((int)in.x, (int)(in.x + in.width - w));

    spawn_bala(g, (Bala){
        .r       = { x, g->caixa.y - h, w, h },   // logo acima da caixa
        .vel     = { 0, 120 },
        .recorta = true,                          // so aparece quando entra
        .textura = &lagrima,
        .escala  = escala,                       
    });
}

// ---------------------------------------------------------------------------------

// circulo (jevil): a cada disparo nasce um anel de N balas em volta do centro da
// caixa. a bala k espera k*ATRASO segundos e entao acelera em linha reta pro centro.
static void padrao_circulo(Game *g, Onda o, int n) {

    // static Texture2D jevil = { 0 };
    // if (jevil.id == 0) jevil = LoadTexture("assets/jevil.png");

    const int   N      = 8;
    const float RAIO   = 110.0f;   // acima de ~120 a bala de baixo nasce fora da tela
    const float ACEL   = 200.0f;   // px/s²
    const float ATRASO = 0.10f;    // s entre uma bala e a vizinha (volta = N*ATRASO)
    // recuo (telegrafa o ataque): a bala sai pra FORA com essa vel e a acel pro centro
    // freia ela ate voltar. recua RECUO²/(2*ACEL) px (~9px) por RECUO/ACEL s (~0.3s).
    const float RECUO  = 60.0f;    // px/s
    const float TAM    = 8 * o.escala;

    Vector2 c = centro(g->caixa);

    float giro = (n % 2) * (PI / N);
    int horario = GetRandomValue(0, 1);

    for (int k = 0; k < N; k++) {
        Vector2 pos = no_circulo(c, RAIO, giro + k * 2 * PI / N);
        int ordem = horario ? (N - 1 - k) : k;   // quem sai primeiro

        spawn_bala(g, (Bala){
            .r      = ret_centrado(pos, TAM, TAM),
            .vel    = rumo(c, pos, RECUO),        // comeca indo pra fora (recuo)
            .acel   = rumo(pos, c, ACEL),         // acelera em direcao ao centro
            .espera = ordem * ATRASO,
            .escala = o.escala,
            // .textura = &jevil,
        });
    }
}

// ---------------------------------------------------------------------------------

// barreira (sans / "flappy bird"): a cada disparo, duas balas finas e altas no mesmo
// x cobrem a altura da caixa menos uma fresta, e atravessam a caixa de lado.
// a fresta de cada par fica perto da anterior, pra sempre dar tempo de alcancar.
static void padrao_barreira(Game *g, Onda o, int n) {
    // static Texture2D barreira = { 0 };
    // if (barreira.id == 0) barreira = LoadTexture("assets/barreira.png");

    const float VEL    = 140.0f;          // px/s pro lado
    const int   DIR    = 1;               // +1 esquerda->direita, -1 direita->esquerda
    const float LARG   = 12 * o.escala;   // largura da barreira
    const float FRESTA = 2.0f * TAM_ALMA; // altura da fresta

    Rectangle in = interior(g);
    float topo  = in.y;
    float fundo = in.y + in.height;

    // topo da fresta do par anterior (static: guarda entre disparos)
    static float frestaY;
    if (n == 0) frestaY = centro(in).y - FRESTA / 2;   // 1o par: fresta no meio

    // a fresta anda no maximo o que a alma anda em um intervalo (com 30% de folga)
    int passo = (int)(VEL_JOGADOR * o.intervalo * 0.7f);

    // puxa a fresta de volta pro meio: quanto mais longe, mais forte o puxao
    // (0 = passeio aleatorio puro, 1 = volta pro meio de uma vez)
    const float PUXAO = 0.35f;
    float meio  = centro(in).y - FRESTA / 2;
    float passoY = GetRandomValue(-passo, passo) + (meio - frestaY) * PUXAO;
    passoY = Clamp(passoY, -passo, passo);   // nunca pula mais do que a alma alcanca
    frestaY = Clamp(frestaY + passoY, topo, fundo - FRESTA);

    // nasce logo fora da parede de entrada (.recorta esconde ate entrar)
    float x = (DIR > 0) ? in.x - LARG : in.x + in.width;
    Vector2 vel = { DIR * VEL, 0 };

    // de cima: do topo ate a fresta
    spawn_bala(g, (Bala){
        .r = { x, topo, LARG, frestaY - topo },
        .vel = vel, .recorta = true, .persiste = true,
        .escala = o.escala,   // .textura = &barreira,
    });

    // de baixo: do fim da fresta ate o fundo
    float yBaixo = frestaY + FRESTA;
    spawn_bala(g, (Bala){
        .r = { x, yBaixo, LARG, fundo - yBaixo },
        .vel = vel, .recorta = true, .persiste = true,
        .escala = o.escala,   // .textura = &barreira,
    });
}

// ---------------------------------------------------------------------------------

// espingarda: a cada disparo, um leque de K balas mirado na alma, saindo de fora da
// caixa. alterna esquerda/direita; a altura e aleatoria em ate 0.75 caixa acima ou
// abaixo do centro (cobre 1.5 caixa). sem recorte: da pra ver a bala chegando.
static void padrao_espingarda(Game *g, Onda o, int n) {
    const int   K      = 6;                 // balas por disparo
    const float ABERTO = 60 * DEG2RAD;      // abertura total do leque
    const float VEL    = 140.0f;            // px/s
    const float DIST   = 50.0f;             // distancia da boca ate a parede da caixa
    const float TAM    = 6 * o.escala;

    Vector2 c = centro(g->caixa);
    int esq   = (n % 2 == 0);               // par = esquerda, impar = direita
    float x   = esq ? g->caixa.x - DIST : g->caixa.x + g->caixa.width + DIST;
    int faixa = (int)(0.75f * g->caixa.height);
    float y   = c.y + GetRandomValue(-faixa, faixa);
    Vector2 boca = { x, y };

    Vector2 mira = rumo(boca, g->jogador, VEL);   // mira na alma

    // com K par desloca o leque meio passo pra um lado (sorteado),
    // pra sempre ter uma bala exatamente na mira.
    float passo = ABERTO / (K - 1);
    float desvio = (K % 2 == 0) ? (GetRandomValue(0, 1) ? 0.5f : -0.5f) * passo : 0;

    for (int k = 0; k < K; k++) {
        float ang = -ABERTO / 2 + k * passo + desvio;
        spawn_bala(g, (Bala){
            .r      = ret_centrado(boca, TAM, TAM),
            .vel    = Vector2Rotate(mira, ang),
            .escala = o.escala,
        });
    }
}


// ---------------------------------------------------------------------------------

// faca ("hitscan"): nasce num ponto aleatorio em volta da caixa apontada pra alma,
// mostra a linha de aviso roxa, recua um pouco (como no circulo) e dispara muito rapido.
// a hitbox e uma fileira de M quadradinhos com o mesmo movimento, formando a lamina
// (Rectangle nao gira). de quebra, como a fileira anda no sentido do proprio
// comprimento, ela nao "pula" a alma entre um frame e outro mesmo em alta velocidade.
static void padrao_faca(Game *g, Onda o, int n) {
    (void)n;

    const float RAIO  = 120.0f;   // distancia do centro da caixa onde a faca nasce
    const float SEGUE = 0.6f;     // s acompanhando a alma (mesma distancia, mesma mira)
    const float AVISO = 0.0f;     // s congelada antes de disparar
    const float ACEL  = 55000.0f;  // px/s²
    // recua RECUO²/(2*ACEL) px (~12px) por RECUO/ACEL s (~0.1s)
    const float RECUO = 250.0f;   // px/s
    const int   M     = 10;       // quadradinhos na lamina
    const float PASSO = 4 * o.escala;   // distancia entre eles (comprimento ~ M*PASSO)
    const float TAM   = 5 * o.escala;   // lado de cada quadradinho

    Vector2 c = centro(g->caixa);
    float ang = GetRandomValue(0, 359) * DEG2RAD;
    Vector2 ponta = no_circulo(c, RAIO, ang);
    Vector2 dir = rumo(ponta, g->jogador, 1);   // unitario, mirando na alma

    for (int k = 0; k < M; k++) {
        // k = 0 e a ponta; o resto vai ficando pra tras ao longo da lamina
        Vector2 pos = Vector2Subtract(ponta, Vector2Scale(dir, k * PASSO));
        spawn_bala(g, (Bala){
            .r        = ret_centrado(pos, TAM, TAM),
            .vel      = Vector2Scale(dir, -RECUO),   // recua antes de ir
            .acel     = Vector2Scale(dir, ACEL),
            .segue    = SEGUE,
            .espera   = AVISO,
            .persiste = true,                        // a lamina nao se desmancha
            .laser    = (k == 0),                    // so a ponta desenha a linha
            .escala   = o.escala,
        });
    }
}

// ---------------------------------------------------------------------------------

// chama o padrao do tipo da onda
static void exec_padrao(Game *g, Onda o, int n) {
    switch (o.tipo) {
        case PAD_CHUVA:    padrao_chuva(g, o, n);    break;
        case PAD_CIRCULO:  padrao_circulo(g, o, n);  break;
        case PAD_BARREIRA: padrao_barreira(g, o, n); break;
        case PAD_ESPINGARDA: padrao_espingarda(g, o, n); break;
        case PAD_FACA:     padrao_faca(g, o, n);     break;
        /* case PAD_X:     padrao_x(g, o, n);        break; */
        default:           break;
    }
}

/*
 * nova onda: linha aqui
 * novo ataque: nome em enum, função padrao, case no switch
 */

static Onda ondas[] = {
    // tipo        duracao  intervalo  escala
//    { PAD_CHUVA,    8.0f,    0.25f,     1.0f },   // chuva leve
//    { PAD_CHUVA,    8.0f,    0.40f,     2.0f },   // chuva pesada
//    { PAD_CIRCULO,  8.0f,    2.0f,      1.0f },   // circulo do jevil
//    { PAD_BARREIRA, 8.0f,    0.4f,      1.0f },   // barreiras do sans
//    { PAD_ESPINGARDA, 8.0f,  0.8f,      1.0f },   // espingarda
    { PAD_FACA,     8.0f,    0.6f,      1.0f },   // facas com aviso
};

// quantidade de ondas = tamanho total / tamanho de cada
#define NUM_ONDAS ((int)(sizeof(ondas) / sizeof(ondas[0])))


// alguma bala ainda ativa na tela?
static bool tem_balas(Game *g) {
    for (int i = 0; i < MAX_BALAS; i++)
        if (g->balas[i].ativo) return true;
    return false;
}

// chamado 1x por frame no main.c. retorna true quando acabarem as ondas.
// e aqui (e so aqui) que se decide QUANDO um padrao dispara.
bool att_ondas(Game *g, float dt) {
    if (g->onda >= NUM_ONDAS) return true;
    g->tempoOnda += dt;

    Onda atual = ondas[g->onda];

    // disparo n acontece no instante n * intervalo (0, intervalo, 2*intervalo...)
    // depois da duracao para de disparar
    if (g->tempoOnda <= atual.duracao &&
        g->tempoOnda >= g->disparo * atual.intervalo) {
        exec_padrao(g, atual, g->disparo);
        g->disparo++;
    }

    // acabou o tempo da onda E todas as balas sumiram: passa pra proxima
    if (g->tempoOnda > atual.duracao && !tem_balas(g)) {
        g->onda++;
        g->tempoOnda = 0;
        g->disparo = 0;
    }
    return g->onda >= NUM_ONDAS;
}
