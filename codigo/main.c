#include "game.h"

// Clamp() 
#include "raymath.h"


Sound dano;

// o que main.c gerencia:
// 1. a unica definicao da struct que carrega os dados do jogo 
// 2. inicializa janela, desenha cada frame
// 2. Input do jogador 
// 4. Controle de estados (morreu? venceu?)
// 5. chama funcoes principais dos outros arquivos: atualiza_entidades, desenha_entidades, etc. 

// Definição de tipos e estruturas
// definicao: ESTADO DO JOGO; essa estrutura carrega todas as informações relevantes frame a frame.

// enum: permitir remapeamento de entrada para diferentes teclas.
// Ao inves de usar `IsKeyDown(KEY_LEFT)`, voce pode usar `IsActionDown(ACTION_LEFT)`, que define como quiser.
typedef enum ActionType {
    NO_ACTION = 0,
    ACTION_UP, ACTION_DOWN, ACTION_LEFT, ACTION_RIGHT,
    ACTION_PARRY,
    MAX_ACTION // 6
} ActionType;

// 
typedef struct ActionInput {
    int key;
    int button;
} ActionInput;

// Variaveis globais
static int gamepadIndex = 0; 
static ActionInput actionInputs[MAX_ACTION] = { 0 };


// declaracao de funcoes
static bool IsActionPressed(int action);    // 1 no primeiro frame da acao
static bool IsActionReleased(int action);   // 1 no ultimo frame da acao
static bool IsActionDown(int action);       // durante acao

static void SetActionsDefault(void);        // define WASD
static void SetActionsCursor(void);         // define setinhas
static void desenha_olho(Vector2 centro, Vector2 alvo); // do exemplo raylib shapes_following_eyes

