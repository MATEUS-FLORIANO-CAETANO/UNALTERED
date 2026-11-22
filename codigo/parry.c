// parry: pesonagem rebate a bala rebativel
// o spawn fica em ataques.c (padrao_parry)

#include "game.h"

// area de parry: logo acima da alma
static Rectangle zona_parry(Vector2 jogador) {
    const float T = 22;
    return (Rectangle){ jogador.x - T / 2, jogador.y - TAM_ALMA / 2.0f - T, T, T * 2 };
}

void att_parry(Game *g, bool apertou, float dt) {
    // apertou: hitbox fica ativa por JANELA_PARRY segs
    if (g->parry && apertou) g->janela_parry = JANELA_PARRY;

    if (g->anim_inimigo > 0) g->anim_inimigo -= dt;

    Rectangle zona = zona_parry(g->jogador);
    if (g->janela_parry > 0) g->janela_parry -= dt;

    for (int i = 0; i < MAX_BALAS; i++) {
        Bala *b = &g->balas[i];
        if (!b->ativo || !b->rebativel) continue;

        // alma rebate (descendo)
        if (g->janela_parry > 0 && b->vel.y > 0 && CheckCollisionRecs(zona, b->r)) {
            b->vel.y = -b->vel.y;
            g->janela_parry = 0;   // um rebate por aperto
            g->hp += 5;
            efeito_parry();
        }

        // errou o bloqueio (o dano ja foi em entidades.c): passou do fundo da caixa,
        // volta pra baixo dos olhos e cai de novo, na mesma velocidade.
        // o ataque so acaba quando completar os bloqueios do nivel
        if (b->vel.y > 0 && b->r.y > g->caixa.y + g->caixa.height) {
            b->r.y = LINHA_INIMIGO + 5;
            b->espera = 0.5f;
        }

        // chegou no inimigo (subindo): devolve ou some
        if (b->vel.y < 0 && b->r.y <= LINHA_INIMIGO) {
            if (b->rebates > 0) {
                b->rebates--;
                b->vel.y = -b->vel.y * MULT_REBATE;
                efeito_rebate();
                olhos_rebate();
            } else {
                b->ativo = false;
                g->anim_inimigo = 1.0f;
                g->rally_ganho = true;
            }
        }
    }
}

void desenha_parry(Game *g) {
    if (!g->parry) return;
    Rectangle zona = zona_parry(g->jogador);
    if (g->janela_parry > 0) DrawRectangleRec(zona, Fade(PINK, 0.5f));   // ativa
    DrawRectangleLinesEx(zona, 1, PINK);
}
