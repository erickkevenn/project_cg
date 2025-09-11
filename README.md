# 🏛️ Igreja - Projeto de Computação Gráfica

**Universidade Federal de Alagoas (UFAL)**  
**Disciplina: Computação Gráfica**

## 📋 Sobre o Projeto

Este projeto consiste em uma simulação 3D interativa de uma igreja desenvolvida em C++ utilizando OpenGL e GLUT. O projeto foi criado como parte das atividades da disciplina de Computação Gráfica da UFAL, demonstrando conceitos fundamentais de renderização 3D, iluminação, câmera e interação do usuário.

## 🎮 Funcionalidades

### **Ambiente 3D**
- **Igreja completa** com nave, altar, fachada e jardim
- **Iluminação realística** com múltiplas fontes de luz
- **Materiais texturizados** simulando mármore, madeira e outros materiais
- **Transparência** para vitrais e elementos decorativos

### **Objetos 3D**
- **Altar elevado** com degraus pretos
- **Crucifixo dourado** no altar
- **Cadeiras organizadas** em fileiras
- **Estátuas religiosas** decorativas
- **Flores e plantas** no jardim
- **Vitrais coloridos** nas janelas
- **Cruz processional** na entrada

### **Controles Interativos**
- **WASD**: Movimento pela igreja
- **Mouse**: Controle da câmera (olhar ao redor)
- **V**: Ativar/desativar modo voo
- **F**: Ligar/desafar lanterna
- **R**: Resetar posição da câmera
- **M**: Alternar captura do mouse
- **ESC**: Sair do programa

### **Recursos Técnicos**
- **Sistema de colisão** para navegação realística
- **Múltiplas fontes de iluminação** (ambiente, direcional, spot)
- **Materiais com propriedades** (ambiente, difusa, especular)
- **Blending e transparência** para efeitos visuais
- **Projeção perspectiva** para visualização 3D

## 🛠️ Tecnologias Utilizadas

- **C++** - Linguagem de programação
- **OpenGL** - API de renderização gráfica
- **GLUT (FreeGLUT)** - Biblioteca para criação de janelas e eventos
- **GLU** - Utilitários OpenGL

## 📁 Estrutura do Projeto

```
projetocg/
├── igreja.cpp          # Código fonte principal
└── README.md           # Este arquivo
```

## 🚀 Como Compilar e Executar

### **Pré-requisitos**
- Compilador C++ (GCC, MinGW, Visual Studio)
- Bibliotecas OpenGL e GLUT instaladas

### **Compilação**
```bash
g++ igreja.cpp -o igreja_final.exe -lfreeglut -lopengl32 -lglu32
```

### **Execução**
```bash
./igreja_final.exe
```

## 🎯 Objetivos de Aprendizagem

Este projeto demonstra os seguintes conceitos de Computação Gráfica:

1. **Modelagem 3D** - Criação de objetos geométricos primitivos
2. **Transformações** - Translação, rotação e escala de objetos
3. **Sistemas de Coordenadas** - Espaço do mundo, câmera e projeção
4. **Iluminação** - Modelos de iluminação (ambiente, difusa, especular)
5. **Materiais** - Propriedades de superfície e reflexão
6. **Câmera Virtual** - Controle de visualização e navegação
7. **Interação** - Captura de entrada do usuário (teclado/mouse)
8. **Renderização** - Pipeline gráfico e otimizações

#
