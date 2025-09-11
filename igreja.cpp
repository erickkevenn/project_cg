// arquivo: igreja_melhorada.cpp
// g++ igreja_melhorada.cpp -o igreja.exe -lfreeglut -lopengl32 -lglu32
// Igreja moderna com fachada em V, entrada realista e objetos detalhados

#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

//================== CONFIG ==================
static const char* TITLE = "Igreja Moderna 3D — V=Voar, F=Lanterna, R=Reset, M=Mouse, E=Porta";
int   WIN_W = 1280, WIN_H = 720;

const float CH_WIDTH  = 12.0f;   // x interno -6..+6
const float CH_DEPTH  = 40.0f;   // z interno -25..+15
const float CH_HEIGHT =  6.0f;
const float DOOR_HALF =  2.0f;
const float WALL_T    =  0.2f;
const float FLOOR_Y   =  0.0f;
const float EYE_H     =  1.7f;

const float RADIUS    = 0.3f;

float camX = 0.0f, camY = EYE_H, camZ = 27.2f;
float yawDeg = 0.0f, pitchDeg = 0.0f;
bool keyDown[256]{}, spDown[256]{};
bool flashlightOn = false, mouseCaptured = true, flyingMode = false, doorOpen = false;
float baseSpeed = 4.0f; int lastMs = 0;
bool shiftHeld = false;

static inline float deg2rad(float d){ return d*3.1415926535f/180.0f; }
static inline void  clampPitch(){ pitchDeg = std::max(-89.0f, std::min(89.0f, pitchDeg)); }

void getLookVectors(float& fx,float& fy,float& fz,float& rx,float& rz){
    float yaw = deg2rad(yawDeg), pitch = deg2rad(pitchDeg);
    float cp=std::cos(pitch), sp=std::sin(pitch);
    float sy=std::sin(yaw),   cy=std::cos(yaw);
    fx =  cp*sy; fy = sp; fz = -cp*cy; rx = cy; rz = sy;
}

//================== PRIMITIVAS MELHORADAS ==================
void drawBox(float cx,float cy,float cz,float sx,float sy,float sz,
             float r,float g,float b,bool wire=false){
    glPushMatrix();
    glTranslatef(cx,cy,cz); glScalef(sx,sy,sz); glColor3f(r,g,b);
    if (wire) glutWireCube(1.0); else glutSolidCube(1.0);
    glPopMatrix();
}

void drawCylinder(float x, float y, float z, float radius, float height, int slices, float r, float g, float b){
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor3f(r, g, b);
    GLUquadric* quad = gluNewQuadric();
    gluCylinder(quad, radius, radius, height, slices, 1);
    gluDeleteQuadric(quad);
    glPopMatrix();
}

void drawSphere(float x, float y, float z, float radius, int slices, int stacks, float r, float g, float b){
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor3f(r, g, b);
    glutSolidSphere(radius, slices, stacks);
    glPopMatrix();
}

void drawCross(float x,float y,float z,float s=1.0f,float r=0.95f,float g=0.95f,float b=0.97f){
    float th=0.10f*s;
    drawBox(x,y,z, th,2.0f*s,th, r,g,b);
    drawBox(x,y+0.4f*s,z, 1.2f*s,th,th, r,g,b);
}
void drawPortalAFrame(float zCenter, bool addCross, bool groundLevel = false){
    const float fr=0.82f, fg=0.84f, fb=0.86f; // cinza claro
    // montantes inclinados
    glPushMatrix(); glTranslatef(-3.9f, 3.6f, zCenter); glRotatef(-17,0,0,1);
    drawBox(0,0,0, 0.45f, 5.2f, 0.50f, fr,fg,fb); glPopMatrix();
    glPushMatrix(); glTranslatef( 3.9f, 3.6f, zCenter); glRotatef( 17,0,0,1);
    drawBox(0,0,0, 0.45f, 5.2f, 0.50f, fr,fg,fb); glPopMatrix();
    // travessa superior (bem alta)
    drawBox(0.0f, 5.2f, zCenter, 6.6f, 0.45f, 0.50f, fr,fg,fb);

    if (addCross){
        // placa/janela alta opcional
        drawBox(0.0f, 4.6f, zCenter+0.2f, 1.6f, 1.0f, 0.06f, 0.94f,0.95f,0.97f);
        // cruz discreta no topo
        drawCross(0.0f, 6.2f, zCenter+0.1f, 0.9f, 0.95f,0.95f,0.97f);
    }
    
    if (groundLevel){
        // Base adicional para o arco de trás
        drawBox(0.0f, 0.1f, zCenter, 6.6f, 0.2f, 0.50f, fr*0.9f, fg*0.9f, fb*0.9f);
        
    }
}

//================== FACHADA EM A-FRAME (V INVERTIDO) ==================
void drawAFrameFacade(){
    // Base de pedra com RECORTE central
    const float baseW = 14.0f, baseH = 2.2f, baseZ = 16.8f;
    const float gapW  = 4.8f; // vão livre
    const float sideW = (baseW - gapW) * 0.5f;
    const float stoneR=0.62f, stoneG=0.58f, stoneB=0.54f;

    drawBox(-(gapW*0.5f + sideW*0.5f), baseH*0.5f, baseZ, sideW, baseH, 0.30f, stoneR,stoneG,stoneB);
    drawBox( (gapW*0.5f + sideW*0.5f), baseH*0.5f, baseZ, sideW, baseH, 0.30f, stoneR,stoneG,stoneB);
    drawBox(0.0f, 0.12f, baseZ, gapW, 0.24f, 0.28f, stoneR*0.9f, stoneG*0.9f, stoneB*0.9f);

    // >>> Dois arcos um atrás do outro, com espaço e sem bloquear a porta:
    const float zFront = 16.8f; // arco da frente
    const float zBack  = 16.0f; // arco de trás, um pouco mais próximo da parede

    drawPortalAFrame(zFront, true,  false); // frente: com cruz/placa, não encosta
    drawPortalAFrame(zBack,  false, true ); // trás: sem cruz/placa, encosta no chão
}



