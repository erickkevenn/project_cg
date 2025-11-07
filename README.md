# Igreja 3D - Projeto de Computação Gráfica

## Sobre o Projeto

Este é um projeto desenvolvido para a disciplina de **Computação Gráfica** da **Universidade Federal de Alagoas (UFAL)**. O projeto consiste em uma simulação 3D interativa de uma igreja moderna, implementada em C++ usando OpenGL e GLUT.

## Características do Projeto

### 🏛️ Estrutura da Igreja
- **Fachada em A-Frame**: Estrutura moderna com design em V invertido
- **Entrada Realista**: Porta principal com moldura detalhada
- **Paredes e Teto**: Estrutura completa com materiais realistas
- **Piso**: Superfície texturizada com azulejos reflexivos
- **Parede do Altar**: Textura de mármore branco suave com veios sutis
- **Parede Externa Frontal**: Textura de concreto/reboco desgastado (sem cobrir a porta)

### 🪟 Janelas e Iluminação
- **Vitrais Coloridos**: Janelas laterais com vidros coloridos (verde, laranja, amarelo, azul)
- **Sistema de Iluminação Avançado**: 
  - Luz ambiente global
  - Luzes LED embutidas no teto (simulação visual)
  - Spotlight no altar
  - Lanterna do jogador (tecla F)
  - Luz direcional simulando sol
  - Luz quente no altar
  - Luzes externas para fachada
- **Toggle de Iluminação**: Tecla L para ligar/desligar toda a iluminação
- **Janelas Dinâmicas**: Escurecem automaticamente quando a iluminação está desligada
- **Transparência**: Blending configurado para os vitrais

### 🪑 Mobiliário e Objetos
- **Altar**: Base em mármore com degraus de pedra, painel frontal preto com Agnus Dei dourado, toalha branca e bíblia aberta
- **Ambão**: Estrutura de madeira com símbolo eclesiástico em mármore
- **Crucifixo**: Madeira escura na parede do fundo com Cristo realista
- **Estátuas**: Virgem Maria e São José nas laterais do altar
- **Cadeiras**: Layout organizado voltado ao altar (cadeiras brancas de plástico)
- **Cruz Processional**: Dourada posicionada ao lado do altar
- **Lustre**: Lustre decorativo com cristais pendentes acima do altar
- **Ventiladores de Teto**: Ventiladores posicionados estrategicamente ao lado das janelas

### 🌳 Ambiente Externo
- **Jardim**: Palmeiras altas nas laterais
- **Caminho Frontal**: Piso texturizado até a porta
- **Gramado**: Área verde texturizada ao redor da igreja

### 🚪 Interatividade
- **Porta**: Abre/fecha com a tecla E (animação suave)
- **Sistema de Colisão**: Colisão com paredes, cadeiras e objetos principais
- **Modo Voo**: Alterna com V (movimento livre no espaço 3D)
- **Lanterna**: Tecla F (spotlight acoplado à câmera)
- **Toggle de Iluminação**: Tecla L (liga/desliga todas as luzes e escurece janelas)

## Controles

| Tecla | Função |
|-------|--------|
| **W, A, S, D** | Movimento (frente, esquerda, trás, direita) |
| **Mouse** | Olhar ao redor (quando capturado) |
| **V** | Alternar modo voo |
| **F** | Ligar/desligar lanterna |
| **E** | Abrir/fechar porta |
| **L** | Ligar/desligar iluminação (escurece janelas também) |
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
├── igreja_final.exe    # Executável compilado
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
- **Fog**: Neblina leve para profundidade atmosférica

### Iluminação
- **Modelo**: `GL_LIGHTING` habilitado com `GL_COLOR_MATERIAL` (difusa/ambiente pelo `glColor*`). Ambiente global via `glLightModelfv`.
- **Materiais**: Especular definido com `glMaterialfv` e `glMaterialf` (`shininess` ≈ 18), `GL_LIGHT_MODEL_TWO_SIDE` ativo. Emissão usada sutilmente no piso e nas janelas.
- **Luzes internas**:
  - `GL_LIGHT0` (teto): luz principal difusa e especular suaves.
  - `GL_LIGHT2` (altar): ponto de luz no altar, tom neutro.
  - `GL_LIGHT3` (direcional): pseudo "sol" entrando da fachada para o interior.
  - `GL_LIGHT4` (altar quente): ponto com atenuação linear (0.04) para tonalidade mais quente.
- **Luzes externas**:
  - `GL_LIGHT5` (spot fachada): `SPOT_CUTOFF` ≈ 22°, `SPOT_EXPONENT` ≈ 14, atenuação linear ≈ 0.03.
  - `GL_LIGHT6` (direcional céu): preenchimento frio e suave.
  - `GL_LIGHT7` (bounce do piso): direcional de baixo para cima, de baixa intensidade.
