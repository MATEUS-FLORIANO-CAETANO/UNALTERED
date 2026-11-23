// entupi de comentario pra vc poder entender o que cada coisa faz

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
 *- golpe que cai do ceu com pouca vel pra baixo e muita vel lateral, quando chega na borda da caixa inverte, como se tivesse quicando (.quica)
 *- chuva que cai tudo de uma vez, mas marca com laser antes pra encontrar a "folga"
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
 * um padrao eh uma funcao  static void padrao_x(Game *g, Onda o, int n)
 * que o att_ondas chama a cada o.intervalo segundos. ela so cria balas:
 *
 *   g -> estado do jogo (caixa, jogador...)
 *   o -> a linha da tabela ondas[] (o.escala, o.intervalo, o.duracao)
 *   n -> numero do disparo nessa onda: 0 no primeiro, 1 no segundo...
 *        (n == 0 serve pra resetar variaveis 'static' no comeco da onda)
 *
 * pra criar uma bala, preenche so os campos que importam:
 *
 *   spawn_bala(g, (Bala){
 *       .r        = { x, y, largura, altura },   // ou ret_centrado(centro, w, h)
 *       .vel      = { 0, 120 },                  // px/s
 *       .acel     = { 0, 0 },                    // px/s²
 *       .espera   = 0.5f,                        // s parada antes de sair
 *       .persiste = true,                        // nao some ao acertar
 *       .recorta  = true,                        // so aparece dentro da caixa
 *       .textura  = &sprite, .escala = o.escala,
 * 
 *       .angulo   = atan2f(vel.y, vel.x) * RAD2DEG, // CLAUDE: nao soubemos fazer. 
 *                                                      gira o desenho (graus) pra onde vai
 *
 *       .segue    = 0.6f,                        // s acompanhando a alma antes da espera
 *       .vida     = 2.0f,                        // s ate sumir (0 = nunca)
 *       .persegue = 2.0f,                        // rad/s girando a vel pra alma
 *       .laser    = true,                        // aviso verde na direcao da ACEL (nao da vel)
 *       .quica    = true,                        // inverte vel.x nas paredes laterais da caixa
 *       .oval     = true,                        // desenha e colide como elipse
 *       .rebativel = true, .rebates = 2,         // bala de parry (ver padrao_parry)
 *   });
 *
 * dps coloca uma linha na tabela ondas[].
 */

// cada onda carrega a funcao do padrao, sua duracao (s), o intervalo entre disparos (s),
// a escala das balas e o atraso (s) antes do 1o disparo (opcional, 0 se omitido)
typedef struct Onda Onda;
typedef void (*Padrao)(Game *g, Onda o, int n);

struct Onda {
    Padrao padrao;
    float duracao;     // s disparando (conta depois do atraso)
    float intervalo;
    float escala;
    float atraso;      // s esperando antes do 1o disparo
    int nivel;         // 1 facil, 2 medio, 3 dificil. ao mudar de nivel a vida volta a 100
};

// valor conforme o nivel da onda. ex: por_nivel(o, 6, 8, 10) = 6 balas no facil...
// o medio e sempre o valor original do padrao
static float por_nivel(Onda o, float facil, float medio, float dificil) {
    if (o.nivel <= 1) return facil;
    if (o.nivel == 2) return medio;
    return dificil;
}


// ---------------------------------------------------------------------------------
//CLAUDE: AJUDANTES: contas de geometria que todo padrao precisa

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

// textura da gota de fogo, carregada uma vez so e usada por todas as chuvas
static Texture2D *gota(void) {
    static Texture2D t = { 0 };
    if (t.id == 0) t = LoadTexture("assets/chuva-fogo.png");
    return &t;
}


// ---------------------------------------------------------------------------------
// PADROES

