#include "game.h"


static void dano_player(Game *g) {
    g->hp -= 10;
    g->iframes = 1.0f;
}

// TODO: TB DEVE RECEBER ENDERECO DE TEXTURA COMO PARAMETRO
void spawn_bala(Game *g, Vector2 pos, Vector2 vel, bool recorta /*,Texture2D *textura*/) {
    int i;
    for (i = 0; i < MAX_BALAS; i++) {
        if (!g->balas[i].ativo) break;
    }
    if (i >= MAX_BALAS) return;

    Bala *b = &g->balas[i];

    b->r.height = g->balas[i].r.width = 5;
    b->r.x = pos.x;
    b->r.y = pos.y;

    b->vel.x = vel.x;
    b->vel.y = vel.y;

    b->ativo = true;
    b->recorta = recorta;   // o ataque decide se a bala e recortada pela caixa
    /*g->balas.textura = textura; */
}

void att_entidades(Game *g, float dt) {
    // mover balas

     for (int i = 0; i < MAX_BALAS; i++) {
        if (!g->balas[i].ativo) continue;
        Bala *b = &g->balas[i];

        b->r.x += b->vel.x*dt;
        b->r.y += b->vel.y*dt;

        Rectangle hitbox = { g->jogador.x - 8, g->jogador.y - 8, 8, 8};
        if (CheckCollisionRecs(hitbox, b->r) && g->iframes <= 0) dano_player(g);

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
            DrawRectangleRec(g->balas[i].r, WHITE);
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
            DrawRectangleRec(g->balas[i].r, WHITE);
        }
    }

    EndScissorMode();
}