- **Lanterna (F)**: `GL_LIGHT1` spot acoplado à câmera com `SPOT_CUTOFF` ≈ 25°, `SPOT_EXPONENT` ≈ 12 e atenuação linear ≈ 0.06.
- **Toggle de Iluminação (L)**: Desabilita todas as luzes e reduz emissão das janelas para 5% quando desligado.
- **Transparências específicas**: Desliga `GL_LIGHTING` ao desenhar elementos translúcidos (ex.: chamas/vidros), habilita `GL_BLEND` e depois restaura a iluminação.

### Texturas 
- **Geração**: Totalmente procedurais em CPU, convertidas para texturas com mipmaps via `gluBuild2DMipmaps`.
- **Parâmetros**: `GL_TEXTURE_MIN_FILTER = GL_LINEAR_MIPMAP_LINEAR`, `GL_TEXTURE_MAG_FILTER = GL_LINEAR`, `GL_TEXTURE_WRAP_{S,T} = GL_REPEAT`.
- **Conjunto utilizado**:
  - `TEX_REFLECTIVE_TILES`: piso interno com padrão diagonal de azulejos e leve emissão simulando reflexo ambiental.
  - `TEX_PLASTER`: reboco das paredes internas (tons bege/amarelo claro).
  - `TEX_MARBLE`: mármore com veios sutis (altar, painel do crucifixo, detalhes).
  - `TEX_WOOD`: madeira com grãos naturais (porta, ambão, detalhes).
  - `TEX_STONE`: pedra com fissuras e irregularidades (elementos estruturais/degmaros/fachada).
  - `TEX_TILE`: caminho frontal externo (padrão xadrez).
  - `TEX_GRASS`: gramado externo com variações naturais.
  - `TEX_CROSS`: textura procedural específica para a cruz (madeira escura com detalhes dourados).
  - `TEX_CEILING`: teto com painéis horizontais modernos.
  - `TEX_ALTAR_WALL`: parede do altar com textura de mármore branco suave e veios sutis.
- **Aplicação**: uso de `drawTexturedQuad` com repetição de UV controlada por `uRepeat`/`vRepeat` para evitar estiramento; `glBindTexture` é habilitado/desabilitado por objeto.

### Física
- **Colisão**: Paredes, cadeiras e objetos do altar; deslizamento em quinas
- **Movimento**: Primeira pessoa com WASD, suavização de movimento com aceleração/desaceleração
- **Animação**: Porta com animação suave (interpolação linear)

### Otimizações
- **Frustum Culling**: Renderização apenas do que está visível
- **Batch Rendering**: Agrupamento de objetos similares
- **Texturas Procedurais**: Geração eficiente de texturas sem arquivos externos

## Funcionalidades Implementadas

### Estrutura e Ambiente
- ✅ Estrutura completa da igreja (paredes, teto, piso)
- ✅ Fachada em A-Frame com elementos estruturais
- ✅ Parede do altar com textura de mármore realista
- ✅ Parede externa frontal com textura de concreto/reboco
- ✅ Ambiente externo (caminho, gramado, jardim)

### Objetos e Mobiliário
- ✅ Altar completo com degraus, painel frontal e detalhes
- ✅ Ambão com símbolo eclesiástico
- ✅ Crucifixo realista na parede do fundo
- ✅ Estátuas da Virgem Maria e São José
- ✅ Cadeiras organizadas em layout
- ✅ Cruz processional dourada
- ✅ Lustre decorativo com cristais
- ✅ Ventiladores de teto

### Iluminação e Efeitos
- ✅ Sistema de iluminação completo (8 luzes)
- ✅ Luzes LED embutidas no teto (simulação visual)
- ✅ Lanterna do jogador (spotlight)
- ✅ Toggle de iluminação (tecla L)
- ✅ Janelas que escurecem quando iluminação desligada
- ✅ Fog para profundidade atmosférica

### Interatividade
- ✅ Câmera em primeira pessoa
- ✅ Sistema de colisão completo
- ✅ Porta interativa com animação
- ✅ Modo voo
- ✅ Lanterna do jogador
- ✅ Controles de movimento suavizados

### Texturas e Materiais
- ✅ 10 texturas procedurais diferentes
- ✅ Aplicação de texturas em todos os objetos principais
- ✅ Repetição de UV controlada
- ✅ Materiais com propriedades realistas (especular, emissão)

## Novidades Recentes

### Versão Atual
- **Toggle de Iluminação**: Nova tecla L para ligar/desligar toda a iluminação
- **Janelas Dinâmicas**: Janelas escurecem automaticamente quando iluminação está desligada
- **Textura na Parede do Altar**: Parede atrás da cruz com textura de mármore branco suave
- **Textura na Parede Externa**: Parede frontal externa com textura de concreto/reboco desgastado
- **Lustre**: Novo objeto decorativo acima do altar
- **Ventiladores de Teto**: Ventiladores posicionados estrategicamente
- **Luzes LED**: Simulação visual de luzes LED embutidas no teto
- **Ajustes de Tamanho**: Igreja ajustada para melhor proporção


