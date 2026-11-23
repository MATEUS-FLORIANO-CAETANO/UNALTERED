# Transparência sobre uso de IA

> Os usos de IA estão marcados com `// CLAUDE` no codigo

## Principal uso

Configurar builds pra Windows, web e Linux (genérico), e a criação da Makefile.

## Casos notaveis

1. Implementação da resolução virtual x real com 2x escala (para aumentar tamanho da tela sem precisar mexer nos valores internos)
2. Refatoracao de `ataques.c`, incluindo:
   - criacao de funções ajudantes
   - as partes de trigonometria
   - passeio aleatorio do golpe de barreiras
   - legibilidade em geral

   *(tudo revisado pela equipe)*
3. Bug fix de texturas e sons que nao carregavam devido ao caminho dos arquivos ser relativo ao local do executavel
4. Implementação de alguns efeitos
5. Conserto do rebate
6. Rotação de texturas

---

**A logica em geral, a estrutura do motor, e toda arte foi feita por humanos.**