//================== ENTRADA ESTILO FOTO ==================
void drawPhotoStyleEntrance(){
    // Moldura recuada que NÃO fecha o vão
    const float doorW = DOOR_HALF*2.0f;   // 4.0 m
    const float doorH = 3.0f;
    const float frameW= 0.35f;            // largura das ombreiras / verga
    const float zFace = 14.90f;           // um pouco para dentro da parede (z=15)
    const float fr=0.72f, fg=0.74f, fb=0.77f;

    // ombreiras
    drawBox(-(doorW*0.5f + frameW*0.5f), doorH*0.5f, zFace, frameW, doorH, 0.20f, fr,fg,fb);
    drawBox( (doorW*0.5f + frameW*0.5f), doorH*0.5f, zFace, frameW, doorH, 0.20f, fr,fg,fb);
    // verga
    drawBox(0.0f, doorH + frameW*0.5f, zFace, doorW + frameW*2.0f, frameW, 0.20f, fr,fg,fb);

    // >>> IMPORTANTE: REMOVIDO o bloco sólido que você tinha:
    // // drawBox(0.0f, 1.5f, 15.0f, 6.0f, 3.0f, 0.2f, ...);  // ISSO TAMPAVA A PORTA

    // iluminação do teto da entrada
    drawBox(0.0f, 3.1f, 15.0f, 0.8f, 0.25f, 0.25f, 0.90f,0.90f,0.92f);

    // tapete/capacho (opcional)
    drawBox(0.0f, FLOOR_Y+0.01f, 14.6f, 3.8f, 0.02f, 1.2f, 0.75f,0.10f,0.10f);
    drawBox(0.0f, FLOOR_Y+0.02f, 14.9f, 1.9f, 0.02f, 0.7f, 0.55f,0.40f,0.22f);
}
//================== JARDIM EXTERNO ==================
void drawGarden(){
    // Apenas palmeiras altas ao redor da igreja (sem plantações)
    float trunkR=0.6f, trunkG=0.4f, trunkB=0.2f;
    float leavesR=0.2f, leavesG=0.6f, leavesB=0.2f;
    
    // Palmeiras nas laterais
    drawBox(-8.0f, FLOOR_Y+3.0f, 10.0f, 0.3f, 6.0f, 0.3f, trunkR, trunkG, trunkB);
    drawBox(8.0f, FLOOR_Y+3.0f, 10.0f, 0.3f, 6.0f, 0.3f, trunkR, trunkG, trunkB);
    drawBox(-10.0f, FLOOR_Y+3.0f, 5.0f, 0.3f, 6.0f, 0.3f, trunkR, trunkG, trunkB);
    drawBox(10.0f, FLOOR_Y+3.0f, 5.0f, 0.3f, 6.0f, 0.3f, trunkR, trunkG, trunkB);
    
    // Folhas das palmeiras
    drawSphere(-8.0f, FLOOR_Y+6.5f, 10.0f, 1.0f, 12, 12, leavesR, leavesG, leavesB);
    drawSphere(8.0f, FLOOR_Y+6.5f, 10.0f, 1.0f, 12, 12, leavesR, leavesG, leavesB);
    drawSphere(-10.0f, FLOOR_Y+6.5f, 5.0f, 1.0f, 12, 12, leavesR, leavesG, leavesB);
    drawSphere(10.0f, FLOOR_Y+6.5f, 5.0f, 1.0f, 12, 12, leavesR, leavesG, leavesB);
}

//================== VITRAIS MELHORADOS ==================
void drawStainedGlassXY(float w=1.4f,float h=2.6f,float archH=0.7f){
    float frameT=0.05f, gap=0.03f, bodyH=h-archH;
    
    // Moldura mais detalhada
    glColor3f(0.85f,0.85f,0.87f);
    glBegin(GL_QUADS);
    glVertex3f(-w*0.5f,0,0.0f); glVertex3f(w*0.5f,0,0.0f);
    glVertex3f(w*0.5f,bodyH,0.0f); glVertex3f(-w*0.5f,bodyH,0.0f);
    glEnd();

    int seg=28;
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0;i<=seg;i++){
        float t=i/(float)seg, ang=3.14159265f*(1.0f-t);
        float x=(w*0.5f)*std::cos(ang), y=bodyH+archH*std::sin(ang);
        glVertex3f(x,y,0.0f); glVertex3f(x*0.92f,y-0.02f,0.0f);
    }
    glEnd();

    auto pane=[&](float x0,float y0,float x1,float y1,float r,float g,float b,float a){
        glColor4f(r,g,b,a);
        glBegin(GL_QUADS);
        glVertex3f(x0,y0,0.01f); glVertex3f(x1,y0,0.01f);
        glVertex3f(x1,y1,0.01f); glVertex3f(x0,y1,0.01f);
        glEnd();
    };

    float xL=-w*0.5f+frameT, xR=w*0.5f-frameT, yB=frameT, yT=bodyH-frameT, xM=0, yM=bodyH*0.5f;
    
    // Cores mais vibrantes e realistas
    pane(xL,yM+gap,xM-gap,yT, 0.15f,0.65f,0.15f,0.75f);  // Verde mais suave
    pane(xM+gap,yM+gap,xR,yT, 0.95f,0.45f,0.05f,0.75f);  // Laranja dourado
    pane(xL,yB,xM-gap,yM-gap, 0.95f,0.60f,0.15f,0.75f);  // Amarelo dourado
    pane(xM+gap,yB,xR,yM-gap, 0.20f,0.75f,0.20f,0.75f);  // Verde esmeralda

    // Arco azul mais profundo
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.15f,0.45f,0.85f,0.75f);
    glVertex3f(0, bodyH+archH*0.6f, 0.01f);
    for(int i=0;i<=seg;i++){
        float t=i/(float)seg, ang=3.14159265f*(1.0f-t);
        float x=(w*0.5f-0.05f)*std::cos(ang), y=bodyH+(archH-0.05f)*std::sin(ang);
        glVertex3f(x,y,0.01f);
    }
    glEnd();

    // Travessas mais finas e elegantes
    glLineWidth(2.0f); glColor4f(0.92f,0.92f,0.94f,0.95f);
    glBegin(GL_LINES);
    glVertex3f(0,0.05f,0.02f); glVertex3f(0,bodyH+archH-0.05f,0.02f);
    glVertex3f(-w*0.5f+0.06f,yM,0.02f); glVertex3f(w*0.5f-0.06f,yM,0.02f);
    glEnd();
}

inline void placeStainedOnSide(float xSide,float y,float z){
    glPushMatrix();
    glTranslatef(xSide, y, z);
    if (xSide>0) glRotatef(90,0,1,0); else glRotatef(-90,0,1,0);
    drawStainedGlassXY();
    glPopMatrix();
}

