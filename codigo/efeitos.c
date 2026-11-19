// efeitos de impacto: congelar o jogo (hitstop) e tremer a tela.
// estado fica so aqui. pra desligar, apaga as chamadas efeito_* (main.c, entidades.c, parry.c)

#include "game.h"

#define DANO_PAUSA   0.06f   // segs. congelado
#define DANO_TREMOR  0.25f   // segs. tremendo
#define DANO_FORCA   4.0f    // px

#define PARRY_PAUSA  0.10f
#define PARRY_TREMOR 0.35f
#define PARRY_FORCA  2.0f

#define REBATE_PAUSA  0.0f   // inimigo devolvendo: so treme
#define REBATE_TREMOR 0.35f
#define REBATE_FORCA  2.0f

static float pausa;     // segs. congelado restantes
static float tremor;    // segs. tremendo restantes
static float duracao;   // duracao total do tremor atual
static float forca;     // px de deslocamento maximo

// nunca encurta um efeito que ja esta rolando
static void impacto(float p, float t, float f) {
    if (p > pausa) pausa = p;
    if (t > tremor) { tremor = t; duracao = t; }
    if (f > forca || tremor <= 0) forca = f;
}

void efeito_dano(void)  { impacto(DANO_PAUSA,  DANO_TREMOR,  DANO_FORCA);  }
void efeito_parry(void) { impacto(PARRY_PAUSA, PARRY_TREMOR, PARRY_FORCA); }
void efeito_rebate(void) { impacto(REBATE_PAUSA, REBATE_TREMOR, REBATE_FORCA); }

// 1x por frame. true = jogo congelado nesse frame
bool efeito_congelado(float dt) {
    if (tremor > 0) tremor -= dt;
    else forca = 0;
    if (pausa > 0) { pausa -= dt; return true; }
    return false;
}

// deslocamento da tela nesse frame (px da resolucao virtual). enfraquece com o tempo
Vector2 efeito_tremor(void) {
    if (tremor <= 0) return (Vector2){ 0, 0 };
    float f = forca * (tremor / duracao);
    return (Vector2){ GetRandomValue(-100, 100) / 100.0f * f,
                      GetRandomValue(-100, 100) / 100.0f * f };
}
