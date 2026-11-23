# UNALTERED

> Este projeto é uma especie de "demake" do sistema de combate do jogo UNDERTALE

### Equipe

- Marcos Luan dos Santos
- Rafael Zapora
- Mateus Floriano Caetano

---

## Como jogar

WASD/setas (apertando tab) -> movimentação
ESPAÇO -> parry, somente balas laranjas (você vai saber quando)

---

## Rodar

Os executáveis estão em `build/`.

Por padrão é só rodar, mas se for retirar da pasta certifique-se que o diretório deles está adjacente ao dos `assets`.

---

## Como compilar

### NixOS

```sh
cd build && nix-shell --run make && ./unaltered
```

### Ubuntu / Debian (sem Nix)

raylib não está no apt, então compile do fonte uma vez:

```sh
sudo apt install build-essential git libasound2-dev libx11-dev libxrandr-dev \
  libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev
git clone --depth 1 --branch 5.5 https://github.com/raysan5/raylib.git
cd raylib/src && make PLATFORM=PLATFORM_DESKTOP
# sem sudo: copia pra ~/.local
mkdir -p ~/.local/lib ~/.local/include
cp libraylib.a ~/.local/lib/ && cp raylib.h raymath.h rlgl.h ~/.local/include/
# (ou, com root: sudo make install, que vai pra /usr/local)
```

Depois:

```sh
cd build && make && ./unaltered
```

O Makefile usa `pkg-config` se achar raylib; senão procura em `~/.local` e depois `/usr/local`
(mude com `make RAYLIB_PREFIX=/outro/caminho`).

---

## Arquitetura

### `game.h` — header

Todo arquivo `.c` inclui ele. Declara funções e protótipos de estruturas que os outros arquivos usam, como o `struct Game` que mantém o estado do jogo.

### `main.c`

Gerencia:

1. Definição de `Game g`, a struct que segura todos os dados do jogo a cada frame. É passada por referência a tudo que manipula o estado do jogo.
2. Inicializa janela e inicia o loop de desenho.
3. Input do jogador
4. Controle de estados (morreu? venceu?)
5. Chama funcoes principais dos outros arquivos: `atualiza_entidades`, `desenha_entidades`, etc.

### `entidades.c`

Responsável por realmente criar as balas e atualizar a posição das mesmas, e verificar colisão.

### `ataques.c`

Padrões de ataque e sequência de ondas.

Decide quando e onde as coisas nascem, por meio de funções que chamam `spawn_bala` da `entidades.c` (que recebe tamanho, posicao, e velocidade) pra gerar determinado padrão de ataque.

Também gerencia as sequencias de ataques, com duração e intervalo entre cada bala.
não move balas e nem verifica colisão, isso fica no `entidades.c`
