#include "game.h"


static void dano_player(Game *g) {
    g->hp -= 10;
    g->iframes = 1.0f;
}

void spawn_bala(Game *g, Bala nova) {
    // sem tamanho = nada pra criar (ex: barreira com a fresta encostada no topo)
    if (nova.r.width <= 0 || nova.r.height <= 0) return;

    // escala nao preenchida (0) desenharia o sprite com tamanho zero
    if (nova.escala == 0) nova.escala = 1;
    nova.ativo = true;

    for (int i = 0; i < MAX_BALAS; i++) {
        if (!g->balas[i].ativo) {
            // copia a struct inteira: nao sobra nada da bala que morava aqui
            g->balas[i] = nova;
            return;
        }
    }
    // array cheio: a bala nao nasce
}

// sem sprite desenha quadrado
static void desenha_bala(Bala *b) {
    if (b->textura != NULL) {
        float px = b->r.x + (b->r.width  - b->textura->width  * b->escala) / 2.0f;
        float py = b->r.y + (b->r.height - b->textura->height * b->escala) / 2.0f;
        DrawTextureEx(*b->textura, (Vector2){ px, py }, 0.0f, b->escala, WHITE);
    } else {
        DrawRectangleRec(b->r, WHITE);
    }
}

void att_entidades(Game *g, float dt) {
    // mover balas

     for (int i = 0; i < MAX_BALAS; i++) {
        if (!g->balas[i].ativo) continue;
        Bala *b = &g->balas[i];

        // ainda esperando: fica parada (mas continua colidindo, abaixo)
        if (b->espera > 0) {
            b->espera -= dt;
        } else {
            b->vel.x += b->acel.x*dt;   // velocidade
            b->vel.y += b->acel.y*dt;
            b->r.x += b->vel.x*dt;      // posicao
            b->r.y += b->vel.y*dt;
        }

        // hitbox da alma: 8x8 centrado no jogador (jogador = centro do sprite).
        Rectangle hitbox = { g->jogador.x - HITBOX_ALMA / 2, g->jogador.y - HITBOX_ALMA / 2,
                             HITBOX_ALMA, HITBOX_ALMA };
        if (CheckCollisionRecs(hitbox, b->r)) {
            if (g->iframes <= 0) dano_player(g);
            // bala some ao dar dano; barreira continua (os iframes impedem
            // que ela tire vida todo frame enquanto atravessa a alma)
            if (!b->persiste) b->ativo = false;
        }

           // quando sair da tela, ativo = false;
    if (b->r.x < -20 || b->r.x > VIRTUAL_W + 20 || b->r.y < -20 || b->r.y > VIRTUAL_H + 20) b->ativo = false;
        }


 
    }
   


void desenha_entidades(Game *g) {
    float dt = GetFrameTime();
    // decrementa imunidade
    if (g->iframes > 0) g->iframes -= dt;
    
    // balas sem recorte: aparecem em qualquer lugar da tela
    for (int i = 0; i < MAX_BALAS; i++) {
        if (g->balas[i].ativo && !g->balas[i].recorta) {
            desenha_bala(&g->balas[i]);
        }
    }

    // balas com recorte: so aparecem dentro da caixa
    BeginScissorMode((int)(g->caixa.x + BORDA),
                     (int)(g->caixa.y + BORDA),
                     (int)(g->caixa.width  - 2 * BORDA),
                     (int)(g->caixa.height - 2 * BORDA));

    for (int i = 0; i < MAX_BALAS; i++) {
        if (g->balas[i].ativo && g->balas[i].recorta) {
            desenha_bala(&g->balas[i]);
        }
    }

    EndScissorMode();
}

