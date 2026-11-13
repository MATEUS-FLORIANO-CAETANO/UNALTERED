#include "game.h"

static void dano_player(Game *g) {
    g->hp -= 10;
    g->iframes = 0.3f;
}

// typedef struct { Rectangle r; Vector2 vel; bool ativo; } Bala;
void spawn_bala(Game *g, Vector2 pos, Vector2 vel) {
    int i;
    for (i = 0; i < MAX_BALAS; i++) {
        if (!g->balas[i].ativo) break;
    }
    if (i >= MAX_BALAS) return;   // nenhuma bala livre

    g->balas[i].r.height = g->balas[i].r.width = 5;
    g->balas[i].r.x = pos.x;
    g->balas[i].r.y = pos.y;

    g->balas[i].vel.x = vel.x;
    g->balas[i].vel.y = vel.y;

    g->balas[i].ativo = true;
}

void att_entidades(Game *g, float dt) {
    // mover balas e testar colisão com jogador

     for (int i = 0; i < MAX_BALAS; i++) {
        if (g->balas[i].ativo) {
            g->balas[i].r.x += g->balas[i].vel.x*dt;
            g->balas[i].r.y += g->balas[i].vel.y*dt;
        }
    }
    
    // se estiver na hitbox do player, chama dano_player(g);
    // quando sair da tela, ativo = false;
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