// chuva: balas nascem acima da caixa em x aleatorio e caem reto pra baixo
static void padrao_chuva(Game *g, Onda o, int n) {
    
    (void)n;

    // tamanho aleatorio: +10% a +100%.
    float fator = GetRandomValue(110, 200) / 100.0f;
    float escala = o.escala * fator;

    // chuva-fogo.png e 16x16, a gota ocupa x 4..11, y 0..9. duas hitboxes:
    // corpo: oval 8x6 nas linhas 4..9 (centro y = 7). o centro do png e 8,
    //        entao o sprite e desenhado 1px abaixo do centro da hitbox
    // chama: retangulo 3x3 em x 5..7, linhas 1..3 (a linguinha da direita e a
    //        pontinha da linha 0 ficam de fora)
    float w = 8 * escala;
    float h = 6 * escala;
    const Vector2 DESLOC = { 0, 1.0f };
    Rectangle cauda = { 1 * escala, -3 * escala, 3 * escala, 3 * escala };  // relativo ao corpo

    // x aleatorio dentro da caixa, sem passar da parede direita
    Rectangle in = interior(g);
    float x = (float)GetRandomValue((int)in.x, (int)(in.x + in.width - w));

    spawn_bala(g, (Bala){
        .r       = { x, g->caixa.y - h, w, h },   // logo acima da caixa
        .extra   = cauda,
        .oval    = true,
        .vel     = { 0, por_nivel(o, 100, 120, 145) },
        .recorta = true,                          // so aparece qnd entra
        .textura = gota(),
        .escala  = escala,
        .sprite_desloc = DESLOC,
    });
}

// ---------------------------------------------------------------------------------

// chuva lateral: igual a chuva, mas nasce fora da parede esquerda ou direita em y
// aleatorio e anda reto pro lado. a gota e desenhada girada 90 graus, entao a
// hitbox vira 6x8 (em pe) e a chama vai pra tras da gota.
static void gota_lateral(Game *g, Onda o, bool da_esquerda) {
    float fator = GetRandomValue(110, 200) / 100.0f;
    float escala = o.escala * fator;

    // corpo: oval 8x6 deitado vira 6x8 em pe
    float w = 6 * escala;
    float h = 8 * escala;

    // girando o png, o deslocamento do sprite e a chama giram junto:
    // indo pra direita (-90 graus) a chama fica atras (esquerda) e um pouco abaixo;
    // indo pra esquerda (+90 graus) fica atras (direita) e um pouco acima
    Vector2 desloc;
    Rectangle cauda;
    float angulo;
    if (da_esquerda) {
        angulo = -90;
        desloc = (Vector2){ 1.0f, 0 };
        cauda  = (Rectangle){ -3 * escala, 4 * escala, 3 * escala, 3 * escala };
    } else {
        angulo = 90;
        desloc = (Vector2){ -1.0f, 0 };
        cauda  = (Rectangle){ 6 * escala, 1 * escala, 3 * escala, 3 * escala };
    }

    // y aleatorio dentro da caixa, sem passar do chao
    Rectangle in = interior(g);
    float y = (float)GetRandomValue((int)in.y, (int)(in.y + in.height - h));
    float x = da_esquerda ? g->caixa.x - w : g->caixa.x + g->caixa.width;

    spawn_bala(g, (Bala){
        .r       = { x, y, w, h },                // logo fora da parede
        .extra   = cauda,
        .oval    = true,
        .vel     = { (da_esquerda ? 1 : -1) * por_nivel(o, 100, 110, 120), 0 },
        .recorta = true,
        .textura = gota(),
        .escala  = escala,
        .angulo  = angulo,
        .sprite_desloc = desloc,
    });
}

// sorteia um lado no comeco da onda e a chuva toda vem dele
static void padrao_chuva_lado(Game *g, Onda o, int n) {
    static bool esq;
    if (n == 0) esq = GetRandomValue(0, 1);
    gota_lateral(g, o, esq);
}

// cada gota vem de um lado: esquerda, direita, esquerda...
static void padrao_chuva_alterna(Game *g, Onda o, int n) {
    gota_lateral(g, o, n % 2 == 0);
}

// ---------------------------------------------------------------------------------

