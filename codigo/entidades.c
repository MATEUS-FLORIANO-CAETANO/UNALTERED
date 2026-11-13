#include "game.h"

static void dano_player(Game *g) {
    g->hp -= 10;
    g->iframes = 0.3f;
}

void spawn_bala(Game *g, Vector2 pos, Vector2 vel) {
    // define um espaco pra bala e ativa
}

void att_entidades(Game *g, float dt) {
    // mover balas e testar colisão com jogador

    // pra cada bala, movimenta por velBala*dt
    // se estiver na hitbox do player, chama dano_player(g);
    // quando sair da tela, ativo = false;
}

void desenha_entidades(Game *g) {
    // decrementa imunidade
    //if (g->iframes > 0) g->iframes -= dt;
    
    // desenha balas;
    // pra ataques que so devem aparecer dentro das paredes, use BeginScissorMode(caixa) e dps EndScissorMode(caixa);
}