//================== OBJETOS REALISTAS ==================
void drawRealisticAltar(){
    // Degrau de escada preto (base elevada) - mais largo
    drawBox(0.0f, FLOOR_Y+0.15f, -22.5f, 5.0f, 0.3f, 2.0f, 0.1f, 0.1f, 0.1f);
    
    // Segundo degrau menor - mais largo
    drawBox(0.0f, FLOOR_Y+0.35f, -22.3f, 4.5f, 0.2f, 1.6f, 0.15f, 0.15f, 0.15f);
    
    // Base do altar (mármore branco com veios cinzentos) - mais largo
    float marbleR=0.95f, marbleG=0.95f, marbleB=0.97f;
    drawBox(0.0f, FLOOR_Y+0.75f, -22.5f, 3.8f, 1.1f, 1.4f, marbleR, marbleG, marbleB);
    
    // Painel frontal (mármore preto com Agnus Dei dourado) - mais largo
    drawBox(0.0f, FLOOR_Y+0.75f, -21.9f, 3.0f, 1.0f, 0.05f, 0.12f, 0.12f, 0.15f);
    
    // Detalhe dourado do Agnus Dei (Cordeiro de Deus) - mais largo
    drawBox(0.0f, FLOOR_Y+1.0f, -21.85f, 1.2f, 0.4f, 0.02f, 0.9f, 0.7f, 0.2f);
    
    // Toalha de altar (branca com renda nas bordas) - mais larga
    drawBox(0.0f, FLOOR_Y+1.3f, -22.5f, 4.0f, 0.05f, 1.5f, 1.0f, 1.0f, 1.0f);
    
    // Bíblia aberta (capa avermelhada/marrom) - mais larga
    drawBox(-0.8f, FLOOR_Y+1.35f, -22.3f, 0.5f, 0.03f, 0.3f, 0.7f, 0.4f, 0.3f);
    drawBox(-0.8f, FLOOR_Y+1.38f, -22.3f, 0.48f, 0.01f, 0.28f, 0.95f, 0.95f, 0.98f);
}

void drawRealisticCrucifix(){
    // Parede de mármore cinza claro com veios escuros
    float marbleR=0.88f, marbleG=0.88f, marbleB=0.90f;
    drawBox(0.0f, 3.0f, -24.9f, 4.0f, 6.0f, 0.1f, marbleR, marbleG, marbleB);
    
    // Cruz (madeira escura marrom)
    float woodR=0.45f, woodG=0.30f, woodB=0.20f;
    drawBox(0.0f, 3.8f, -24.8f, 0.2f, 3.5f, 0.08f, woodR, woodG, woodB);
    drawBox(0.0f, 4.5f, -24.8f, 1.8f, 0.2f, 0.08f, woodR, woodG, woodB);
    
    // Cristo (corpo realista com tons de pele)
    float skinR=0.95f, skinG=0.85f, skinB=0.75f;
    drawBox(0.0f, 4.2f, -24.75f, 0.15f, 0.8f, 0.05f, skinR, skinG, skinB);
    drawBox(0.0f, 4.6f, -24.75f, 0.12f, 0.12f, 0.05f, skinR, skinG, skinB);
    
    // Braços
    drawBox(-0.3f, 4.4f, -24.75f, 0.6f, 0.08f, 0.05f, skinR, skinG, skinB);
    
    // Pano branco cobrindo a cintura
    drawBox(0.0f, 3.9f, -24.7f, 0.25f, 0.3f, 0.03f, 1.0f, 1.0f, 1.0f);
}

void drawRealisticStatues(){
    // Estátua da Virgem Maria (esquerda) - em prateleira branca
    float blueR=0.2f, blueG=0.4f, blueB=0.8f;
    float whiteR=0.95f, whiteG=0.95f, whiteB=0.98f;
    float skinR=0.95f, skinG=0.85f, skinB=0.75f;
    
    // Prateleira branca fixada na parede
    //drawBox(-4.5f, FLOOR_Y+1.2f, -20.0f, 0.8f, 0.1f, 0.4f, 1.0f, 1.0f, 1.0f);
    
    // Corpo (vestido azul)
    drawBox(-4.5f, FLOOR_Y+1.0f, -20.0f, 0.3f, 1.2f, 0.2f, blueR, blueG, blueB);
    
    // Cabeça
    drawSphere(-4.5f, FLOOR_Y+1.8f, -20.0f, 0.12f, 12, 12, skinR, skinG, skinB);
    
    // Manto branco
    drawBox(-4.5f, FLOOR_Y+1.3f, -20.0f, 0.4f, 0.8f, 0.15f, whiteR, whiteG, whiteB);
    
    // Estátua de São José (direita) - em prateleira branca
    float brownR=0.6f, brownG=0.4f, brownB=0.2f;
    
    // Prateleira branca fixada na parede
    //drawBox(4.5f, FLOOR_Y+1.2f, -20.0f, 0.8f, 0.1f, 0.4f, 1.0f, 1.0f, 1.0f);
    
    // Corpo (túnica marrom)
    drawBox(4.5f, FLOOR_Y+1.0f, -20.0f, 0.3f, 1.2f, 0.2f, brownR, brownG, brownB);
    
    // Cabeça
    drawSphere(4.5f, FLOOR_Y+1.8f, -20.0f, 0.12f, 12, 12, skinR, skinG, skinB);
    
    // Cajado
    drawBox(4.7f, FLOOR_Y+1.2f, -20.0f, 0.03f, 1.0f, 0.03f, 0.4f, 0.3f, 0.2f);
}


void drawProcessionalCross(){
    float goldR=0.9f, goldG=0.7f, goldB=0.2f;
    
    // Base dourada
    drawBox(2.5f, FLOOR_Y+0.1f, -21.0f, 0.3f, 0.2f, 0.3f, goldR, goldG, goldB);
    
    // Haste principal dourada
    drawBox(2.5f, FLOOR_Y+2.0f, -21.0f, 0.05f, 4.0f, 0.05f, goldR, goldG, goldB);
    
    // Cruz no topo dourada
    drawBox(2.5f, FLOOR_Y+4.2f, -21.0f, 0.05f, 0.4f, 0.05f, goldR, goldG, goldB);
    drawBox(2.5f, FLOOR_Y+4.0f, -21.0f, 0.3f, 0.05f, 0.05f, goldR, goldG, goldB);
    
    // Crucifixo pequeno dourado
    drawBox(2.5f, FLOOR_Y+4.2f, -20.95f, 0.02f, 0.15f, 0.02f, goldR, goldG, goldB);
    drawBox(2.5f, FLOOR_Y+4.15f, -20.95f, 0.1f, 0.02f, 0.02f, goldR, goldG, goldB);
}

