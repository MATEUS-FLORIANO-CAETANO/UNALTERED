#include "game.h"


static void dano_player(Game *g) {
    g->hp -= 10;
    g->iframes = 1.0f;
}

void spawn_bala(Game *g, Vector2 pos, Vector2 vel, Vector2 tamanho, bool recorta, Texture2D *textura, float escala) {
    int i;
    for (i = 0; i < MAX_BALAS; i++) {
        if (!g->balas[i].ativo) break;
    }
    if (i >= MAX_BALAS) return;

    Bala *b = &g->balas[i];

    // hitbox independente do sprite: o ataque passa o tamanho, pois a imagem pode
    // ter bordas transparentes (ex: gota ~10x13 dentro de um png 32x32)
    b->r.width  = tamanho.x;
    b->r.height = tamanho.y;
    b->r.x = pos.x;
    b->r.y = pos.y;

    b->vel.x = vel.x;
    b->vel.y = vel.y;

    b->ativo = true;
    b->recorta = recorta;   // o ataque decide se a bala e recortada pela caixa
    b->textura = textura;   // o ataque decide a aparencia da bala
    b->escala = escala;     // o ataque decide o tamanho do desenho
}

// desenha uma bala: o sprite fica centralizado na hitbox (a imagem costuma ser
// maior que a hitbox por causa das bordas transparentes). sem sprite, um quadrado.
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

        b->r.x += b->vel.x*dt;
        b->r.y += b->vel.y*dt;

        // hitbox da alma: 8x8 centrado no jogador (jogador = centro do sprite).
        // antes deslocava pro canto superior-esquerdo (-8 em vez de -4), por isso
        // os acertos registravam fora da alma.
        Rectangle hitbox = { g->jogador.x - 4, g->jogador.y - 4, 8, 8 };
        if (CheckCollisionRecs(hitbox, b->r)) {
            if (g->iframes <= 0) dano_player(g);
            b->ativo = false;   // a gota some ao encostar na alma
        }

           // quando sair da tela, ativo = false;
    if (b->r.x < -20 || b->r.x > 660 || b->r.y < -20 || b->r.y > 500) b->ativo = false;
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

    // balas com recorte: so aparecem dentro da caixa (ex: chuva, que nasce acima
    // das paredes).
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