int main(void)
{
    //-----------------------------------------------------------------------------------------
    // inicializacao
    Game g = {0};
    const int screenWidth = VIRTUAL_W;
    const int screenHeight = VIRTUAL_H;
    const int vel = VEL_JOGADOR;
    const int escala = 2;

    g.hp = 100;
    g.jogador.x = 320;
    g.jogador.y = 240 + 100;

    // arena padrao do undertale: interior 140x130, com borda 150x140
    g.caixa.width = 150;
    g.caixa.height = 140;

    // rectangle x e y sao o canto superior esquerdo, nao o centro.
    // subtrai metade da largura/altura pra centralizar a caixa na tela, depois desce.
    g.caixa.x = screenWidth / 2 - g.caixa.width / 2;
    g.caixa.y = screenHeight / 2 - g.caixa.height / 2 + 100;

    InitWindow(screenWidth * escala, screenHeight * escala, "UNALTERED");
    InitAudioDevice();
    
    
    // Escalar resolucao
    RenderTexture2D alvo = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(alvo.texture, TEXTURE_FILTER_POINT);

    // CLAUDE consertou:
    // assets sao relativo ao diretorio de trabalho.
    // o executavel fica em build/, mas a pasta assets/ fica na raiz do projeto,
    // um nivel acima: entra no diretorio do executavel e sobe um nivel.
    ChangeDirectory(GetApplicationDirectory());
    ChangeDirectory("..");
    dano = LoadSound("assets/snd-hurt1.ogg");      
    Texture2D alma = LoadTexture("assets/alma.png");

    // olhos: descomentar quando os arquivos existirem (ver desenha_olho)
    // Texture2D esclera = LoadTexture("assets/esclera.png");
    // Texture2D iris    = LoadTexture("assets/iris.png");

    //  Padrao WASD
    char actionSet = 0;
    SetActionsDefault();
    bool releaseAction = false;


    SetTargetFPS(60);

    //--------------------------------------------------------------------------------------
    // loop
    while (!WindowShouldClose()) 
    {
        float dt = GetFrameTime();

        // controle de estados:
        // ESTADO_MORTE e ESTADO_VITORIA n tem nada ainda
        switch (g.estado)
        {
        case ESTADO_JOGANDO:
        {
            // movimentacao
            gamepadIndex = 0; //
            if (IsActionDown(ACTION_UP)) g.jogador.y -= dt*vel;
            if (IsActionDown(ACTION_DOWN)) g.jogador.y += dt*vel;
            if (IsActionDown(ACTION_LEFT)) g.jogador.x -= dt*vel;
            if (IsActionDown(ACTION_RIGHT)) g.jogador.x += dt*vel;
            if (IsActionPressed(ACTION_PARRY))
            {
                // n faz nada ainda, placeholder pra uma "defesa"
            }

            // x e y da caixa nao contam borda. hitbox da alma fica no canto e nao no centro. / 2 ajeita
            float meiaW = alma.width  / 2.0f; // centro x
            float meiaH = alma.height / 2.0f; // centro y
            
            float xMin = g.caixa.x + BORDA + meiaW;
            float xMax = g.caixa.x + g.caixa.width  - BORDA - meiaW; // direita
            float yMin = g.caixa.y + BORDA + meiaH;
            float yMax = g.caixa.y + g.caixa.height - BORDA - meiaH;

            g.jogador.x = Clamp(g.jogador.x, xMin, xMax);
            g.jogador.y = Clamp(g.jogador.y, yMin, yMax);

            att_entidades(&g, dt);

            // sequencia de ondas retorna true quando todos os ataques acabam -> vitoria
            // TROCAR SE IMPLEMENTAR O PARRY
            if (att_ondas(&g, dt)) g.estado = ESTADO_VITORIA;

            
            if (g.hp <= 0) g.estado = ESTADO_MORTE;
        } break;

        case ESTADO_MORTE:
            // tela de morte (a implementar)
            break;

        case ESTADO_VITORIA:
            // tela de vitoria (a implementar)
            break;
        }

    /* parry, nao implementado
        releaseAction = false;
        if (IsActionReleased(ACTION_PARRY)) releaseAction = true;
    */

    /* TAB
        // tab: WASD <---> SETAS 
        if (IsKeyPressed(KEY_TAB))
        {
            actionSet = !actionSet;
            if (actionSet == 0) SetActionsDefault();
            else SetActionsCursor();
        }
    */

        //----------------------------------------------------------------------------------
        // desenhar


        // logica de upscaling com auxilio do CLAUDE. eu nao soube fazer - Luan

        // 1. desenha o jogo na resolucao virtual (640x480), dentro do alvo
        //    toda a logica usa essas coordenadas, entao nada aqui muda
        BeginTextureMode(alvo);

            ClearBackground(BLACK);
            DrawRectangleLinesEx(g.caixa, 5, WHITE);
            // DrawTexture desenha a partir do canto superior esquerdo
            // desloca metade do tamanho pra centralizar a alma na posicao do jogador
            DrawTexture(alma, g.jogador.x - alma.width / 2.0f, g.jogador.y - alma.height / 2.0f, WHITE);

             DrawText(TextFormat("HP: %d; ESTADO: %d", g.hp, g.estado), 10, 50, 20, GREEN);

            desenha_entidades(&g);

            // olhos acima da caixa, seguindo a alma
            desenha_olho((Vector2){ screenWidth / 2.0f - 50, 130 }, g.jogador);
            desenha_olho((Vector2){ screenWidth / 2.0f + 50, 130 }, g.jogador);

        EndTextureMode();

        
        // 2. estica o alvo pra janela inteira, mantendo a proporcao
        BeginDrawing();

            ClearBackground(BLACK);
            DrawTexturePro(
                alvo.texture,
                // altura NEGATIVA: raylib guarda a render texture invertida no eixo Y
                (Rectangle){ 0, 0, (float)alvo.texture.width, -(float)alvo.texture.height },
                (Rectangle){ 0, 0, (float)screenWidth * escala, (float)screenHeight * escala },
                (Vector2){ 0, 0 }, 0.0f, WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // fechamento
    UnloadRenderTexture(alvo);
    UnloadSound(dano);
    // UnloadTexture(esclera);
    // UnloadTexture(iris);
    CloseWindow();
       

    CloseAudioDevice();     
    return 0;
}

//----------------------------------------------------------------------------------
// definicao de funcoes 

// verifica botao pressionado
static bool IsActionPressed(int action)
{
    bool result = false;
    if (action < MAX_ACTION) result = (IsKeyPressed(actionInputs[action].key) || 
                                       IsGamepadButtonPressed(gamepadIndex, actionInputs[action].button));
    return result;
}

// verifica botao solto
static bool IsActionReleased(int action)
{
    bool result = false;
    if (action < MAX_ACTION) result = (IsKeyReleased(actionInputs[action].key) || 
                                       IsGamepadButtonReleased(gamepadIndex, actionInputs[action].button));
    return result;
}

// verifica botao segurando
static bool IsActionDown(int action)
{
    bool result = false;
    if (action < MAX_ACTION) result = (IsKeyDown(actionInputs[action].key) || 
                                       IsGamepadButtonDown(gamepadIndex, actionInputs[action].button));
    return result;
}

// ativa WASD
static void SetActionsDefault(void)
{
    actionInputs[ACTION_UP].key = KEY_W;
    actionInputs[ACTION_DOWN].key = KEY_S;
    actionInputs[ACTION_LEFT].key = KEY_A;
    actionInputs[ACTION_RIGHT].key = KEY_D;
    actionInputs[ACTION_PARRY].key = KEY_SPACE;

    actionInputs[ACTION_UP].button = GAMEPAD_BUTTON_LEFT_FACE_UP;
    actionInputs[ACTION_DOWN].button = GAMEPAD_BUTTON_LEFT_FACE_DOWN;
    actionInputs[ACTION_LEFT].button = GAMEPAD_BUTTON_LEFT_FACE_LEFT;
    actionInputs[ACTION_RIGHT].button = GAMEPAD_BUTTON_LEFT_FACE_RIGHT;
    actionInputs[ACTION_PARRY].button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN;
}

// ativa setas
static void SetActionsCursor(void)
{
    actionInputs[ACTION_UP].key = KEY_UP;
    actionInputs[ACTION_DOWN].key = KEY_DOWN;
    actionInputs[ACTION_LEFT].key = KEY_LEFT;
    actionInputs[ACTION_RIGHT].key = KEY_RIGHT;
    actionInputs[ACTION_PARRY].key = KEY_SPACE;

    actionInputs[ACTION_UP].button = GAMEPAD_BUTTON_RIGHT_FACE_UP;
    actionInputs[ACTION_DOWN].button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN;
    actionInputs[ACTION_LEFT].button = GAMEPAD_BUTTON_RIGHT_FACE_LEFT;
    actionInputs[ACTION_RIGHT].button = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT;
    actionInputs[ACTION_PARRY].button = GAMEPAD_BUTTON_LEFT_FACE_DOWN;
}

// olho que segue o alvo
// adaptado de raylib/examples/shapes/shapes_following_eyes.c
static void desenha_olho(Vector2 centro, Vector2 alvo)
{
    const float raioEsclera = 40.0f;
    const float raioIris = 12.0f;

    Vector2 iris = alvo;
    // se o alvo esta fora do alcance, prende a iris na borda da esclera na direcao do alvo
    if (!CheckCollisionPointCircle(iris, centro, raioEsclera - raioIris))
    {
        float angulo = atan2f(alvo.y - centro.y, alvo.x - centro.x);
        iris.x = centro.x + (raioEsclera - raioIris) * cosf(angulo);
        iris.y = centro.y + (raioEsclera - raioIris) * sinf(angulo);
    }

    DrawCircleV(centro, raioEsclera, LIGHTGRAY);
    DrawCircleV(iris, raioIris, BROWN);
    DrawCircleV(iris, 5, BLACK);

    /* versao com textura (quando assets/esclera.png e assets/iris.png existirem):
       1. assinatura vira: desenha_olho(Vector2 centro, Vector2 alvo, Texture2D texEsclera, Texture2D texIris)
          (atualizar a declaracao la em cima e as duas chamadas: ..., g.jogador, esclera, iris)
       2. raios vem do tamanho das imagens:
            const float raioEsclera = texEsclera.width / 2.0f;
            const float raioIris    = texIris.width / 2.0f;
       3. troca os tres DrawCircleV por (DrawTexture desenha do canto superior esquerdo, entao centraliza):
            DrawTexture(texEsclera, centro.x - texEsclera.width / 2.0f, centro.y - texEsclera.height / 2.0f, WHITE);
            DrawTexture(texIris,    iris.x   - texIris.width    / 2.0f, iris.y   - texIris.height    / 2.0f, WHITE);
    */
}
