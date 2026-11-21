#include "game.h"
#include "raymath.h"   // funcoes em vector2

static void dano_player(Game *g) {
    PlaySound(dano);
    g->hp -= 10;
    g->iframes = 1.0f;
    g->cont_dano += 10;
    efeito_dano();
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

// elipse com centro em float. DrawEllipse/DrawEllipseLines do raylib recebem o centro
// em int e truncam, deslocando o desenho ate 1px pra esquerda/cima da hitbox real
#define SEGS_ELIPSE 36
static Vector2 ponto_elipse(Vector2 c, float rx, float ry, int k) {
    float a = k * 2 * PI / SEGS_ELIPSE;
    return (Vector2){ c.x + rx * cosf(a), c.y + ry * sinf(a) };
}

static void elipse(Rectangle r, Color cor) {
    Vector2 c = { r.x + r.width / 2, r.y + r.height / 2 };
    for (int k = 0; k < SEGS_ELIPSE; k++)   // ordem anti-horaria na tela, como o raylib quer
        DrawTriangle(c, ponto_elipse(c, r.width / 2, r.height / 2, k + 1),
                        ponto_elipse(c, r.width / 2, r.height / 2, k), cor);
}

static void elipse_contorno(Rectangle r, Color cor) {
    Vector2 c = { r.x + r.width / 2, r.y + r.height / 2 };
    for (int k = 0; k < SEGS_ELIPSE; k++)
        DrawLineV(ponto_elipse(c, r.width / 2, r.height / 2, k),
                  ponto_elipse(c, r.width / 2, r.height / 2, k + 1), cor);
}

// sem sprite desenha quadrado
static void desenha_bala(Bala *b) {
    Color cor = b->rebativel ? ORANGE : WHITE;
    if (b->textura != NULL) {
        if (b->invisivel) return;
        // CLAUDE: rotacao de texturas
        // dest posicionado no centro da hitbox (+ sprite_desloc) + origem no centro da
        // textura: fica centralizada e gira em volta do proprio meio (angulo 0 = sem giro)
        Texture2D t = *b->textura;
        float w = t.width * b->escala, h = t.height * b->escala;
        Rectangle dest = { b->r.x + b->r.width  / 2 + b->sprite_desloc.x * b->escala,
                           b->r.y + b->r.height / 2 + b->sprite_desloc.y * b->escala, w, h };
        DrawTexturePro(t, (Rectangle){ 0, 0, (float)t.width, (float)t.height },
                       dest, (Vector2){ w / 2, h / 2 }, b->angulo, cor);
    } else if (b->oval) {
        elipse(b->r, cor);
    } else {
        DrawRectangleRec(b->r, cor);
    }
}

// oval: ponto da hitbox mais perto do centro da elipse, dentro dela?
// extra: 2a hitbox retangular que anda junto com r
static bool acerta(Rectangle hitbox, Bala *b) {
    if (b->extra.width > 0) {
        Rectangle e = { b->r.x + b->extra.x, b->r.y + b->extra.y,
                        b->extra.width, b->extra.height };
        if (CheckCollisionRecs(hitbox, e)) return true;
    }
    if (!b->oval) return CheckCollisionRecs(hitbox, b->r);

    float rx = b->r.width / 2, ry = b->r.height / 2;
    Vector2 c = { b->r.x + rx, b->r.y + ry };
    float px = Clamp(c.x, hitbox.x, hitbox.x + hitbox.width);
    float py = Clamp(c.y, hitbox.y, hitbox.y + hitbox.height);
    float dx = (px - c.x) / rx, dy = (py - c.y) / ry;
    return dx * dx + dy * dy <= 1;
}

void att_entidades(Game *g, float dt) {
    // decrementa imunidade dps de tomar dano
    if (g->iframes > 0) g->iframes -= dt;

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

            // quica: so inverte se estiver indo pra fora
            if (b->quica) {
                float esq = g->caixa.x + BORDA;
                float dir = g->caixa.x + g->caixa.width - BORDA;
                if (b->r.x < esq && b->vel.x < 0)                 b->vel.x = -b->vel.x;
                if (b->r.x + b->r.width > dir && b->vel.x > 0)    b->vel.x = -b->vel.x;
            }
        }

        // hitbox da alma: 8x8 centrado no jogador (jogador = centro do sprite).
        Rectangle hitbox = { g->jogador.x - HITBOX_ALMA / 2, g->jogador.y - HITBOX_ALMA / 2,
                             HITBOX_ALMA, HITBOX_ALMA };
        if (acerta(hitbox, b)) {
            if (g->iframes <= 0) dano_player(g);
            if (!b->persiste) b->ativo = false;
        }

        // quando sair da tela, ativo = false
        // bala com vida ja some sozinha: pode nascer fora da tela (ex: gaiola grande)
        const float MARGEM = 64.0f;
        if (b->vida <= 0 && (b->r.x < -MARGEM || b->r.x > VIRTUAL_W + MARGEM ||
            b->r.y < -MARGEM || b->r.y > VIRTUAL_H + MARGEM)) b->ativo = false;
        }


 
    }
   


void desenha_entidades(Game *g) {

    // linhas de aviso saem do centro da bala na direcao da acel e
    // atravessam a tela. so enquanto espera ou recua (vel contra a acel).
    for (int i = 0; i < MAX_BALAS; i++) {
        Bala *b = &g->balas[i];
        if (!b->ativo || !b->laser) continue;
        if (b->espera <= 0 && Vector2DotProduct(b->vel, b->acel) >= 0) continue;

        Vector2 ini = { b->r.x + b->r.width / 2, b->r.y + b->r.height / 2 };
        Vector2 fim = Vector2Add(ini, Vector2Scale(Vector2Normalize(b->acel), 1000));
        DrawLineEx(ini, fim, 1.5f, GREEN);
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

// debug (modo foco): contorno das hitboxes de todas as balas, sem recorte.
// vermelho = hitbox principal (r), amarelo = extra
void desenha_hitboxes(Game *g) {
    for (int i = 0; i < MAX_BALAS; i++) {
        Bala *b = &g->balas[i];
        if (!b->ativo) continue;

        if (b->oval)
            elipse_contorno(b->r, RED);
        else
            DrawRectangleLinesEx(b->r, 1, RED);

        if (b->extra.width > 0)
            DrawRectangleLinesEx((Rectangle){ b->r.x + b->extra.x, b->r.y + b->extra.y,
                                              b->extra.width, b->extra.height }, 1, YELLOW);
    }
}

