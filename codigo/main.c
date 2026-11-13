/*******************************************************************************************
*
* Este projeto é uma especie de "demake" do sistema de combate dos jogos UNDERTALE e DELTARUNE
* 
* Marcos Luan dos Santos
* Rafael Zapora
* Mateus Floriano Caetano
*
* Códigos utilizados como Base:
* https://github.com/raysan5/raylib/blob/master/examples/core/core_input_actions.c
* https://github.com/raysan5/raylib/blob/master/examples/shapes/shapes_following_eyes.c
*
********************************************************************************************/


#include "game.h"
// o que main.c gerencia:
// 1. a unica definicao da struct que carrega os dados do jogo OK
// 2. inicializa janela, desenha cada frame
// 2. Input do jogador OK
// 4. Controle de estados (morreu? venceu?)
// 5. chama funcoes principais dos outros arquivos: atualiza_entidades, desenha_entidades, etc. 

// Definição de tipos e estruturas
// definicao: ESTADO DO JOGO; essa estrutura carrega todas as informações relevantes frame a frame.
Game g = {0};


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
    const int screenWidth = 640;
    const int screenHeight = 480;
    const int vel = 240;
    g.jogador.x = 320;
    g.jogador.y = 240;


    InitWindow(screenWidth, screenHeight, "UNALTERED");

    Texture2D alma = LoadTexture("assets/alma.png");

    //  Padrao WASD
    char actionSet = 0;
    SetActionsDefault();
    bool releaseAction = false;


    SetTargetFPS(60);
    
    //--------------------------------------------------------------------------------------
    // loop
    while (!WindowShouldClose()) // Detecta ESC ou botão de fechar no windows
    {
        int dt = GetFrameTime();
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

        /* 
            Organizar chamados:
            spawn_bala(Game *g, Vector2 pos, Vector2 vel);
            att_entidades(Game *g, float dt);
            desenha_entidades(Game *g);
        */

        releaseAction = false;
        if (IsActionReleased(ACTION_PARRY)) releaseAction = true;
        // tab: WASD <---> SETAS 
        if (IsKeyPressed(KEY_TAB))
        {
            actionSet = !actionSet;
            if (actionSet == 0) SetActionsDefault();
            else SetActionsCursor();
        }

        //----------------------------------------------------------------------------------
        // desenhar 
 
        BeginDrawing();

            ClearBackground(BLACK);
            DrawTexture(alma, g.jogador.x, g.jogador.y, WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // fechamento
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
