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
- **Ambão (Púlpito)**: Estrutura de madeira posicionada à frente
- **Crucifixo**: Madeira escura na parede do fundo
- **Estátuas**: Virgem Maria e São José
- **Cadeiras**: Layout organizado voltado ao altar
- **Cruz Processional**: Dourada

### 🌳 Ambiente Externo
- **Jardim**: Palmeiras laterais
- **Caminho Frontal**: Piso até a porta
- **Gramado**: Área verde ao redor

### 🚪 Interatividade
- **Porta**: Abre/fecha com a tecla E
- **Sistema de Colisão**: Colisão com paredes, cadeiras e objetos principais
- **Modo Voo**: Alterna com V (sem colisão)
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
- **GLU**: Utilitários OpenGL para operações matemáticas
- **GCC/G++**: Compilador C++

## Estrutura do Projeto

```
projetocg/
├── igreja.cpp          # Código fonte principal
├── igreja_final.exe    # Executável compilado
├── igreja_glfw.cpp     # Versão para Linux (GLFW)
├── Makefile            # Script de compilação automática
├── CMakeLists.txt      # Configuração CMake
├── run.sh              # Script de execução (Linux)
└── README.md           # Este arquivo
```

## Compilação e Execução

### Windows (GLUT)

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

### Linux (GLFW)

**Pré-requisitos:**
```bash
sudo apt-get install libglfw3-dev libgl1-mesa-dev libglu1-mesa-dev
```

**Compilação:**
```bash
g++ igreja_glfw.cpp -o igreja_glfw -lglfw -lGL -lGLU
```

**Execução:**
```bash
./igreja_glfw
```

### Compilação Automática

**Usando Makefile:**
```bash
make windows    # Para Windows
make linux      # Para Linux
make all        # Detecta automaticamente o SO
```

**Usando CMake (Linux):**
```bash
./run.sh
```

## Aspectos Técnicos

### Renderização
- **Pipeline OpenGL**: Renderização em tempo real
- **Culling**: Otimização de faces não visíveis
- **Depth Testing**: Teste de profundidade para objetos
- **Blending**: Transparência para vidros e vitrais

### Iluminação
- **Luz Ambiente**: Iluminação global suave
- **Luz Direcional**: Spotlight no altar
- **Lanterna**: Luz direcional do jogador
- **Materiais**: Propriedades de reflexão realistas

### Física
- **Colisão**: Paredes, cadeiras e objetos do altar; deslizamento em quinas
- **Movimento**: Primeira pessoa com WASD

### Otimizações
- **Frustum Culling**: Renderização apenas do que está visível
- **LOD**: Níveis de detalhe para objetos distantes
- **Batch Rendering**: Agrupamento de objetos similares

## Funcionalidades Implementadas

### ✅ Completas
- [x] Estrutura da igreja (paredes, teto, piso)
- [x] Câmera em primeira pessoa
- [x] Iluminação
- [x] Objetos principais (altar, ambão, cruz, estátuas)
- [x] Ambiente externo (caminho + gramado)
- [x] Porta interativa
- [x] Sistema de colisão (paredes, cadeiras e altar)
- [x] Modo voo
- [x] Lanterna do jogador

### 🔄 Melhorias Futuras
- [ ] Animações suaves para a porta
- [ ] Som ambiente
- [ ] Texturas mais detalhadas
- [ ] Sistema de partículas para velas
- [ ] Sombras dinâmicas
- [ ] Reflexões em superfícies

## Desenvolvimento

### Autores
- Projeto desenvolvido para a disciplina de Computação Gráfica - UFAL

### Versão
- **v1.0**: Versão inicial
- **v1.1**: Objetos principais e ambiente externo
- **v1.2**: Porta interativa
- **v1.3**: Sistema de colisão refinado (paredes, cadeiras e altar)

### Licença
Este projeto é desenvolvido para fins educacionais na UFAL.

## Comandos Úteis do Makefile

```bash
make help        # Mostra ajuda
make clean       # Limpa arquivos compilados
make windows     # Compila para Windows
make linux       # Compila para Linux
make all         # Compila automaticamente
```

## Solução de Problemas

### Erro de Compilação
- Verifique se todas as bibliotecas estão instaladas
- No Windows, certifique-se de que o FreeGLUT está no PATH
- No Linux, instale as dependências com apt-get

### Problemas de Renderização
- Verifique se o driver de vídeo suporta OpenGL
- Tente executar com diferentes configurações de resolução

### Controles Não Funcionam
- Pressione M para capturar o mouse
- Verifique se a janela está em foco
- Use R para resetar a posição

---

**Desenvolvido para a disciplina de Computação Gráfica - UFAL**
