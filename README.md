# Igreja 3D - Projeto de Computação Gráfica

## Sobre o Projeto

Este é um projeto desenvolvido para a disciplina de **Computação Gráfica** da **Universidade Federal de Alagoas (UFAL)**. O projeto consiste em uma simulação 3D interativa de uma igreja moderna, implementada em C++ usando OpenGL e GLUT.

## Características do Projeto

### 🏛️ Estrutura da Igreja
- **Fachada em A-Frame**: Estrutura moderna com design em V invertido
- **Entrada Realista**: Porta principal com moldura detalhada
- **Paredes e Teto**: Estrutura completa com materiais realistas
- **Piso**: Superfície texturizada com tons naturais

### 🪟 Janelas e Iluminação
- **Vitrais Coloridos**: Janelas laterais com vidros coloridos
- **Sistema de Iluminação**: Luz ambiente, spotlight no altar e lanterna do jogador
- **Transparência**: Blending configurado para os vitrais

### 🪑 Mobiliário e Objetos
- **Altar**: Base em mármore e degraus
- **Ambão**: Estrutura de madeira posicionada à frente
- **Crucifixo**: Madeira escura na parede do fundo
- **Estátuas**: Virgem Maria e São José
- **Cadeiras**: Layout organizado voltado ao altar
- **Cruz Processional**: Dourada

### 🌳 Ambiente Externo
- **Jardim**: Palmeiras laterais
- **Caminho Frontal**: Piso até a porta
- **Gramado**: Área verde ao redor

### 🚪 Interatividade
- **Porta**: Abre/fecha com a tecla E / botão visual
- **Sistema de Colisão**: Colisão com paredes, cadeiras e objetos principais
- **Modo Voo**: Alterna com V 
- **Lanterna**: Tecla F

## Controles

| Tecla | Função |
|-------|--------|
| **W, A, S, D** | Movimento (frente, esquerda, trás, direita) |
| **Mouse** | Olhar ao redor (quando capturado) |
| **V** | Alternar modo voo |
| **F** | Ligar/desligar lanterna |
| **E** | Abrir/fechar porta |
| **R** | Resetar posição |
| **M** | Capturar/liberar mouse |
| **Shift** | Correr (movimento mais rápido) |
| **Espaço** | Subir (apenas no modo voo) |
| **C** | Descer (apenas no modo voo) |
| **ESC** | Sair do programa |

## Tecnologias Utilizadas

- **C++**: Linguagem de programação principal
- **OpenGL**: API gráfica para renderização 3D
- **GLUT (FreeGLUT)**: Biblioteca para gerenciamento de janelas e entrada
- **GCC/G++**: Compilador C++

## Estrutura do Projeto

```
projetocg/
├── igreja.cpp          # Código fonte principal
└── README.md           # Este arquivo
```

## Compilação e Execução

### Windows 

**Pré-requisitos:**
- MinGW ou Visual Studio
- FreeGLUT
- OpenGL

**Compilação:**
```bash
g++ igreja.cpp -o igreja_final.exe -lfreeglut -lopengl32 -lglu32
```

**Execução:**
```bash
./igreja_final.exe
```
## Aspectos Técnicos

### Renderização
- **Pipeline OpenGL**: Renderização em tempo real
- **Culling**: Otimização de faces não visíveis
- **Depth Testing**: Teste de profundidade para objetos
- **Blending**: Transparência para vidros e vitrais

### Iluminação
- **Modelo**: `GL_LIGHTING` habilitado com `GL_COLOR_MATERIAL` (difusa/ambiente pelo `glColor*`). Ambiente global via `glLightModelfv`.
- **Materiais**: Especular definido com `glMaterialfv` e `glMaterialf` (`shininess` ≈ 18), `GL_LIGHT_MODEL_TWO_SIDE` ativo. Emissão usada sutilmente no piso.
- **Luzes internas**:
  - `GL_LIGHT0` (teto): luz principal difusa e especular suaves.
  - `GL_LIGHT2` (altar): ponto de luz no altar, tom neutro.
  - `GL_LIGHT3` (direcional): pseudo “sol” entrando da fachada para o interior.
  - `GL_LIGHT4` (altar quente): ponto com atenuação linear (0.04) para tonalidade mais quente.
- **Luzes externas**:
  - `GL_LIGHT5` (spot fachada): `SPOT_CUTOFF` ≈ 22°, `SPOT_EXPONENT` ≈ 14, atenuação linear ≈ 0.03.
  - `GL_LIGHT6` (direcional céu): preenchimento frio e suave.
  - `GL_LIGHT7` (bounce do piso): direcional de baixo para cima, de baixa intensidade.
- **Lanterna (F)**: `GL_LIGHT1` spot acoplado à câmera com `SPOT_CUTOFF` ≈ 25°, `SPOT_EXPONENT` ≈ 12 e atenuação linear ≈ 0.06.
- **Transparências específicas**: Desliga `GL_LIGHTING` ao desenhar elementos translúcidos (ex.: chamas/vidros), habilita `GL_BLEND` e depois restaura a iluminação.

### Texturas 
- **Geração**: Totalmente procedurais em CPU, convertidas para texturas com mipmaps via `gluBuild2DMipmaps`.
- **Parâmetros**: `GL_TEXTURE_MIN_FILTER = GL_LINEAR_MIPMAP_LINEAR`, `GL_TEXTURE_MAG_FILTER = GL_LINEAR`, `GL_TEXTURE_WRAP_{S,T} = GL_REPEAT`.
- **Conjunto utilizado**:
  - `TEX_REFLECTIVE_TILES`: piso interno com leve emissão simulando reflexo ambiental.
  - `TEX_PLASTER`: reboco das paredes internas.
  - `TEX_MARBLE`: mármore (altar, painel do crucifixo, detalhes).
  - `TEX_WOOD`: madeira (porta, ambão, detalhes).
  - `TEX_STONE`: pedra (elementos estruturais/degmaros/fachada).
  - `TEX_TILE`: caminho frontal externo.
  - `TEX_GRASS`: gramado externo.
  - `TEX_CROSS`: textura procedural específica para a cruz.
- **Aplicação**: uso de `drawTexturedQuad` com repetição de UV controlada por `uRepeat`/`vRepeat` para evitar estiramento; `glBindTexture` é habilitado/desabilitado por objeto.

### Física
- **Colisão**: Paredes, cadeiras e objetos do altar; deslizamento em quinas
- **Movimento**: Primeira pessoa com WASD

### Otimizações
- **Frustum Culling**: Renderização apenas do que está visível
- **LOD**: Níveis de detalhe para objetos distantes
- **Batch Rendering**: Agrupamento de objetos similares

## Funcionalidades Implementadas


- Estrutura da igreja (paredes, teto, piso)
- Câmera em primeira pessoa
- Iluminação
- Objetos principais (altar, ambão, cruz, estátuas)
- Ambiente externo (caminho + gramado)
- Porta interativa
- Sistema de colisão (paredes, cadeiras e altar)
- Modo voo
- Lanterna do jogador




