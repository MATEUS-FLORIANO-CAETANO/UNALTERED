#include "game.h"
#include "raymath.h"   // Vector2Normalize, Vector2DotProduct...

static void dano_player(Game *g) {
    PlaySound(dano);
    g->hp -= 10;
    g->iframes = 1.0f;
}

void spawn_bala(Game *g, Bala nova) {
    // tam 0 nao faz nada
    if (nova.r.width <= 0 || nova.r.height <= 0) return;

    if (nova.escala == 0) nova.escala = 1;
    nova.ativo = true;
    // guarda a distancia ate a alma 
    nova.desloc = (Vector2){ nova.r.x - g->jogador.x, nova.r.y - g->jogador.y };

    for (int i = 0; i < MAX_BALAS; i++) {
        if (!g->balas[i].ativo) {
            // reescreve a bala inteira
            g->balas[i] = nova;
            return;
        }
    }
    // array cheio bala nao nasce
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

        // vida acabou, some (0 = nao expira)
        if (b->vida > 0) {
            b->vida -= dt;
            if (b->vida <= 0) { b->ativo = false; continue; }
        }

        // seguindo: sempre na mesma distancia da alma
        if (b->segue > 0) {
            b->segue -= dt;
            b->r.x = g->jogador.x + b->desloc.x;
            b->r.y = g->jogador.y + b->desloc.y;
        // esperando: nao faz nada
        } else if (b->espera > 0) {
            b->espera -= dt;
        } else {
            // perseguidora: gira a vel no maximo persegue*dt rad em direcao a alma
            if (b->persegue > 0) {
                Vector2 meio = { b->r.x + b->r.width / 2, b->r.y + b->r.height / 2 };
                Vector2 alvo = Vector2Subtract(g->jogador, meio);
                float dif = atan2f(alvo.y, alvo.x) - atan2f(b->vel.y, b->vel.x);
                dif = atan2f(sinf(dif), cosf(dif));   // menor caminho: [-PI, PI]
                float max = b->persegue * dt;
                b->vel = Vector2Rotate(b->vel, Clamp(dif, -max, max));
            }

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
            if (!b->persiste) b->ativo = false;
        }

        // quando sair da tela, ativo = false
        const float MARGEM = 64.0f;
        if (b->r.x < -MARGEM || b->r.x > VIRTUAL_W + MARGEM ||
            b->r.y < -MARGEM || b->r.y > VIRTUAL_H + MARGEM) b->ativo = false;
        }


 
    }
   


void desenha_entidades(Game *g) {
    float dt = GetFrameTime();
    // decrementa imunidade dps de tomar dano
    if (g->iframes > 0) g->iframes -= dt;

    // linhas de aviso saem do centro da bala na direcao da acel e
    // atravessam a tela. so enquanto espera ou recua (vel contra a acel).
    for (int i = 0; i < MAX_BALAS; i++) {
        Bala *b = &g->balas[i];
        if (!b->ativo || !b->laser) continue;
        if (b->espera <= 0 && Vector2DotProduct(b->vel, b->acel) >= 0) continue;

        Vector2 ini = { b->r.x + b->r.width / 2, b->r.y + b->r.height / 2 };
        Vector2 fim = Vector2Add(ini, Vector2Scale(Vector2Normalize(b->acel), 1000));
        DrawLineEx(ini, fim, 1.5f, PURPLE);
    }

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