// --- Símbolo do ambão (placa de mármore + inlay preto) ---
void drawAmbaoSymbol(float xCenter, float yCenter, float zFrontFace){
    const float M_R=0.94f, M_G=0.94f, M_B=0.96f;      // mármore branco
    const float K    =0.06f;                          // preto
    const float zSym = zFrontFace + 0.01f;            // pequeno avanço p/ evitar z-fighting

    // Placa de mármore (fundo)
    drawBox(xCenter, yCenter, zFrontFace, 0.42f, 0.65f, 0.02f, M_R,M_G,M_B);

    // Cruz
    drawBox(xCenter, yCenter+0.18f, zSym, 0.03f, 0.42f, 0.02f, K,K,K);   // eixo vertical
    drawBox(xCenter, yCenter+0.30f, zSym, 0.16f, 0.03f, 0.02f, K,K,K);   // travessa

    // Base inferior
    drawBox(xCenter, yCenter-0.33f, zSym, 0.26f, 0.03f, 0.02f, K,K,K);

    // Cálice (taça + haste)
    drawBox(xCenter, yCenter-0.10f, zSym, 0.15f, 0.10f, 0.02f, K,K,K);   // “taça”
    drawBox(xCenter, yCenter-0.22f, zSym, 0.03f, 0.12f, 0.02f, K,K,K);   // haste

    // “Chamas” laterais (aproximação com retângulos inclinados)
    glPushMatrix(); glTranslatef(xCenter-0.12f, yCenter-0.08f, zSym); glRotatef(20,0,0,1);
    drawBox(0,0,0, 0.05f, 0.12f, 0.02f, K,K,K); glPopMatrix();
    glPushMatrix(); glTranslatef(xCenter+0.12f, yCenter-0.08f, zSym); glRotatef(-20,0,0,1);
    drawBox(0,0,0, 0.05f, 0.12f, 0.02f, K,K,K); glPopMatrix();

    // Círculo à direita (esfera achatada)
    drawSphere(xCenter+0.19f, yCenter-0.04f, zSym+0.01f, 0.05f, 14,14, K,K,K);
}


void drawAmbao(){
    // Ambão (púlpito) - estrutura de madeira escura, menor e virado para as cadeiras
    float woodR=0.4f, woodG=0.25f, woodB=0.15f;
    float marbleR=0.95f, marbleG=0.95f, marbleB=0.97f;
    
    // Posição do ambão: mais para o lado e virado para as cadeiras
    float ambaoX = -3.0f;  // Movido mais para o lado
    float ambaoZ = -18.0f; // Movido mais para frente (em direção às cadeiras)
    
    // Base do ambão (degrau) - menor
    drawBox(ambaoX, FLOOR_Y+0.15f, ambaoZ, 0.8f, 0.3f, 0.6f, woodR, woodG, woodB);
    
    // Estrutura principal do ambão - menor
    drawBox(ambaoX, FLOOR_Y+0.7f, ambaoZ, 0.7f, 1.4f, 0.5f, woodR, woodG, woodB);
    
    // Mesa do ambão (mármore branco) - menor
    drawBox(ambaoX, FLOOR_Y+1.4f, ambaoZ, 0.9f, 0.1f, 0.6f, marbleR, marbleG, marbleB);
    
    // Apoio para os pés (pequeno degrau) - menor
    drawBox(ambaoX, FLOOR_Y+0.05f, ambaoZ+0.2f, 0.6f, 0.1f, 0.25f, woodR*0.8f, woodG*0.8f, woodB*0.8f);
        // frente do ambão = centroZ + meia profundidade (0.5f)
        float zFront = ambaoZ + 0.5f;
    
        // altura central da placa no corpo do ambão
        drawAmbaoSymbol(ambaoX, FLOOR_Y + 0.95f, zFront);
    
}



void drawDoor(){
    // Duas portas que abrem e fecham com a tecla E
    float doorR=0.6f, doorG=0.4f, doorB=0.2f;  // Madeira escura
    float handleR=0.9f, handleG=0.7f, handleB=0.2f;  // Dourado
    
    if (doorOpen) {
        // Portas abertas (rotacionadas 90 graus)
        // Porta esquerda aberta
        glPushMatrix();
        glTranslatef(-1.0f, 0.0f, 0.0f);
        glRotatef(90, 0, 1, 0);
        glTranslatef(1.0f, 0.0f, 0.0f);
        drawBox(-1.0f, 1.5f, 15.1f, 0.1f, 3.0f, 2.0f, doorR, doorG, doorB);
        glPopMatrix();
        
        // Porta direita aberta
        glPushMatrix();
        glTranslatef(1.0f, 0.0f, 0.0f);
        glRotatef(-90, 0, 1, 0);
        glTranslatef(-1.0f, 0.0f, 0.0f);
        drawBox(1.0f, 1.5f, 15.1f, 0.1f, 3.0f, 2.0f, doorR, doorG, doorB);
        glPopMatrix();
        
        // Maçanetas das portas abertas
        drawBox(-1.8f, 1.5f, 15.15f, 0.05f, 0.1f, 0.05f, handleR, handleG, handleB);
        drawBox(1.8f, 1.5f, 15.15f, 0.05f, 0.1f, 0.05f, handleR, handleG, handleB);
    } else {
        // Portas fechadas
        // Porta esquerda
        drawBox(-1.0f, 1.5f, 15.1f, 2.0f, 3.0f, 0.1f, doorR, doorG, doorB);
        
        // Porta direita
        drawBox(1.0f, 1.5f, 15.1f, 2.0f, 3.0f, 0.1f, doorR, doorG, doorB);
        
        // Maçanetas das portas fechadas
        drawBox(-0.2f, 1.5f, 15.15f, 0.05f, 0.1f, 0.05f, handleR, handleG, handleB);
        drawBox(0.2f, 1.5f, 15.15f, 0.05f, 0.1f, 0.05f, handleR, handleG, handleB);
        
        // Detalhes das portas (molduras)
        // Moldura da porta esquerda
        drawBox(-1.0f, 1.5f, 15.12f, 1.8f, 0.1f, 0.05f, doorR*0.8f, doorG*0.8f, doorB*0.8f);
        drawBox(-1.0f, 1.5f, 15.12f, 0.1f, 2.8f, 0.05f, doorR*0.8f, doorG*0.8f, doorB*0.8f);
        drawBox(-1.0f, 2.5f, 15.12f, 1.8f, 0.1f, 0.05f, doorR*0.8f, doorG*0.8f, doorB*0.8f);
        drawBox(-1.0f, 0.5f, 15.12f, 1.8f, 0.1f, 0.05f, doorR*0.8f, doorG*0.8f, doorB*0.8f);
        
        // Moldura da porta direita
        drawBox(1.0f, 1.5f, 15.12f, 1.8f, 0.1f, 0.05f, doorR*0.8f, doorG*0.8f, doorB*0.8f);
        drawBox(1.0f, 1.5f, 15.12f, 0.1f, 2.8f, 0.05f, doorR*0.8f, doorG*0.8f, doorB*0.8f);
        drawBox(1.0f, 2.5f, 15.12f, 1.8f, 0.1f, 0.05f, doorR*0.8f, doorG*0.8f, doorB*0.8f);
        drawBox(1.0f, 0.5f, 15.12f, 1.8f, 0.1f, 0.05f, doorR*0.8f, doorG*0.8f, doorB*0.8f);
    }
}