// chuva com aviso: uma fileira inteira de gotas nasce acima da caixa, parada,
// mostrando o laser de cada uma. depois cai tudo junto. a folga e onde nao tem laser.
static void padrao_chuva_laser(Game *g, Onda o, int n) {
    (void)n;
    const float AVISO = por_nivel(o, 0.7f, 0.5f, 0.3f);          // s parada mostrando os lasers
    const float ACEL  = por_nivel(o, 800, 1000, 1800);             // px/s^2 pra baixo
    const float FOLGA = por_nivel(o, 2.5f, 2.0f, 1.6f) * TAM_ALMA; // largura minima do buraco

    float escala = o.escala;
    float w = 8 * escala;
    float h = 6 * escala;
    Rectangle cauda = { 1 * escala, -3 * escala, 3 * escala, 3 * escala };

    // quantas gotas cabem lado a lado; a sobra fica dividida nas duas paredes
    Rectangle in = interior(g);
    int colunas = (int)(in.width / w);
    float x0 = in.x + (in.width - colunas * w) / 2;

    // buraco: 'vazias' colunas seguidas sem gota, comecando numa coluna sorteada
    int vazias = (int)ceilf(FOLGA / w);
    int inicio = GetRandomValue(0, colunas - vazias);

    for (int i = 0; i < colunas; i++) {
        if (i >= inicio && i < inicio + vazias) continue;
        spawn_bala(g, (Bala){
            .r       = { x0 + i * w, g->caixa.y - h, w, h },
            .extra   = cauda,
            .oval    = true,
            .acel    = { 0, ACEL },               // laser aponta pra acel: pra baixo
            .espera  = AVISO,
            .laser   = true,
            .recorta = true,
            .textura = gota(),
            .escala  = escala,
            .sprite_desloc = { 0, 1.0f },
        });
    }
}

// ---------------------------------------------------------------------------------

// golpe quicando: cai do ceu devagar pra baixo, mas rapido pro lado,
// e inverte o lado ao bater nas paredes da caixa
static void padrao_quica(Game *g, Onda o, int n) {
    const float VX  = por_nivel(o, 75, 90, 110);   // px/s pro lado
    const float VY  = por_nivel(o, 35, 40, 50);    // px/s pra baixo
    const float TAM = 8 * o.escala;

    Rectangle in = interior(g);
    float x = (float)GetRandomValue((int)in.x, (int)(in.x + in.width - TAM));
    float dir = (n % 2 == 0) ? 1.0f : -1.0f;   // alterna quem comeca indo pra direita

    spawn_bala(g, (Bala){
        .r       = { x, g->caixa.y - TAM, TAM, TAM },
        .vel     = { dir * VX, VY },
        .quica   = true,
        .recorta = true,
        .escala  = o.escala,
    });
}

// ---------------------------------------------------------------------------------

// circulo (jevil): a cada disparo nasce um anel de n balas em volta do centro da
// caixa. a bala k espera k*ATRASO segundos e acelera pro centro
static void padrao_circulo(Game *g, Onda o, int n) {

     static Texture2D jevil = { 0 };
    if (jevil.id == 0) jevil = LoadTexture("assets/jevil.png");

    const int   N      = (int)por_nivel(o, 6, 8, 10);
    const float RAIO   = 110.0f;
    const float ACEL   = por_nivel(o, 240, 300, 340);       // px/s^2
    const float ATRASO = por_nivel(o, 0.13f, 0.10f, 0.08f); // segs. entre uma bala e a vizinha (volta = N*ATRASO)
    // recuo (telegrafa o ataque): a bala sai pra fora com velocidade do recuo e a acel faz voltar pro centro
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
            .textura = &jevil,
        });
    }
}

// ---------------------------------------------------------------------------------

