#### 🎮 **README para `A3-RunAndGun/`**

```markdown
# A3: Shooter Born in Heaven (Jogo Run 'n Gun)

**Disciplina:** Programação 2
**Período:** 2025/1
**Autor:** Pietro Comin (GRR20241955)

> [!WARNING]
> **Este foi o projeto mais desafiador e gratificante do semestre, com mais de 50 horas de desenvolvimento dedicadas.** O resultado é um jogo de ação 2D funcional do gênero *run 'n gun*, inspirado em clássicos como *Contra* e *Metal Slug*.

## 🕹️ Descrição do Jogo

**Shooter Born in Heaven** é um jogo de ação 2D desenvolvido em Linguagem C com a biblioteca Allegro 5. O jogador assume o papel de um soldado de elite que deve avançar por um cenário urbano hostil, enfrentando ondas de gangsters e culminando em uma batalha intensa contra um poderoso chefe final.

## 🚀 Como Compilar e Executar

O projeto utiliza um `makefile` para facilitar todo o processo.

**Requisitos:**
* GCC (ou um compilador C compatível)
* Make
* Biblioteca Allegro 5 e seus addons (image, font, ttf, primitives)

**Passos:**
1.  Abra um terminal na raiz do projeto.
2.  Compile o jogo:
    ```bash
    make
    ```
3.  Execute o jogo:
    ```bash
    make run
    ```
4.  Para limpar os arquivos gerados:
    ```bash
    make clean
    ```

## ⌨️ Controles do Jogo

| Ação                      | Teclado Principal         | Teclado Alternativo         |
| ------------------------- | ------------------------- | --------------------------- |
| **Movimentar** | `A` / `D`                 | Seta Esquerda / Seta Direita|
| **Pular** | `W`                       | Seta Cima                   |
| **Abaixar** | `S`                       | Seta Baixo                  |
| **Correr** | `Shift Esquerdo`          | -                           |
| **Atirar** | `Barra de Espaço`         | `Shift Direito`             |
| **Recarregar** | `R`                       | -                           |
| **Pausar** | `P`                       | -                           |
| **Modo Debug (Hitbox)** | `H`                       | -                           |
| **Sair / Voltar** | `ESC`                     | `ESC`                       |

## ✨ Funcionalidades Implementadas

O jogo não apenas cumpriu todos os requisitos mínimos, mas também incluiu diversas funcionalidades extras que enriqueceram a experiência.

### Requisitos Mínimos (80/80 pontos)
* **Menus e Telas:** Menu inicial (Iniciar, Opções, Sair) e telas de vitória/derrota.
* **Personagem e Animações:** Sprites e animações para todos os estados (parado, andando, correndo, pulando, abaixado, atirando).
* **Mobilidade Total:** O jogador pode andar, pular, se abaixar e atirar em todas as direções e estados.
* **Combate:** Sistema de vida, ataque com projéteis para jogador e inimigos.
* **Cenário:** Background com efeito de rolagem lateral (parallax).
* **Inimigos:** Um inimigo comum (atirador) e um chefe final com IA própria.
* **Estrutura de Fase:** Ondas de inimigos que levam à batalha contra o chefe.

### Funcionalidades Extras (do Enunciado)
* **[15 pts] Inimigo Normal Extra:** Implementado um inimigo de ataque corpo a corpo (`ENEMY_MELEE`).
* **[10 pts] Abaixar e Atirar Abaixado:** O jogador pode se defender e atacar de uma posição abaixada.
* **[10 pts] Sistema de Estamina:** Correr consome uma barra de estamina, que se regenera com o tempo.
* **[5 pts] Botão de Pausa:** O jogo pode ser pausado a qualquer momento.

### Funcionalidades Adicionais (Além do Enunciado)
* **Sistema de Recarga de Munição:** O jogador tem munição limitada e precisa recarregar (`R`), adicionando uma camada tática ao combate.
* **Sistema de Ondas de Inimigos:** Inimigos surgem em ondas com dificuldade progressiva.
* **IA Avançada do Chefe:** O chefe possui múltiplos padrões de ataque, um modo "Fúria" ativado com 50% de vida e um sistema de "Poise" que o torna resistente a atordoamentos.
* **Modo "Freeplay" Pós-Vitória:** Após vencer, o jogador pode continuar na fase, e o chefe pode reaparecer para novos desafios.
* **Modo de Depuração:** A tecla `H` ativa a visualização de todas as *hitboxes*, ferramenta essencial para o desenvolvimento e balanceamento.

## 📂 Estrutura do Projeto
O código foi organizado de forma modular para garantir a separação de responsabilidades.
* `src/`: Contém todos os arquivos de código-fonte (.c).
* `include/`: Contém todos os arquivos de cabeçalho (.h).
* `assets/`: Contém todos os recursos gráficos (sprites, backgrounds) e fontes.