//================== CADEIRAS MELHORADAS ==================
void drawPlasticChairWhite(){
    // tons off-white para não estourar na luz
    const float body = 0.93f;   // assento/encosto
    const float frame= 0.88f;   // “tubos”/pernas

    // assento
    drawBox(0.0f, 0.44f, 0.0f,  0.48f, 0.05f, 0.46f,  body, body, body);

    // encosto (duas peças para dar altura)
    drawBox(0.0f, 0.78f, -0.20f, 0.48f, 0.52f, 0.06f, body, body, body);
    drawBox(0.0f, 1.02f, -0.20f, 0.44f, 0.12f, 0.06f, body, body, body);

    // pernas (PVC)
    const float legT = 0.06f, legH = 0.44f;
    drawBox(-0.20f, legH*0.5f, -0.20f, legT, legH, legT, frame, frame, frame);
    drawBox( 0.20f, legH*0.5f, -0.20f, legT, legH, legT, frame, frame, frame);
    drawBox(-0.20f, legH*0.5f,  0.18f, legT, legH, legT, frame, frame, frame);
    drawBox( 0.20f, legH*0.5f,  0.18f, legT, legH, legT, frame, frame, frame);

    // travessas para dar rigidez
    drawBox(0.0f, 0.18f, -0.20f, 0.42f, 0.04f, 0.04f, frame, frame, frame);
    drawBox(0.0f, 0.18f,  0.18f, 0.42f, 0.04f, 0.04f, frame, frame, frame);
    drawBox(-0.20f,0.18f, 0.0f,  0.04f, 0.04f, 0.36f, frame, frame, frame);
    drawBox( 0.20f,0.18f, 0.0f,  0.04f, 0.04f, 0.36f, frame, frame, frame);
}
void drawChairsLayout(){
    // Cadeiras orientadas para o altar (cruz) - fileiras mais organizadas
    for (float z=8.0f; z>=-16.0f; z-=2.5f){
        for(int c=0;c<4;++c){
            float off=1.0f*c;
            // Lado direito - cadeiras viradas para o altar
            glPushMatrix(); 
            glTranslatef(2.5f+off,0,z); 
            glRotatef(180, 0, 1, 0);  // Rotaciona 180° para ficar virada para o altar
            drawPlasticChairWhite(); 
    glPopMatrix();
            
            // Lado esquerdo - cadeiras viradas para o altar
            glPushMatrix(); 
            glTranslatef(-2.5f-off,0,z); 
            glRotatef(180, 0, 1, 0);  // Rotaciona 180° para ficar virada para o altar
            drawPlasticChairWhite(); 
    glPopMatrix();
        }
    }
}

void drawCeilingFan(float x, float y, float z, bool isLeftSide = true){
    // Ventilador simples grudado na parede, apontado para o meio
    float fanR = 0.2f, fanG = 0.2f, fanB = 0.2f;  // Cor escura
    
    glPushMatrix();
    glTranslatef(x, y, z);
    
    // Rotacionar para apontar para o meio da igreja
    if (isLeftSide) {
        glRotatef(90, 0, 1, 0);  // Ventilador esquerdo aponta para a direita (meio)
    } else {
        glRotatef(-90, 0, 1, 0); // Ventilador direito aponta para a esquerda (meio)
    }
    
    // Base do ventilador (grudada na parede)
    drawBox(0.0f, 0.0f, 0.0f, 0.3f, 0.2f, 0.1f, fanR, fanG, fanB);
    
    // Motor do ventilador
    drawBox(0.0f, 0.0f, 0.05f, 0.2f, 0.15f, 0.1f, fanR*0.8f, fanG*0.8f, fanB*0.8f);
    
    // Pás do ventilador (3 pás)
    // Pá 1 (horizontal)
    drawBox(0.0f, 0.0f, 0.1f, 0.8f, 0.05f, 0.02f, fanR*0.9f, fanG*0.9f, fanB*0.9f);
    
    // Pá 2 (diagonal)
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.1f);
    glRotatef(120, 0, 0, 1);
    drawBox(0.0f, 0.0f, 0.0f, 0.8f, 0.05f, 0.02f, fanR*0.9f, fanG*0.9f, fanB*0.9f);
    glPopMatrix();
    
    // Pá 3 (diagonal)
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.1f);
    glRotatef(240, 0, 0, 1);
    drawBox(0.0f, 0.0f, 0.0f, 0.8f, 0.05f, 0.02f, fanR*0.9f, fanG*0.9f, fanB*0.9f);
    glPopMatrix();
    
    glPopMatrix();
}

void drawFrontPath(){
    // medidas principais (ajuste se quiser)
    const float pathW = 4.8f;   // LARGURA do caminho (≈ largura do vão)
    const float pathL = 22.0f;  // COMPRIMENTO do caminho para "fora"
    const float z0    = 16.2f;  // começa logo depois da porta/mureta (parede está em z=15)

    // base cinza (um tico acima do gramado para não dar z-fighting)
    drawBox(0.0f, FLOOR_Y-0.005f, z0 + pathL*0.5f,
            pathW, 0.02f, pathL, 0.72f, 0.72f, 0.74f);

    // bordas laterais um pouco mais escuras (marcam onde você desenhou as linhas vermelhas)
    drawBox(-pathW*0.5f + 0.15f, FLOOR_Y-0.003f, z0 + pathL*0.5f,
            0.30f, 0.015f, pathL, 0.60f, 0.60f, 0.62f);
    drawBox( pathW*0.5f - 0.15f, FLOOR_Y-0.003f, z0 + pathL*0.5f,
            0.30f, 0.015f, pathL, 0.60f, 0.60f, 0.62f);

    // lajotas para dar textura (opcional)
    const float tileW = 0.80f, tileL = 0.45f, gap = 0.10f;
    for (float z = z0 + 0.35f; z < z0 + pathL - 0.35f; z += tileL + gap){
        for (float x = -pathW*0.5f + 0.40f; x <= pathW*0.5f - 0.40f; x += tileW + gap){
            drawBox(x, FLOOR_Y-0.001f, z, tileW, 0.01f, tileL, 0.80f, 0.80f, 0.83f);
        }
    }
}

