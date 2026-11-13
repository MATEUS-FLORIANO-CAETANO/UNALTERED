# UNALTERED

* Este projeto é uma especie de "demake" do sistema de combate dos jogos UNDERTALE e DELTARUNE
* 
* Marcos Luan dos Santos
* Rafael Zapora
* Mateus Floriano Caetano

Arquitetura:
## game.h
Header. todo arquivo .c inclui ele. declara funções e protótipos de estruturas que os outros arquivos usam, como o struct Game que mantém o estado do jogo.

## main.c 
Gerencia:
1. definição de Game g, a struct que segura todos os dados do jogo a cada frame. É passada por referência a tudo que manipula o estado do jogo.
2. inicializa janela e inicia o loop de desenho.
3. Input do jogador
4. Controle de estados (morreu? venceu?)
5. chama funcoes principais dos outros arquivos: atualiza_entidades, desenha_entidades, etc.
 
## entidades.c
 responsável por realmente criar as balas e atualizar a posição das mesmas, e verificar colisão.
 ## ataques.c

padrões de ataque e sequência de ondas.
decide quando e onde as coisas nascem, por meio de funções que chamam spawn_bala da entidades.c (que recebe tamanho, posicao, e velocidade) pra gerar determinado padrão de ataque.
também gerencia as sequencias de ataques, com duração e intervalo entre cada bala.
NÃO move balas e nem verifica colisão, isso é trabalho do entidades.c