// explosao: N balas empilhadas num ponto (parece uma bala so, e o aviso),
// depois saem todas pra fora em anel, acelerando.
// o ponto e sorteado dentro da caixa longe da alma: no centro fixo, a alma
// parada no meio levaria dano no instante em que as balas nascem.
static void padrao_explosao(Game *g, Onda o, int n) {
    const int   N     = (int)por_nivel(o, 10, 12, 14);
    const float VEL   = 30.0f;    // px/s logo que sai
    const float ACEL  = por_nivel(o, 180, 220, 270);   // px/s^2 pra fora
    const float AVISO = 0.7f;     // s empilhadas antes de explodir
    const float LONGE = 50.0f;    // distancia minima da alma
    const float TAM   = 6 * o.escala;

    // sorteia ate achar um ponto longe da alma (10 tentativas bastam numa caixa desse tamanho)
    Rectangle in = interior(g);
    Vector2 c = centro(in);
    for (int t = 0; t < 10; t++) {
        c = (Vector2){ (float)GetRandomValue((int)(in.x + TAM), (int)(in.x + in.width - TAM)),
                       (float)GetRandomValue((int)(in.y + TAM), (int)(in.y + in.height - TAM)) };
        if (Vector2Distance(c, g->jogador) >= LONGE) break;
    }

    // como no circulo: disparos impares giram meio passo, pra mudar onde ficam os buracos
    float giro = (n % 2) * (PI / N);

    for (int k = 0; k < N; k++) {
        Vector2 dir = no_circulo((Vector2){ 0, 0 }, 1, giro + k * 2 * PI / N);   // unitario
        spawn_bala(g, (Bala){
            .r       = ret_centrado(c, TAM, TAM),
            .vel     = Vector2Scale(dir, VEL),
            .acel    = Vector2Scale(dir, ACEL),
            .espera  = AVISO,
            .recorta = true,
            .escala  = o.escala,
        });
    }
}

// ---------------------------------------------------------------------------------