//================== IGREJA PRINCIPAL ==================
void drawChurchOpaque(){
    // Cores mais realistas
    float wallR=0.80f, wallG=0.74f, wallB=0.68f;  // bege quente das paredes
    float ceilR=0.92f, ceilG=0.92f, ceilB=0.94f;  // teto off-white
    float floorR=0.78f, floorG=0.77f, floorB=0.75f; // piso claro sem branco puro

    // Piso, teto
    drawBox(0.0f, FLOOR_Y-0.05f, -5.0f, CH_WIDTH, 0.1f, CH_DEPTH, floorR,floorG,floorB);
    drawBox(0.0f, CH_HEIGHT, -5.0f,   CH_WIDTH, 0.1f, CH_DEPTH,  ceilR,ceilG,ceilB);

    // Paredes
    drawBox(-CH_WIDTH*0.5f, CH_HEIGHT*0.5f, -5.0f, WALL_T, CH_HEIGHT, CH_DEPTH, wallR,wallG,wallB);
    drawBox( CH_WIDTH*0.5f, CH_HEIGHT*0.5f, -5.0f, WALL_T, CH_HEIGHT, CH_DEPTH, wallR,wallG,wallB);
    drawBox(0.0f, CH_HEIGHT*0.5f, -25.0f,   CH_WIDTH, CH_HEIGHT, WALL_T, wallR,wallG,wallB);

    // Frente (porta) - CORRIGIDO: entrada no nível do chão
    drawBox(-(CH_WIDTH*0.5f+DOOR_HALF)*0.5f, CH_HEIGHT*0.5f, 15.0f,
            (CH_WIDTH*0.5f-DOOR_HALF), CH_HEIGHT, WALL_T, wallR,wallG,wallB);
    drawBox( (CH_WIDTH*0.5f+DOOR_HALF)*0.5f, CH_HEIGHT*0.5f, 15.0f,
            (CH_WIDTH*0.5f-DOOR_HALF), CH_HEIGHT, WALL_T, wallR,wallG,wallB);
    drawBox(0.0f, (3.0f+CH_HEIGHT)*0.5f, 15.0f, DOOR_HALF*2.0f, (CH_HEIGHT-3.0f), WALL_T, wallR,wallG,wallB);

    // Objetos realistas
    drawChairsLayout();
    drawRealisticCrucifix();
    drawRealisticAltar();
    drawAmbao();
    drawRealisticStatues();


    drawProcessionalCross();
    
    // Ventiladores nas paredes laterais (nos pontos vermelhos)
    drawCeilingFan(-5.8f, 4.5f, 0.0f, true);   // Ventilador esquerdo (aponta para direita/meio)
    drawCeilingFan(5.8f, 4.5f, 0.0f, false);   // Ventilador direito (aponta para esquerda/meio)

    // Fachada e entrada estilo foto
    drawAFrameFacade();
    drawPhotoStyleEntrance();
    drawDoor();

    //frente cinza caminho
    drawFrontPath();  
    // Jardim externo
    drawGarden();

    // Exterior (gramado)
    drawBox(0.0f, -0.06f, 30.0f, 120.0f, 0.1f, 120.0f, 0.70f,0.88f,0.72f);
    
    
    
    
    
    // Muro removido - apenas gramado ao redor
    
    // Postes de luz no estacionamento
   // drawBox(-20.0f, FLOOR_Y+3.0f, 30.0f, 0.1f, 6.0f, 0.1f, 0.8f, 0.8f, 0.8f);
   // drawBox(20.0f, FLOOR_Y+3.0f, 30.0f, 0.1f, 6.0f, 0.1f, 0.8f, 0.8f, 0.8f);
    //drawSphere(-20.0f, FLOOR_Y+5.5f, 30.0f, 0.2f, 12, 12, 1.0f, 1.0f, 0.9f);
    //drawSphere(20.0f, FLOOR_Y+5.5f, 30.0f, 0.2f, 12, 12, 1.0f, 1.0f, 0.9f);
}

// Helper: desenha porta dupla no ponto (cx,cz) rotacionada 'yawDeg' em Y
static void drawDoorAt(float cx, float cz, float yawDeg){
	float doorR=0.6f, doorG=0.4f, doorB=0.2f;
	float handleR=0.9f, handleG=0.7f, handleB=0.2f;

	glPushMatrix();
	glTranslatef(cx, 0.0f, cz);
	glRotatef(yawDeg, 0,1,0);

	if (doorOpen) {
		// esquerda aberta
		glPushMatrix();
		glTranslatef(-1.0f, 0.0f, 0.0f);
		glRotatef(90, 0, 1, 0);
		glTranslatef(1.0f, 0.0f, 0.0f);
		drawBox(-1.0f, 1.5f, 0.1f, 0.1f, 3.0f, 2.0f, doorR, doorG, doorB);
		glPopMatrix();

		// direita aberta
		glPushMatrix();
		glTranslatef(1.0f, 0.0f, 0.0f);
		glRotatef(-90, 0, 1, 0);
		glTranslatef(-1.0f, 0.0f, 0.0f);
		drawBox(1.0f, 1.5f, 0.1f, 0.1f, 3.0f, 2.0f, doorR, doorG, doorB);
		glPopMatrix();

		// maçanetas
		drawBox(-1.8f, 1.5f, 0.15f, 0.05f, 0.1f, 0.05f, handleR, handleG, handleB);
		drawBox( 1.8f, 1.5f, 0.15f, 0.05f, 0.1f, 0.05f, handleR, handleG, handleB);
	} else {
		// duas folhas fechadas
		drawBox(-1.0f, 1.5f, 0.1f, 2.0f, 3.0f, 0.1f, doorR, doorG, doorB);
		drawBox( 1.0f, 1.5f, 0.1f, 2.0f, 3.0f, 0.1f, doorR, doorG, doorB);

		// maçanetas
		drawBox(-0.2f, 1.5f, 0.15f, 0.05f, 0.1f, 0.05f, handleR, handleG, handleB);
		drawBox( 0.2f, 1.5f, 0.15f, 0.05f, 0.1f, 0.05f, handleR, handleG, handleB);
	}
	glPopMatrix();
}

void drawChurchWindows(){
    // desenha os vitrais nas duas faces da parede (interno e externo)
    const float inset = 0.18f;      // distância da janela em relação à superfície da parede
    const float xL_in  = -CH_WIDTH*0.5f + inset;   // esquerda (dentro)
    const float xL_out = -CH_WIDTH*0.5f - inset;   // esquerda (fora)
    const float xR_in  =  CH_WIDTH*0.5f - inset;   // direita (dentro)
    const float xR_out =  CH_WIDTH*0.5f + inset;   // direita (fora)

    auto put = [&](float x, float y, float z){
        glPushMatrix();
        glTranslatef(x, y, z);
        // mesma orientação usada antes (−90° na esquerda, +90° na direita)
        if (x > 0) glRotatef( 90, 0,1,0);
        else       glRotatef(-90, 0,1,0);
        drawStainedGlassXY();   // vitral colorido
        glPopMatrix();
    };

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); // não grava no depth para não “cortar” transparências

    // Mesmas posições de janelas que você já usa
    const float Y = 2.0f;
    put(xL_in , Y,  6.0f);  put(xL_out, Y,  6.0f);
    put(xL_in , Y, -8.0f);  put(xL_out, Y, -8.0f);
    put(xR_in , Y,  6.0f);  put(xR_out, Y,  6.0f);
    put(xR_in , Y, -8.0f);  put(xR_out, Y, -8.0f);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

