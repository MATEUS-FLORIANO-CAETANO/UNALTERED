#include "game.h"


static void dano_player(Game *g) {
    g->hp -= 10;
    g->iframes = 1.0f;
}

Bala *spawn_bala(Game *g, Vector2 pos, Vector2 vel, Vector2 tamanho, bool recorta, Texture2D *textura, float escala) {
    int i;
    for (i = 0; i < MAX_BALAS; i++) {
        if (!g->balas[i].ativo) break;
    }
    if (i >= MAX_BALAS) return NULL;

    Bala *b = &g->balas[i];

    // hitbox independente do sprite: o ataque passa o tamanho, pois a imagem pode
    // ter bordas transparentes (ex: gota ~10x13 dentro de um png 32x32)
    b->r.width  = tamanho.x;
    b->r.height = tamanho.y;
    b->r.x = pos.x;
    b->r.y = pos.y;

    b->vel.x = vel.x;
    b->vel.y = vel.y;

    // o espaco do vetor e reaproveitado, temq zerar pra prox bala
    b->acel = (Vector2){ 0, 0 };
    b->espera = 0;
    b->persiste = false;

    b->ativo = true;
    // o ataque decide:
    b->recorta = recorta;   // e a bala e recortada pela caixa
    b->textura = textura;   // a aparencia da bala
    b->escala = escala;     // o tamanho do desenho
    return b;
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

