#include "game.h"

// o que main.c gerencia:
// 1. a unica definicao da struct que carrega os dados do jogo (OK)
// 2. inicializa janela, desenha cada frame
// 2. Input do jogador OK
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

int main(void)
{
    //-----------------------------------------------------------------------------------------
    // inicializacao
    Game g = {0};
    const int screenWidth = 640;
    const int screenHeight = 480;
    const int vel = 120;
    const int escala = 2;

    g.hp = 100;
    g.jogador.x = 320;
    g.jogador.y = 240 + 70;

    // arena padrao do undertale: interior ~140x130, exterior ~150x140 com a borda.
    // g.caixa = exterior (que e desenhado); borda de 5px por lado => inner = outer - 10.
    g.caixa.width = 150;
    g.caixa.height = 140;
    // rectangle.x/.y sao o canto superior esquerdo, nao o centro.
    // subtrai metade da largura/altura pra centralizar a caixa na tela, depois desce.
    g.caixa.x = screenWidth / 2 - g.caixa.width / 2;
    g.caixa.y = screenHeight / 2 - g.caixa.height / 2 + 70;

    // g.caixa e a aparencia (outer): a borda de 5px e desenhada pra dentro.
    // inner e a caixa encolhida 5px em cada lado = area jogavel real,
    // usada pra prender a alma e pra logica de colisao das balas.
    // (a colisao vive em att_entidades; calcular inner la a partir de g->caixa)
    // Rectangle inner = {
    //     g.caixa.x + 5,
    //     g.caixa.y + 5,
    //     g.caixa.width  - 2*5,
    //     g.caixa.height - 2*5
    // };

    InitWindow(screenWidth * escala, screenHeight * escala, "UNALTERED");

    // Escalar resolucao; 
    RenderTexture2D alvo = LoadRenderTexture(screenWidth, screenHeight);
    SetTextureFilter(alvo.texture, TEXTURE_FILTER_POINT);

    // assets sao resolvidos relativo ao diretorio de trabalho.
    // o executavel fica em build/, mas a pasta assets/ fica na raiz do projeto,
    // um nivel acima: entra no diretorio do executavel e sobe um nivel.
    ChangeDirectory(GetApplicationDirectory());
    ChangeDirectory("..");
    Texture2D alma = LoadTexture("assets/alma.png");

    //  Padrao WASD
    char actionSet = 0;
    SetActionsDefault();
    bool releaseAction = false;


    SetTargetFPS(60);
    
    spawn_bala(&g, (Vector2){200, 200}, (Vector2){80, 60});

    //--------------------------------------------------------------------------------------
    // loop
    while (!WindowShouldClose()) // Detecta ESC ou botão de fechar no windows
    {
        float dt = GetFrameTime();
        // movimentacao
        gamepadIndex = 0; // 
        if (IsActionDown(ACTION_UP)) g.jogador.y -= dt*vel;
        if (IsActionDown(ACTION_DOWN)) g.jogador.y += dt*vel;
        if (IsActionDown(ACTION_LEFT)) g.jogador.x -= dt*vel;
        if (IsActionDown(ACTION_RIGHT)) g.jogador.x += dt*vel;
        if (IsActionPressed(ACTION_PARRY))
        {
            // nao faz nada ainda, placeholder pra uma "defesa"
        }

        att_entidades(&g, dt);

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

        // 1. desenha o jogo na resolucao virtual (640x480), dentro do alvo.
        //    toda a logica usa essas coordenadas, entao nada aqui muda.
        BeginTextureMode(alvo);

            ClearBackground(BLACK);
            DrawRectangleLinesEx(g.caixa, 5, WHITE);
            // DrawTexture desenha a partir do canto superior esquerdo;
            // desloca metade do tamanho pra centralizar a alma na posicao do jogador.
            DrawTexture(alma, g.jogador.x - alma.width / 2.0f, g.jogador.y - alma.height / 2.0f, WHITE);

            desenha_entidades(&g);

        EndTextureMode();

        
        // 2. estica o alvo pra janela inteira, mantendo a proporcao.
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
    CloseWindow();
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