//================== LUZ/CÂMERA ==================
void setupLights(){
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Ambiente baixo (evita “lavado”)
    GLfloat globalAmb[4] = {0.14f,0.14f,0.14f,1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);

    // Luz principal: difusa moderada, sem especular
    GLfloat pos0[4]  = { 0.0f, 7.0f, 0.0f, 1.0f };
    GLfloat dif0[4]  = { 0.65f,0.65f,0.65f,1.0f };
    GLfloat amb0[4]  = { 0.05f,0.05f,0.05f,1.0f };
    GLfloat spec0[4] = { 0.00f,0.00f,0.00f,1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  dif0);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  amb0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spec0);

    // “Spot” suave no altar
    glEnable(GL_LIGHT2);
    GLfloat pos2[4] = { 0.0f, 4.0f, -22.0f, 1.0f };
    GLfloat dif2[4] = { 0.55f,0.55f,0.55f,1.0f };
    GLfloat amb2[4] = { 0.04f,0.04f,0.04f,1.0f };
    glLightfv(GL_LIGHT2, GL_POSITION, pos2);
    glLightfv(GL_LIGHT2, GL_DIFFUSE,  dif2);
    glLightfv(GL_LIGHT2, GL_AMBIENT,  amb2);

    // Materiais sem brilho especular (global)
    GLfloat specMat[4] = {0,0,0,1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specMat);
    glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
}


void updateFlashlight(){
    if (flashlightOn){
        glEnable(GL_LIGHT1);
        GLfloat pos[4]={camX,camY,camZ,1};
        float fx,fy,fz,rx,rz; getLookVectors(fx,fy,fz,rx,rz);
        GLfloat dir[3]={fx,fy,fz};
        GLfloat dif[4]={1.0f,1.0f,0.9f,1.0f};
        GLfloat amb[4]={0.00f,0.00f,0.00f,1.0f};
        glLightfv(GL_LIGHT1,GL_POSITION,pos);
        glLightfv(GL_LIGHT1,GL_SPOT_DIRECTION,dir);
        glLightfv(GL_LIGHT1,GL_DIFFUSE,dif);
        glLightfv(GL_LIGHT1,GL_AMBIENT,amb);
        glLightf(GL_LIGHT1,GL_SPOT_CUTOFF,25.0f);
        glLightf(GL_LIGHT1,GL_SPOT_EXPONENT,12.0f);
        glLightf(GL_LIGHT1,GL_LINEAR_ATTENUATION,0.06f);
    } else glDisable(GL_LIGHT1);
}

void applyCamera(){
    float fx,fy,fz,rx,rz; getLookVectors(fx,fy,fz,rx,rz);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    gluLookAt(camX,camY,camZ, camX+fx,camY+fy,camZ+fz, 0,1,0);
}

//================== COLISÃO E MOVIMENTO ==================
// Função para verificar colisão com cadeiras
bool checkChairCollision(float x, float z){
    // Usa o mesmo arranjo de cadeiras de drawChairsLayout(),
    // mas só checa fileiras próximas para performance
    const float startZ = 8.0f;
    const float endZ   = -16.0f;
    const float pitchZ = 2.5f;
    const float baseX  = 2.5f;   // posição base em cada lado
    const float pitchX = 1.0f;   // espaçamento entre colunas
    const int   cols   = 4;      // 4 colunas por lado
    const float chairRadius = 0.35f; // raio aproximado da cadeira
    const float sumR2 = (RADIUS + chairRadius) * (RADIUS + chairRadius);

    // Estimar fileira mais próxima
    int idx = (int)std::round((startZ - z) / pitchZ);
    int minIdx = std::max(idx - 2, 0);
    int maxIdx = std::min(idx + 2, (int)std::floor((startZ - endZ) / pitchZ));

    for (int i = minIdx; i <= maxIdx; ++i){
        float zChair = startZ - i * pitchZ;
        if (zChair < endZ || zChair > startZ) continue;

        for(int c=0;c<cols;++c){
            float off = pitchX * c;
            float chairXR =  baseX + off;   // lado direito
            float chairXL = -baseX - off;   // lado esquerdo

            // Direita
            float dx = x - chairXR;
            float dz = z - zChair;
            if (dx*dx + dz*dz < sumR2) return true;

            // Esquerda
            dx = x - chairXL; dz = z - zChair;
            if (dx*dx + dz*dz < sumR2) return true;
        }
    }
    return false;
}

// Função para verificar colisão com objetos do altar
bool checkAltarCollision(float x, float z) {
    // Altar principal
    if (x >= -2.5f && x <= 2.5f && z >= -23.5f && z <= -21.5f) {
        return true;
    }

    // Ambão
    if (x >= -3.40f && x <= -2.60f && z >= -18.3f && z <= -17.7f) {
        return true;
    }

    // Estátuas laterais
    if (x >= -4.9f && x <= -4.1f && z >= -20.2f && z <= -19.8f) { // esquerda
        return true;
    }
    if (x >= 4.1f && x <= 4.9f && z >= -20.2f && z <= -19.8f) { // direita
        return true;
    }

    // Cruz processional
    if (x >= 2.35f && x <= 2.65f && z >= -21.15f && z <= -20.85f) {
        return true;
    }

    return false;
}

void collideAndMove(float& nx,float& ny,float& nz,float ox,float oy,float oz){
    if (flyingMode) {
        // No modo voo, não há colisão - movimento livre
        return;
    }
    
    // Colisão com paredes
    float halfW=CH_WIDTH*0.5f, backZ=-25, frontZ=15;
    float tx=nx, ty=ny, tz=nz;
    
    // Manter altura do chão
    if (ty < EYE_H) ty = EYE_H;
    
    // Colisão com paredes laterais
    if (oz>backZ-RADIUS && oz<frontZ+RADIUS){
        if (tx<-halfW+RADIUS) tx=-halfW+RADIUS;
        if (tx> halfW-RADIUS) tx= halfW-RADIUS;
    }
    
    // Colisão com paredes frontal e traseira
    if (tx > -halfW-RADIUS && tx < halfW+RADIUS){
        if (tz<backZ+RADIUS) tz=backZ+RADIUS;
        if (tz>frontZ-RADIUS){ 
            // Se a porta estiver fechada, não pode passar
            if (!doorOpen && std::fabs(tx)>DOOR_HALF) tz=frontZ-RADIUS; 
        }
    }
    
    // Verificar colisão com cadeiras/objetos e tentar "deslizar" nos eixos
    if (checkChairCollision(tx, tz) || checkAltarCollision(tx, tz)) {
        // Tenta mover só em X
        float tryX = tx, tryZ = oz;
        if (!checkChairCollision(tryX, tryZ) && !checkAltarCollision(tryX, tryZ)) {
            tz = tryZ;
        } else {
            // Tenta mover só em Z
            tryX = ox; tryZ = tz;
            if (!checkChairCollision(tryX, tryZ) && !checkAltarCollision(tryX, tryZ)) {
                tx = tryX;
            } else {
                // Nenhum eixo possível: fica parado
                tx = ox; tz = oz;
            }
        }
    }
    
    nx = tx;
    ny = ty;
    nz = tz;
}

