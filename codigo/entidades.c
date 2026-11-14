#include "game.h"


static void dano_player(Game *g) {
    g->hp -= 10;
    g->iframes = 1.0f;
}

// TODO: TB DEVE RECEBER ENDERECO DE TEXTURA COMO PARAMETRO
void spawn_bala(Game *g, Vector2 pos, Vector2 vel /*,Texture2D *textura*/) {
    int i;
    for (i = 0; i < MAX_BALAS; i++) {
        if (!g->balas[i].ativo) break;
    }
    if (i >= MAX_BALAS) return;   

    g->balas[i].r.height = g->balas[i].r.width = 5;
    g->balas[i].r.x = pos.x;
    g->balas[i].r.y = pos.y;

    g->balas[i].vel.x = vel.x;
    g->balas[i].vel.y = vel.y;

    g->balas[i].ativo = true;
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
    
    // desenha balas
    for (int i = 0; i < MAX_BALAS; i++) {
        if (g->balas[i].ativo) {
            DrawRectangleRec(g->balas[i].r, WHITE);
        }
    }
 
    // pra ataques que so devem aparecer dentro das paredes, use BeginScissorMode(caixa) e dps EndScissorMode(caixa);
}

