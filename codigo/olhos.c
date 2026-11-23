// olhos do inimigo: desenho e reacoes ao parry.
    
#include "game.h"
#include <math.h>   // atan2f, cosf, sinf

#define RAIO_ESCLERA 80.0f
#define RAIO_IRIS    24.0f

#define PISCADA_DUR  0.15f   // segs. pra fechar e abrir no rebate
#define RECUO_DUR    0.30f   // segs. pra voltar ao lugar
#define RECUO_PX     6.0f    // quanto sobe no impacto

static float piscada;   // segs. restantes
static float recuo;     // segs. restantes

// parry.c chama quando o inimigo devolve a bala
void olhos_rebate(void) {
    piscada = PISCADA_DUR;
    recuo   = RECUO_DUR;
}

void olhos_att(float dt) {
    if (piscada > 0) piscada -= dt;
    if (recuo   > 0) recuo   -= dt;
}

// bala de parry ativa (ou NULL)
static Bala *bala_parry(Game *g) {
    for (int i = 0; i < MAX_BALAS; i++)
        if (g->balas[i].ativo && g->balas[i].rebativel) return &g->balas[i];
    return NULL;
}

// fecha = 0 aberto, 1 fechado. palpebra desce de cima, da cor do fundo
// adaptado de raylib/examples/shapes/shapes_following_eyes.c
// esc = escala do olho (1 = tamanho cheio)
static void desenha_olho(Vector2 centro, Vector2 alvo, Color corIris, float fecha, float esc) {
    float rEsc = RAIO_ESCLERA * esc;
    float rIri = RAIO_IRIS * esc;
    Vector2 iris = alvo;
    // se o alvo esta fora do alcance, prende a iris na borda da esclera na direcao do alvo
    if (!CheckCollisionPointCircle(iris, centro, rEsc - rIri)) {
        float angulo = atan2f(alvo.y - centro.y, alvo.x - centro.x);
        iris.x = centro.x + (rEsc - rIri) * cosf(angulo);
        iris.y = centro.y + (rEsc - rIri) * sinf(angulo);
    }

    DrawCircleV(centro, rEsc, WHITE);
    DrawCircleV(iris, rIri, corIris);
    DrawCircleV(iris, 5 * esc, BLACK);

    if (fecha > 0)
        DrawRectangle(centro.x - rEsc, centro.y - rEsc,
                      rEsc * 2, rEsc * 2 * fecha, BLACK);

    /* versao com textura (quando assets/esclera.png e assets/iris.png existirem):
       1. raios vem do tamanho das imagens (texEsclera.width / 2, texIris.width / 2)
       2. troca os tres DrawCircleV por (DrawTexture desenha do canto, entao centraliza):
            DrawTexture(texEsclera, centro.x - texEsclera.width / 2.0f, centro.y - texEsclera.height / 2.0f, WHITE);
            DrawTexture(texIris,    iris.x   - texIris.width    / 2.0f, iris.y   - texIris.height    / 2.0f, corIris);
    */
}

void desenha_olhos(Game *g) {
    // levou a bala: pisca 3x em 1s (6 metades)
    if (g->anim_inimigo > 0 && (int)(g->anim_inimigo * 6) % 2 == 1) return;

    Color corIris = DARKGREEN;
    float fecha   = 0;
    // tamanho por wave: 60% nas waves 1-2, 80% na 3. base fica em LINHA_INIMIGO (onde a bala do parry bate)
    float esc     = g->nivel_aviso >= 3 ? 0.8f : 0.6f;
    float y       = LINHA_INIMIGO - RAIO_ESCLERA * esc;
    Bala *b = bala_parry(g);

    // REACAO 4: ainda vai devolver = iris colorida. ultima = olho meio fechado (cansado)
    if (b) {
        if (b->rebates > 0) corIris = ORANGE;
        else fecha = 0.35f;
    }

    // REACAO 2: pisca ao devolver (fecha e abre: 0 -> 1 -> 0)
    if (piscada > 0) {
        float t = 1 - piscada / PISCADA_DUR;
        fecha = fmaxf(fecha, 1 - fabsf(2 * t - 1));
    }

    // REACAO 3: recua pra cima no impacto e volta
    if (recuo > 0) y -= RECUO_PX * (recuo / RECUO_DUR);

    desenha_olho((Vector2){ VIRTUAL_W / 2.0f, y }, g->jogador, corIris, fecha, esc);

    // desenha_olho((Vector2){ VIRTUAL_W / 2.0f - 50, y }, g->jogador, corIris, fecha);
    // desenha_olho((Vector2){ VIRTUAL_W / 2.0f + 50, y }, g->jogador, corIris, fecha);
}