//================== OVERLAY 2D (MIRA) ==================
void drawCrosshair(){
    // Salva projeção/modelview atuais
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Desliga profundidade e iluminação para overlay
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    // Coordenadas do centro da tela
    int cx = WIN_W / 2;
    int cy = WIN_H / 2;

    // Desenha cruz simples
    glLineWidth(2.0f);
    glColor3f(1.0f, 1.0f, 1.0f); // branco
    glBegin(GL_LINES);
        glVertex2i(cx - 8, cy); glVertex2i(cx + 8, cy); // horizontal
        glVertex2i(cx, cy - 8); glVertex2i(cx, cy + 8); // vertical
    glEnd();

    // Restaura estado
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();              // MODELVIEW
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();              // PROJECTION
    glMatrixMode(GL_MODELVIEW);
}

//================== RENDER LOOP ==================
void display(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    applyCamera();
    updateFlashlight();

    // 1) opacos
    drawChurchOpaque();
    // 2) transparências (vitral)
    drawChurchWindows();

    drawCrosshair();

    glutSwapBuffers();
}

void reshape(int w,int h){
    if (h<=0) h=1; WIN_W=w; WIN_H=h;
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(70.0, (double)w/(double)h, 0.3, 220.0);
    glMatrixMode(GL_MODELVIEW);
}

//================== INPUT / UPDATE ==================
void captureMouseCenter(){ if(!mouseCaptured) return; glutSetCursor(GLUT_CURSOR_NONE); glutWarpPointer(WIN_W/2,WIN_H/2); }
void passiveMotion(int x,int y){
    if(!mouseCaptured) return; int cx=WIN_W/2, cy=WIN_H/2; int dx=x-cx, dy=y-cy;
    const float sens=0.12f; yawDeg+=dx*sens; pitchDeg-=dy*sens; clampPitch(); captureMouseCenter();
}
void keyDownCb(unsigned char k,int,int){
    unsigned char kk = (k>='A' && k<='Z') ? (k-'A'+'a') : k; // normaliza para minúscula
    keyDown[kk]=true;
    shiftHeld = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0;

    if(kk==27) std::exit(0);
    else if(kk=='f') flashlightOn=!flashlightOn;
    else if(kk=='v'){
        flyingMode=!flyingMode; 
        if(!flyingMode && camY < EYE_H) camY = EYE_H; 
    }
    else if(k=='r'||k=='R'){ camX=0; camY=EYE_H; camZ=30; yawDeg=0; pitchDeg=0; flyingMode=false; doorOpen=false; }
    else if(k=='m'||k=='M'){ mouseCaptured=!mouseCaptured; if(mouseCaptured) captureMouseCenter(); else glutSetCursor(GLUT_CURSOR_LEFT_ARROW); }
    else if(k=='e'||k=='E'){ doorOpen=!doorOpen; }
}
void keyUpCb(unsigned char k,int,int){
    unsigned char kk = (k>='A' && k<='Z') ? (k-'A'+'a') : k; // normaliza
    keyDown[kk]=false;
    shiftHeld = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0;
}
void spDownCb(int k,int,int){ spDown[k]=true; shiftHeld = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0; }
void spUpCb(int k,int,int){ spDown[k]=false; shiftHeld = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0; }

void idle(){
    int now=glutGet(GLUT_ELAPSED_TIME); 
    float dt=(now-lastMs)/1000.0f; 
    lastMs=now;
    
    // Limitar delta time para evitar movimentos bruscos
    if (dt > 0.05f) dt = 0.05f;
    
    float fx,fy,fz,rx,rz; 
    getLookVectors(fx,fy,fz,rx,rz);
    float mvx=0, mvy=0, mvz=0;
    
    // Verificar se alguma tecla está pressionada antes de calcular movimento
    bool anyKeyPressed = false;
    
    // Movimento horizontal
    if(keyDown['w']||keyDown['W']){ mvx+=fx; mvz+=fz; anyKeyPressed = true; }
    if(keyDown['s']||keyDown['S']){ mvx-=fx; mvz-=fz; anyKeyPressed = true; }
    if(keyDown['a']||keyDown['A']){ mvx-=rx; mvz-=rz; anyKeyPressed = true; }
    if(keyDown['d']||keyDown['D']){ mvx+=rx; mvz+=rz; anyKeyPressed = true; }
    
    // Movimento vertical (apenas no modo voo)
    if(flyingMode) {
        if(keyDown[' ']) { mvy += 1.0f; anyKeyPressed = true; }  // Espaço para subir
        if(keyDown['c']||keyDown['C']) { mvy -= 1.0f; anyKeyPressed = true; }  // C para descer
    }
    
    // Se nenhuma tecla está pressionada, nada a fazer
    anyKeyPressed = keyDown['w']||keyDown['W']||keyDown['s']||keyDown['S']||
                         keyDown['a']||keyDown['A']||keyDown['d']||keyDown['D']||
                         (flyingMode && (keyDown[' ']||keyDown['c']||keyDown['C']));
    if (!anyKeyPressed){
        return;
    }

    // Normalizar movimento horizontal
    float len=std::sqrt(mvx*mvx+mvz*mvz); 
    if(len>0.0001f){ mvx/=len; mvz/=len; }

    // Calcular velocidade
    float speed=baseSpeed; 
    if(shiftHeld) speed*=1.8f;

    // Calcular nova posição (limitando o passo para evitar saltos)
    float maxDt = 0.05f; // redundante, mas protege contra spikes
    float usedDt = (dt>maxDt?maxDt:dt);
    float oldX=camX, oldY=camY, oldZ=camZ; 
    float newX=camX+mvx*speed*usedDt, newY=camY+mvy*speed*usedDt, newZ=camZ+mvz*speed*usedDt;

    // Aplicar colisão
    collideAndMove(newX,newY,newZ,oldX,oldY,oldZ); 
    camX=newX; camY=newY; camZ=newZ; 
    glutPostRedisplay();
}

//================== INIT/MAIN ==================
void initGL(){
    glClearDepth(1.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    #ifdef GL_MULTISAMPLE
    glEnable(GL_MULTISAMPLE);
    #endif

    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    setupLights();
}

int main(int argc,char** argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(WIN_W,WIN_H);
    glutCreateWindow(TITLE);

    initGL();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyDownCb);
    glutKeyboardUpFunc(keyUpCb);
    glutSpecialFunc(spDownCb);
    glutSpecialUpFunc(spUpCb);
    glutPassiveMotionFunc(passiveMotion);

    lastMs=glutGet(GLUT_ELAPSED_TIME); captureMouseCenter();

    const GLubyte* r=glGetString(GL_RENDERER);
    const GLubyte* v=glGetString(GL_VENDOR);
    const GLubyte* ver=glGetString(GL_VERSION);
    if(r&&v&&ver) std::printf("Renderer: %s\nVendor  : %s\nVersion : %s\n", r,v,ver);

    glutMainLoop(); return 0;
}