// duas balas varrendo a tela, com uma fresta no meio
// a fresta de cada par fica perto da anterior, pra sempre dar tempo de alcancar.
static void padrao_barreira(Game *g, Onda o, int n) {
    // static Texture2D barreira = { 0 };
    // if (barreira.id == 0) barreira = LoadTexture("assets/barreira.png");

    const float VEL    = por_nivel(o, 115, 140, 170);   // px/s pro lado
    const int   DIR    = 1;               // +1 direita, -1 esquerda
    const float LARG   = 12 * o.escala;
    const float FRESTA = por_nivel(o, 2.5f, 2.0f, 1.6f) * TAM_ALMA; // altura da fresta

    Rectangle in = interior(g);
    float topo  = in.y;
    float fundo = in.y + in.height;

    // topo da fresta do par anterior (static: guarda entre disparos)
    static float frestaY;
    if (n == 0) frestaY = centro(in).y - FRESTA / 2;   // 1 par: fresta no meio

    // a fresta anda no maximo o que a alma anda em um intervalo (com 30% de folga)
    int passo = (int)(VEL_JOGADOR * o.intervalo * 0.7f);

    // puxa a fresta de volta pro meio: quanto mais longe, mais forte o puxao
    // (0 = passeio aleatorio puro, 1 = volta pro meio de uma vez)
    const float PUXAO = 0.35f;
    float meio  = centro(in).y - FRESTA / 2;
    float passoY = GetRandomValue(-passo, passo) + (meio - frestaY) * PUXAO;
    passoY = Clamp(passoY, -passo, passo);   // nunca pula mais do que a alma alcanca
    frestaY = Clamp(frestaY + passoY, topo, fundo - FRESTA);

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

// espingarda: a cada disparo, um leque de k balas mirado na alma, saindo de fora da
// caixa. alterna esquerda/direita; a altura e aleatoria 
// em uma area ao redor do centro da caixa cobrindo 1.5x tamanho del
static void padrao_espingarda(Game *g, Onda o, int n) {
    const int   K      = (int)por_nivel(o, 4, 6, 8);    // balas por disparo
    const float ABERTO = 60 * DEG2RAD;      // abertura do leque
    const float VEL    = por_nivel(o, 115, 140, 170);   // px/s
    const float DIST   = 50.0f;             // distancia da origem ate parde da caixa
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

// faca nasce num ponto aleatorio em volta da caixa apontada pra alma,
// mostra a linha de aviso verde, recua um pouco (como no circulo) e dispara muito rapido.
// a hitbox e uma fileira de M quadradinhos com o mesmo movimento, formando a lamina
// (Rectangle nao gira).
// uma faca: ponta = onde nasce a ponta da lamina, dir = unitario pra onde ela vai.
// usada por padrao_faca e padrao_facas_cruz
static void spawn_faca(Game *g, Onda o, Vector2 ponta, Vector2 dir, float segue, float espera) {
    const float ACEL  = 55000.0f;  // px/s^2
    const float RECUO = 250.0f;   // px/s
    const int   M     = 10;       // quadradinhos na lamina
    const float PASSO = 4 * o.escala;   // distancia entre eles (comprimento ~ M*PASSO)
    const float TAM   = 5 * o.escala;   // lado de cada quadradinho

    // CLAUDE: quadrados escondidos. com sprite, a faca inteira e desenhada 1x so no
    // quadradinho do meio da lamina, girada pra mirar na alma; os outros ficam
    // invisiveis mas continuam cortando. sem sprite nada muda (invisivel e ignorado).
    // descomentar quando assets/faca.png existir (desenhar apontando pra direita, ~41x5):
    // static Texture2D faca = { 0 };
    // if (faca.id == 0) faca = LoadTexture("assets/faca.png");

    for (int k = 0; k < M; k++) {
        // k = 0 e a ponta; o resto vai ficando pra tras ao longo da lamina
        Vector2 pos = Vector2Subtract(ponta, Vector2Scale(dir, k * PASSO));
        spawn_bala(g, (Bala){
            // .textura   = &faca,
            .invisivel = (k != M / 2),
            .angulo    = atan2f(dir.y, dir.x) * RAD2DEG,
            .r        = ret_centrado(pos, TAM, TAM),
            .vel      = Vector2Scale(dir, -RECUO),   // recua antes de ir
            .acel     = Vector2Scale(dir, ACEL),
            .segue    = segue,
            .espera   = espera,
            .persiste = true,                        // a lamina nao se desmancha
            .laser    = (k == 0),                    // so a ponta desenha a linha
            .escala   = o.escala,
        });
    }
}

static void padrao_faca(Game *g, Onda o, int n) {
    (void)n;

    const float RAIO  = 120.0f;   // distancia do centro da caixa onde a faca nasce
    const float SEGUE = por_nivel(o, 0.0f, 0.6f, 0.45f); // s acompanhando a alma (mesma distancia, mesma mira)
    const float AVISO = 0.9f;     // s congelada antes de disparar

    Vector2 c = centro(g->caixa);
    float ang = GetRandomValue(0, 359) * DEG2RAD;
    Vector2 ponta = no_circulo(c, RAIO, ang);
    Vector2 dir = rumo(ponta, g->jogador, 1);   // unitario, mirando na alma

    spawn_faca(g, o, ponta, dir, SEGUE, AVISO);
}

// ---------------------------------------------------------------------------------

// facas em cruz: uma horizontal e uma vertical, na mesma distancia da alma.
// acompanham juntas, congelam juntas e fecham juntas no mesmo ponto.
// DIST 100: de baixo, com a alma no chao, a lamina inteira ainda cabe na tela
static void padrao_facas_cruz(Game *g, Onda o, int n) {
    (void)n;
    const float DIST  = 100.0f;   // distancia da ponta ate a alma
    const float SEGUE = por_nivel(o, 0, 0.8f, GetRandomValue(60, 100) / 100.0f);     // s acompanhando
    const float AVISO = por_nivel(o, 0.9f, 0.15f, 0.03f);  // s congeladas antes de fechar

    float lx = GetRandomValue(0, 1) ? 1.0f : -1.0f;   // +1 = horizontal vem da direita
    float ly = GetRandomValue(0, 1) ? 1.0f : -1.0f;   // +1 = vertical vem de baixo
    Vector2 p = g->jogador;

    spawn_faca(g, o, (Vector2){ p.x + lx * DIST, p.y }, (Vector2){ -lx, 0 }, SEGUE, AVISO);
    spawn_faca(g, o, (Vector2){ p.x, p.y + ly * DIST }, (Vector2){ 0, -ly }, SEGUE, AVISO);
}

// ---------------------------------------------------------------------------------

// gaiola: circulo grande com centro num ponto aleatorio da arena fecha ate o raio
// ficar ~a largura da alma e some. o lugar seguro e o centro (ou uma fresta).
static void padrao_gaiola(Game *g, Onda o, int n) {
    (void)n;
    const int   N      = 40;                // balas no circulo
    const float FINAL  = TAM_ALMA;          // raio em que some
    const float VEL    = por_nivel(o, 75, 100, 150);   // px/s fechando
    const float TAM    = 6 * o.escala;
    const float ESPERA = por_nivel(o, 0.5f, 0.4f, 0.2f);  // segs. parada antes de fechar

    // centro sorteado dentro da arena, com folga pra alma caber em volta dele
    Rectangle in = interior(g);
    Vector2 c = { (float)GetRandomValue((int)(in.x + TAM_ALMA), (int)(in.x + in.width  - TAM_ALMA)),
                  (float)GetRandomValue((int)(in.y + TAM_ALMA), (int)(in.y + in.height - TAM_ALMA)) };

    // raio: distancia ate o canto da caixa (com borda) mais longe do centro,
    // + meia diagonal da bala, assim nenhuma bala nasce encostando na caixa
    Rectangle cx = g->caixa;
    float dx = fmaxf(c.x - cx.x, cx.x + cx.width  - c.x);
    float dy = fmaxf(c.y - cx.y, cx.y + cx.height - c.y);
    float raio = sqrtf(dx * dx + dy * dy) + TAM * 0.75f;

    // vida: espera + tempo pra andar de raio ate FINAL (vida conta durante a espera)
    float vida = ESPERA + ((raio - FINAL) / VEL) - 0.1f;

    for (int k = 0; k < N; k++) {
        Vector2 pos = no_circulo(c, raio, k * 2 * PI / N);
        spawn_bala(g, (Bala){
            .r        = ret_centrado(pos, TAM, TAM),
            .vel      = rumo(pos, c, VEL),
            .espera   = ESPERA,
            .vida     = vida,
            .persiste = true,
            .escala   = o.escala,
        });
    }
}

// ---------------------------------------------------------------------------------

// enxame: muitas balas pequenas vindo de todo lado, cada uma curvando de leve
// pra alma. vida limitada pra nao ficarem orbitando pra sempre
static void padrao_enxame(Game *g, Onda o, int n) {
    (void)n;
    const float RAIO     = 130.0f;   // distancia do centro da caixa onde nascem
    const float VEL      = por_nivel(o, 70, 80, 95);        // px/s
    const float PERSEGUE = por_nivel(o, 0.5f, 0.7f, 0.9f);  // rad/s: curva fraca
    const float VIDA     = 4.0f;     // s
    const float TAM      = 4 * o.escala;

    float ang = GetRandomValue(0, 359) * DEG2RAD;
    Vector2 pos = no_circulo(centro(g->caixa), RAIO, ang);

    spawn_bala(g, (Bala){
        .r        = ret_centrado(pos, TAM, TAM),
        .vel      = rumo(pos, g->jogador, VEL),
        .persegue = PERSEGUE,
        .vida     = VIDA,
        .escala   = o.escala,
    });
}

// ---------------------------------------------------------------------------------

// perseguidora: bala que corre atras da alma, quase tao rapida quanto ela e
// virando forte. parado, leva dano. a cada disparo entra mais uma, no canto
// mais longe da alma, e todas somem juntas no fim da onda
static void padrao_perseguidora(Game *g, Onda o, int n) {
    const float VEL   = por_nivel(o, 95, 105, 112);   // px/s (alma anda 120 reto, ~170 na diagonal)
    const float GIRO  = por_nivel(o, 4, 5, 6);        // rad/s: curva forte
    const float AVISO = 0.8f;     // s parada antes de comecar a correr
    const float TAM   = 8 * o.escala;

    // vida = o que falta da onda. o ultimo disparo cai bem no fim e daria
    // vida ~0, que significa "nunca some", entao nem nasce
    float vida = o.duracao - n * o.intervalo;
    if (vida < 1.0f) return;

    Rectangle in = interior(g);
    Vector2 c = centro(in);
    Vector2 pos = { g->jogador.x < c.x ? in.x + in.width - TAM / 2 : in.x + TAM / 2,
                    g->jogador.y < c.y ? in.y + in.height - TAM / 2 : in.y + TAM / 2 };

    spawn_bala(g, (Bala){
        .r        = ret_centrado(pos, TAM, TAM),
        .vel      = rumo(pos, g->jogador, VEL),   // persegue precisa de vel != 0
        .persegue = GIRO,
        .espera   = AVISO,
        .vida     = vida,
        .persiste = true,
        .recorta  = true,
        .escala   = o.escala,
    });
}

// ---------------------------------------------------------------------------------

// parry: uma bala so por nivel, que vai e volta entre a alma e o inimigo.
// bloqueios pra vencer: nivel 1 = 1, nivel 2 = 3, nivel 3 = 5 (o inimigo devolve
// bloqueios - 1 vezes). cada devolucao acelera a bala (MULT_REBATE); como a bala
// nasce de novo em cada nivel, a velocidade volta ao normal.
// errou o bloqueio: toma dano e a bala volta a cair (parry.c), ate completar
static void padrao_parry(Game *g, Onda o, int n) {
    if (n > 0) return;
    g->parry = true;
    g->rally_ganho = false;
    int bloqueios = 2 * o.nivel - 1;

    Rectangle in = interior(g);
    float h = 15;

    spawn_bala(g, (Bala){
        .r         = { in.x, LINHA_INIMIGO + 5, in.width, h },   // logo abaixo dos olhos
        .vel       = { 0, 220 * o.escala },
        .espera    = 0.5f,
        .oval      = true,
        .recorta   = false,
        .persiste  = true,
        .rebativel = true,
        .rebates   = bloqueios - 1,
    });
    g->contador_inimigo++;
}

// ----------------------------------------------------------------------------------

// nova onda ou novo ataque: so uma linha aqui
static Onda ondas[] = {
    // padrao                duracao  intervalo  escala  atraso  nivel
    // o resto da dificuldade (velocidade, quantidade...) fica em cada padrao, via por_nivel
    // fora da tabela por enquanto: padrao_chuva_lado, padrao_explosao

    //NIVEL 1: facil
    
    // { padrao_chuva,          12.0f,   0.30f,     1.0f,   0.8f,   1 },   // chuva
    // { padrao_circulo,        14.0f,   2.4f,      1.5f,   0.5f,   1 },   // circulo do jevil
    // { padrao_gaiola,         11.0f,   4.2f,      1.0f,   0.0f,   1 },   // gaiola fechando num ponto
    // { padrao_barreira,       10.0f,   0.6f,      1.0f,   0.5f,   1 },   // barreiras do sans
    // { padrao_chuva_laser,    10.0f,   2.2f,      1.0f,   0.0f,   1 },   // chuva com laser e folga
    // { padrao_chuva_alterna,  10.0f,   0.26f,     1.0f,   0.0f,   1 },   // chuva lateral alternando
    // { padrao_espingarda,      8.0f,   1.0f,      1.0f,   0.5f,   1 },   // espingarda
    // { padrao_quica,          10.0f,   0.65f,     1.0f,   0.0f,   1 },   // golpes quicando
    // { padrao_enxame,         10.0f,   0.45f,     1.0f,   0.0f,   1 },   // enxame
    // { padrao_perseguidora,    9.0f,   4.6f,      1.0f,   0.0f,   1 },   // perseguidoras (2)
    // { padrao_faca,           12.0f,   0.8f,      1.0f,   0.5f,   1 },   // facas com aviso
    // { padrao_facas_cruz,     10.0f,   1.8f,      1.0f,   0.0f,   1 },   // facas em cruz
    { padrao_parry,           0.1f,   99.0f,     1.0f,   0.0f,   1 },   // pong, 1 bloqueio

    //NIVEL 2: medio (valores originais)
    // { padrao_chuva,          15.0f,   0.22f,     1.0f,   0.5f,   2 },
    // { padrao_circulo,        16.0f,   2.0f,      1.3f,   0.5f,   2 },
    // { padrao_gaiola,         11.0f,   3.6f,      1.0f,   0.0f,   2 },
    // { padrao_barreira,       12.0f,   0.5f,      1.0f,   0.5f,   2 },
    // { padrao_chuva_laser,    10.0f,   1.8f,      1.0f,   0.0f,   2 },
    // { padrao_chuva_alterna,  10.0f,   0.2f,      1.0f,   0.0f,   2 },
    // { padrao_espingarda,      8.0f,   0.8f,      1.0f,   0.5f,   2 },
    // { padrao_quica,          10.0f,   0.5f,      1.0f,   0.0f,   2 },
    // { padrao_enxame,         10.0f,   0.36f,     1.0f,   0.0f,   2 },
    // { padrao_perseguidora,    9.0f,   3.0f,      1.0f,   0.0f,   2 },   // perseguidoras (3)
    // { padrao_faca,           15.0f,   0.6f,      1.0f,   0.5f,   2 },
    // { padrao_facas_cruz,     10.0f,   1.6f,      1.0f,   0.0f,   2 },
    // { padrao_parry,           0.1f,   99.0f,     1.0f,   0.0f,   2 },   // pong, 3 bloqueios

    // //NIVEL 3: dificil
    // { padrao_chuva,          15.0f,   0.18f,     1.25f,  0.5f,   3 },
    // { padrao_circulo,        16.0f,   1.6f,      1.5f,  0.5f,   3 },
    // { padrao_gaiola,         11.0f,   3.0f,      1.0f,   0.0f,   3 },
    // { padrao_barreira,       12.0f,   0.4f,      1.0f,   0.5f,   3 },
    // { padrao_chuva_laser,    10.0f,   1.5f,      1.0f,   0.0f,   3 },
    // { padrao_chuva_alterna,  10.0f,   0.16f,     1.0f,   0.0f,   3 },
    // { padrao_espingarda,     10.0f,   1.0f,      1.0f,   0.5f,   3 },
    // { padrao_quica,          10.0f,   0.4f,      1.0f,   0.0f,   3 },
    // { padrao_enxame,         10.0f,   0.28f,     1.0f,   0.0f,   3 },
    // { padrao_perseguidora,    9.0f,   2.4f,      1.0f,   0.0f,   3 },   // perseguidoras (4)
    // { padrao_faca,           15.0f,   0.5f,      1.0f,   0.5f,   3 },
    // { padrao_facas_cruz,     10.0f,   1.3f,      1.0f,   0.0f,   3 },
    { padrao_parry,           0.1f,   99.0f,     1.0f,   0.0f,   3 },   // pong, 5 bloqueios
};

// quantidade de ondas = tamanho total / tamanho de cada
#define NUM_ONDAS ((int)(sizeof(ondas) / sizeof(ondas[0])))


// alguma bala ainda ativa na tela?
static bool tem_balas(Game *g) {
    for (int i = 0; i < MAX_BALAS; i++)
        if (g->balas[i].ativo) return true;
    return false;
}

// nivel da onda de indice 'onda' (g->onda). 0 = acabaram as ondas
int nivel_da_onda(int onda) {
    if (onda < 0 || onda >= NUM_ONDAS) return 0;
    return ondas[onda].nivel;
}

// chamado 1x por frame no main.c. retorna true quando acabarem as ondas.
// aqui que decide quando um padrao dispara.
bool att_ondas(Game *g, float dt) {
    if (g->onda >= NUM_ONDAS) return true;
    g->tempoOnda += dt;

    Onda atual = ondas[g->onda];

    // tempo contado a partir do fim do atraso
    float t = g->tempoOnda - atual.atraso;

    // disparo n acontece no instante n * intervalo
    // depois da duracao para de disparar
    if (t >= 0 && t <= atual.duracao &&
        t >= g->disparo * atual.intervalo) {
        atual.padrao(g, atual, g->disparo);
        g->disparo++;
    }

    if (t > atual.duracao && !tem_balas(g)) {
        // errou o pong: repete a mesma onda (os rebates vem do nivel).
        // na pratica a bala ja volta a cair sozinha quando erra (parry.c), isso e garantia
        if (!(atual.padrao == padrao_parry && !g->rally_ganho)) g->onda++;
        g->tempoOnda = -PAUSA_ONDAS;   // negativo: espera antes do 1o disparo
        g->disparo = 0;
        g->parry = 0;

        // comecou um nivel novo: vida cheia
        if (g->onda < NUM_ONDAS && ondas[g->onda].nivel != atual.nivel) g->hp = 100;
    }
    return g->onda >= NUM_ONDAS;
}
