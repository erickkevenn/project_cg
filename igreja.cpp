// igreja_pentagono.cpp
// Compilar no Windows (FreeGLUT):
// g++ igreja_pentagono.cpp -o igreja.exe -lfreeglut -lopengl32 -lglu32

#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

//================== CONFIG ==================
static const char* TITLE = "Igreja Moderna 3D — V=Voar, F=Lanterna, R=Reset, M=Mouse, E=Porta";
int   WIN_W = 1280, WIN_H = 720;

const float CH_WIDTH  = 12.0f;   // x interno -6..+6 (mantido para colisão)
const float CH_DEPTH  = 40.0f;   // z interno -25..+15 (mantido p/ colisão/chão)
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

static inline float deg2rad(float d){ return d*3.1415926535f/180.0f; }
static inline void  clampPitch(){ pitchDeg = std::max(-89.0f, std::min(89.0f, pitchDeg)); }

void getLookVectors(float& fx,float& fy,float& fz,float& rx,float& rz){
    float yaw = deg2rad(yawDeg), pitch = deg2rad(pitchDeg);
    float cp=std::cos(pitch), sp=std::sin(pitch);
    float sy=std::sin(yaw),   cy=std::cos(yaw);
    fx =  cp*sy; fy = sp; fz = -cp*cy; rx = cy; rz = sy;
}

//================== PRIMITIVAS ==================
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

//================== PORTAL A-FRAME (fachada em V) ==================
void drawPortalAFrame(float zCenter, bool addCross, bool groundLevel = false){
    const float fr=0.82f, fg=0.84f, fb=0.86f;
    glPushMatrix(); glTranslatef(-3.9f, 3.6f, zCenter); glRotatef(-17,0,0,1);
    drawBox(0,0,0, 0.45f, 5.2f, 0.50f, fr,fg,fb); glPopMatrix();
    glPushMatrix(); glTranslatef( 3.9f, 3.6f, zCenter); glRotatef( 17,0,0,1);
    drawBox(0,0,0, 0.45f, 5.2f, 0.50f, fr,fg,fb); glPopMatrix();
    drawBox(0.0f, 5.2f, zCenter, 6.6f, 0.45f, 0.50f, fr,fg,fb);

    if (addCross){
        drawBox(0.0f, 4.6f, zCenter+0.2f, 1.6f, 1.0f, 0.06f, 0.94f,0.95f,0.97f);
        drawCross(0.0f, 6.2f, zCenter+0.1f, 0.9f, 0.95f,0.95f,0.97f);
    }
    if (groundLevel){
        drawBox(0.0f, 0.1f, zCenter, 6.6f, 0.2f, 0.50f, fr*0.9f, fg*0.9f, fb*0.9f);
    }
}

void drawAFrameFacade(){
    const float baseW = 14.0f, baseH = 2.2f, baseZ = 16.8f;
    const float gapW  = 4.8f;
    const float sideW = (baseW - gapW) * 0.5f;
    const float stoneR=0.62f, stoneG=0.58f, stoneB=0.54f;

    drawBox(-(gapW*0.5f + sideW*0.5f), baseH*0.5f, baseZ, sideW, baseH, 0.30f, stoneR,stoneG,stoneB);
    drawBox( (gapW*0.5f + sideW*0.5f), baseH*0.5f, baseZ, sideW, baseH, 0.30f, stoneR,stoneG,stoneB);
    drawBox(0.0f, 0.12f, baseZ, gapW, 0.24f, 0.28f, stoneR*0.9f, stoneG*0.9f, stoneB*0.9f);

    const float zFront = 16.8f;
    const float zBack  = 16.0f;
    drawPortalAFrame(zFront, true,  false);
    drawPortalAFrame(zBack,  false, true );
}

//================== ENTRADA ESTILO FOTO ==================
void drawPhotoStyleEntrance(){
    const float doorW = DOOR_HALF*2.0f;   // 4.0 m
    const float doorH = 3.0f;
    const float frameW= 0.35f;
    const float zFace = 14.75f;           // recuada para dentro
    const float fr=0.72f, fg=0.74f, fb=0.77f;

    drawBox(-(doorW*0.5f + frameW*0.5f), doorH*0.5f, zFace, frameW, doorH, 0.20f, fr,fg,fb);
    drawBox( (doorW*0.5f + frameW*0.5f), doorH*0.5f, zFace, frameW, doorH, 0.20f, fr,fg,fb);
    drawBox(0.0f, doorH + frameW*0.5f, zFace, doorW + frameW*2.0f, frameW, 0.20f, fr,fg,fb);

    drawBox(0.0f, 3.1f, 15.0f, 0.8f, 0.25f, 0.25f, 0.90f,0.90f,0.92f);
    drawBox(0.0f, FLOOR_Y+0.01f, 14.6f, 3.8f, 0.02f, 1.2f, 0.75f,0.10f,0.10f);
    drawBox(0.0f, FLOOR_Y+0.02f, 14.9f, 1.9f, 0.02f, 0.7f, 0.55f,0.40f,0.22f);
}

//================== JARDIM/EXTERIOR ==================
void drawGarden(){
    float trunkR=0.6f, trunkG=0.4f, trunkB=0.2f;
    float leavesR=0.2f, leavesG=0.6f, leavesB=0.2f;
    drawBox(-8.0f, FLOOR_Y+3.0f, 10.0f, 0.3f, 6.0f, 0.3f, trunkR, trunkG, trunkB);
    drawBox( 8.0f, FLOOR_Y+3.0f, 10.0f, 0.3f, 6.0f, 0.3f, trunkR, trunkG, trunkB);
    drawBox(-10.0f,FLOOR_Y+3.0f, 5.0f, 0.3f, 6.0f, 0.3f, trunkR, trunkG, trunkB);
    drawBox( 10.0f,FLOOR_Y+3.0f, 5.0f, 0.3f, 6.0f, 0.3f, trunkR, trunkG, trunkB);
    drawSphere(-8.0f, FLOOR_Y+6.5f,10.0f,1.0f,12,12, leavesR, leavesG, leavesB);
    drawSphere( 8.0f, FLOOR_Y+6.5f,10.0f,1.0f,12,12, leavesR, leavesG, leavesB);
    drawSphere(-10.0f,FLOOR_Y+6.5f, 5.0f,1.0f,12,12, leavesR, leavesG, leavesB);
    drawSphere( 10.0f,FLOOR_Y+6.5f, 5.0f,1.0f,12,12, leavesR, leavesG, leavesB);
}

void drawFrontPath(){
    const float pathW = 4.8f;
    const float pathL = 22.0f;
    const float z0    = 16.2f;
    drawBox(0.0f, FLOOR_Y-0.005f, z0 + pathL*0.5f, pathW, 0.02f, pathL, 0.72f,0.72f,0.74f);
    drawBox(-pathW*0.5f + 0.15f, FLOOR_Y-0.003f, z0 + pathL*0.5f, 0.30f, 0.015f, pathL, 0.60f,0.60f,0.62f);
    drawBox( pathW*0.5f - 0.15f, FLOOR_Y-0.003f, z0 + pathL*0.5f, 0.30f, 0.015f, pathL, 0.60f,0.60f,0.62f);
    const float tileW = 0.80f, tileL = 0.45f, gap = 0.10f;
    for (float z = z0 + 0.35f; z < z0 + pathL - 0.35f; z += tileL + gap){
        for (float x = -pathW*0.5f + 0.40f; x <= pathW*0.5f - 0.40f; x += tileW + gap){
            drawBox(x, FLOOR_Y-0.001f, z, tileW, 0.01f, tileL, 0.80f,0.80f,0.83f);
        }
    }
}

//================== VITRAIS ==================
void drawStainedGlassXY(float w=1.4f,float h=2.6f,float archH=0.7f){
    float frameT=0.05f, gap=0.03f, bodyH=h-archH;
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
    pane(xL,yM+gap,xM-gap,yT, 0.15f,0.65f,0.15f,0.75f);
    pane(xM+gap,yM+gap,xR,yT, 0.95f,0.45f,0.05f,0.75f);
    pane(xL,yB,xM-gap,yM-gap, 0.95f,0.60f,0.15f,0.75f);
    pane(xM+gap,yB,xR,yM-gap, 0.20f,0.75f,0.20f,0.75f);

    glBegin(GL_TRIANGLE_FAN);
    glColor4f(0.15f,0.45f,0.85f,0.75f);
    glVertex3f(0, bodyH+archH*0.6f, 0.01f);
    for(int i=0;i<=seg;i++){
        float t=i/(float)seg, ang=3.14159265f*(1.0f-t);
        float x=(w*0.5f-0.05f)*std::cos(ang), y=bodyH+(archH-0.05f)*std::sin(ang);
        glVertex3f(x,y,0.01f);
    }
    glEnd();

    glLineWidth(2.0f); glColor4f(0.92f,0.92f,0.94f,0.95f);
    glBegin(GL_LINES);
    glVertex3f(0,0.05f,0.02f); glVertex3f(0,bodyH+archH-0.05f,0.02f);
    glVertex3f(-w*0.5f+0.06f,yM,0.02f); glVertex3f(w*0.5f-0.06f,yM,0.02f);
    glEnd();
}

//================== HELPERS PAREDES & TELHADO ==================
static void drawWallSegment(float x0,float z0,float x1,float z1,
                            float height, float thickness,
                            float r,float g,float b){
    float dx = x1 - x0, dz = z1 - z0;
    float len = std::sqrt(dx*dx + dz*dz);
    float angDeg = std::atan2(dz, dx) * 180.0f / 3.1415926535f;
    float cx = (x0 + x1) * 0.5f;
    float cz = (z0 + z1) * 0.5f;
    glPushMatrix();
    glTranslatef(cx, height*0.5f, cz);
    glRotatef(-angDeg, 0,1,0);
    drawBox(0,0,0, len, height, thickness, r,g,b);
    glPopMatrix();
}

static void drawWallSegmentAtY(float x0,float z0,float x1,float z1,
                               float yMid, float height, float thickness,
                               float r,float g,float b){
    float dx = x1 - x0, dz = z1 - z0;
    float len = std::sqrt(dx*dx + dz*dz);
    float angDeg = std::atan2(dz, dx) * 180.0f / 3.1415926535f;
    float cx = (x0 + x1) * 0.5f;
    float cz = (z0 + z1) * 0.5f;
    glPushMatrix();
    glTranslatef(cx, yMid, cz);
    glRotatef(-angDeg, 0,1,0);
    drawBox(0,0,0, len, height, thickness, r,g,b);
    glPopMatrix();
}

static inline void triNormal(float ax,float ay,float az,
                             float bx,float by,float bz,
                             float cx,float cy,float cz){
    float ux = bx-ax, uy = by-ay, uz = bz-az;
    float vx = cx-ax, vy = cy-ay, vz = cz-az;
    float nx = uy*vz - uz*vy;
    float ny = uz*vx - ux*vz;
    float nz = ux*vy - uy*vx;
    float len = std::sqrt(nx*nx+ny*ny+nz*nz);
    if(len>1e-6f){ nx/=len; ny/=len; nz/=len; }
    glNormal3f(nx,ny,nz);
}

// x das laterais em função de z, usando as arestas V4→V0 (esq) e V2→V3 (dir)
static inline float xLeftAtZ(float z){
    const float x1=-4.0f, z1=-25.0f, x2=-6.0f, z2=10.0f; // V4→V0
    float t=(z - z1)/(z2 - z1); return x1 + t*(x2 - x1);
}
static inline float xRightAtZ(float z){
    const float x1= 4.0f, z1=-25.0f, x2= 6.0f, z2=10.0f; // V3→V2
    float t=(z - z1)/(z2 - z1); return x1 + t*(x2 - x1);
}

//================== CASCO PENTAGONAL + TELHADO ==================
static void drawChurchPentagonShell(){
    const float H = CH_HEIGHT;
    const float T = WALL_T;
    const float wallR=0.80f, wallG=0.74f, wallB=0.68f;
    const float stoneR=0.62f, stoneG=0.58f, stoneB=0.54f;
    const float friezeR=0.86f, friezeG=0.84f, friezeB=0.82f;

    float V[5][2] = {
        {-6.0f, 10.0f}, { 0.0f, 15.0f}, { 6.0f, 10.0f},
        { 4.0f,-25.0f}, {-4.0f,-25.0f}
    };

    auto W = [&](int i,int j){
        drawWallSegment(V[i][0],V[i][1], V[j][0],V[j][1], H, T, wallR,wallG,wallB);
        drawWallSegmentAtY(V[i][0],V[i][1], V[j][0],V[j][1], 0.175f, 0.35f, T+0.18f, stoneR,stoneG,stoneB);
        drawWallSegmentAtY(V[i][0],V[i][1], V[j][0],V[j][1], H-0.10f, 0.20f, T+0.08f, friezeR,friezeG,friezeB);
    };

    W(0,1); W(1,2); W(2,3); W(3,4); W(4,0);

    for(int i=0;i<5;++i){
        float x=V[i][0], z=V[i][1];
        drawBox(x, H*0.5f, z, 0.28f, H, 0.28f, wallR*0.95f, wallG*0.95f, wallB*0.95f);
    }

    float cx=0, cz=0; for(int i=0;i<5;++i){ cx+=V[i][0]; cz+=V[i][1]; } cx/=5.0f; cz/=5.0f;
    const float apexY = H + 2.6f;
    const float roofR = 0.52f, roofG = 0.16f, roofB = 0.12f;

    glColor3f(roofR,roofG,roofB);
    glBegin(GL_TRIANGLES);
    for(int i=0;i<5;++i){
        int j=(i+1)%5;
        float ax=V[i][0], ay=H, az=V[i][1];
        float bx=V[j][0], by=H, bz=V[j][1];
        float cxr=cx, cyr=apexY, czr=cz;
        triNormal(ax,ay,az, bx,by,bz, cxr,cyr,czr);
        glVertex3f(ax,ay,az);
        glVertex3f(bx,by,bz);
        glVertex3f(cxr,cyr,czr);
    }
    glEnd();

    for(int i=0;i<5;++i){
        int j=(i+1)%5;
        drawWallSegmentAtY(V[i][0],V[i][1], V[j][0],V[j][1], H-0.02f, 0.06f, T+0.22f, roofR*0.85f,roofG*0.85f,roofB*0.85f);
    }
}

//================== PISO INTERNO (preenche o pentágono) ==================
void drawPentagonFloor(){
    float V[5][2]={{-6,10},{0,15},{6,10},{4,-25},{-4,-25}};
    float cx=0, cz=0; for(int i=0;i<5;++i){ cx+=V[i][0]; cz+=V[i][1]; } cx/=5.0f; cz/=5.0f;
    const float y = FLOOR_Y - 0.02f; // um hair abaixo para evitar z-fighting
    const float floorR=0.78f, floorG=0.77f, floorB=0.75f;

    glColor3f(floorR,floorG,floorB);
    glBegin(GL_TRIANGLES);
    for(int i=0;i<5;++i){
        int j=(i+1)%5;
        glVertex3f(cx, y, cz);
        glVertex3f(V[i][0], y, V[i][1]);
        glVertex3f(V[j][0], y, V[j][1]);
    }
    glEnd();
}

//================== VITRAIS POR DENTRO ==================
static void drawWindowInsideOnSegment(float x0,float z0,float x1,float z1,float t){
    float x = x0 + (x1-x0)*t;
    float z = z0 + (z1-z0)*t;
    float dx=x1-x0, dz=z1-z0;
    float len=std::sqrt(dx*dx+dz*dz); if(len<1e-6f) return;
    float nx = -dz/len, nz = dx/len; // normal interna (para dentro do polígono)
    float angDeg = std::atan2(dz, dx) * 180.0f / 3.1415926535f + 90.0f;

    const float inset = 0.12f; // desloca para dentro
    glPushMatrix();
    glTranslatef(x + nx*inset, 2.0f, z + nz*inset);
    glRotatef(angDeg, 0,1,0);
    drawStainedGlassXY();
    glPopMatrix();
}

void drawChurchWindows(){
    float V[5][2]={{-6,10},{0,15},{6,10},{4,-25},{-4,-25}};
    auto duo=[&](int i,int j){
        drawWindowInsideOnSegment(V[i][0],V[i][1], V[j][0],V[j][1], 0.33f);
        drawWindowInsideOnSegment(V[i][0],V[i][1], V[j][0],V[j][1], 0.66f);
    };

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    duo(2,3); // direita
    duo(4,0); // esquerda

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

//================== OBJETOS REALISTAS (internos) ==================
void drawRealisticAltar(){
    drawBox(0.0f, FLOOR_Y+0.15f, -22.5f, 5.0f, 0.3f, 2.0f, 0.1f, 0.1f, 0.1f);
    drawBox(0.0f, FLOOR_Y+0.35f, -22.3f, 4.5f, 0.2f, 1.6f, 0.15f, 0.15f, 0.15f);
    float marbleR=0.95f, marbleG=0.95f, marbleB=0.97f;
    drawBox(0.0f, FLOOR_Y+0.75f, -22.5f, 3.8f, 1.1f, 1.4f, marbleR, marbleG, marbleB);
    drawBox(0.0f, FLOOR_Y+0.75f, -21.9f, 3.0f, 1.0f, 0.05f, 0.12f, 0.12f, 0.15f);
    drawBox(0.0f, FLOOR_Y+1.0f,  -21.85f,1.2f, 0.4f, 0.02f, 0.9f, 0.7f, 0.2f);
    drawBox(0.0f, FLOOR_Y+1.3f,  -22.5f, 4.0f, 0.05f, 1.5f, 1.0f, 1.0f, 1.0f);
    drawBox(-0.8f,FLOOR_Y+1.35f, -22.3f, 0.5f, 0.03f, 0.3f, 0.7f, 0.4f, 0.3f);
    drawBox(-0.8f,FLOOR_Y+1.38f, -22.3f, 0.48f,0.01f, 0.28f,0.95f,0.95f,0.98f);
}

void drawRealisticCrucifix(){
    float marbleR=0.88f, marbleG=0.88f, marbleB=0.90f;
    drawBox(0.0f, 3.0f, -24.9f, 4.0f, 6.0f, 0.1f, marbleR, marbleG, marbleB);
    float woodR=0.45f, woodG=0.30f, woodB=0.20f;
    drawBox(0.0f, 3.8f, -24.8f, 0.2f, 3.5f, 0.08f, woodR, woodG, woodB);
    drawBox(0.0f, 4.5f, -24.8f, 1.8f, 0.2f, 0.08f, woodR, woodG, woodB);
    float skinR=0.95f, skinG=0.85f, skinB=0.75f;
    drawBox(0.0f, 4.2f, -24.75f, 0.15f, 0.8f, 0.05f, skinR, skinG, skinB);
    drawBox(0.0f, 4.6f, -24.75f, 0.12f, 0.12f, 0.05f, skinR, skinG, skinB);
    drawBox(-0.3f,4.4f,-24.75f, 0.6f, 0.08f, 0.05f, skinR, skinG, skinB);
    drawBox(0.0f, 3.9f, -24.7f, 0.25f, 0.3f, 0.03f, 1.0f, 1.0f, 1.0f);
}

void drawRealisticStatues(){
    float blueR=0.2f, blueG=0.4f, blueB=0.8f;
    float whiteR=0.95f, whiteG=0.95f, whiteB=0.98f;
    float skinR=0.95f, skinG=0.85f, skinB=0.75f;
    drawBox(-4.5f, FLOOR_Y+1.2f, -20.0f, 0.8f, 0.1f, 0.4f, 1.0f, 1.0f, 1.0f);
    drawBox(-4.5f, FLOOR_Y+1.0f, -20.0f, 0.3f, 1.2f, 0.2f, blueR, blueG, blueB);
    drawSphere(-4.5f, FLOOR_Y+1.8f, -20.0f, 0.12f, 12, 12, skinR, skinG, skinB);
    drawBox(-4.5f, FLOOR_Y+1.3f, -20.0f, 0.4f, 0.8f, 0.15f, whiteR, whiteG, whiteB);

    float brownR=0.6f, brownG=0.4f, brownB=0.2f;
    drawBox( 4.5f, FLOOR_Y+1.2f, -20.0f, 0.8f, 0.1f, 0.4f, 1.0f, 1.0f, 1.0f);
    drawBox( 4.5f, FLOOR_Y+1.0f, -20.0f, 0.3f, 1.2f, 0.2f, brownR, brownG, brownB);
    drawSphere( 4.5f, FLOOR_Y+1.8f, -20.0f, 0.12f, 12, 12, skinR, skinG, skinB);
    drawBox( 4.7f, FLOOR_Y+1.2f, -20.0f, 0.03f, 1.0f, 0.03f, 0.4f, 0.3f, 0.2f);
}

void drawRealisticFlowers(){
    float redR=0.9f, redG=0.2f, redB=0.2f;
    float orangeR=0.95f, orangeG=0.5f, orangeB=0.1f;
    float yellowR=0.95f, yellowG=0.8f, yellowB=0.1f;
    float purpleR=0.7f, purpleG=0.3f, purpleB=0.8f;
    float greenR=0.2f, greenG=0.6f, greenB=0.2f;
    drawSphere(-4.2f, FLOOR_Y+1.4f, -19.8f, 0.06f, 8, 8, redR, redG, redB);
    drawSphere(-4.8f, FLOOR_Y+1.4f, -19.8f, 0.06f, 8, 8, orangeR, orangeG, orangeB);
    drawSphere(-4.5f, FLOOR_Y+1.5f, -19.8f, 0.06f, 8, 8, yellowR, yellowG, yellowB);
    drawSphere(-4.3f, FLOOR_Y+1.3f, -19.8f, 0.06f, 8, 8, purpleR, purpleG, purpleB);
    drawSphere(-4.5f, FLOOR_Y+1.2f, -19.8f, 0.1f, 8, 8, greenR, greenG, greenB);
    drawSphere( 4.2f, FLOOR_Y+1.4f, -19.8f, 0.06f, 8, 8, redR, redG, redB);
    drawSphere( 4.8f, FLOOR_Y+1.4f, -19.8f, 0.06f, 8, 8, orangeR, orangeG, orangeB);
    drawSphere( 4.5f, FLOOR_Y+1.5f, -19.8f, 0.06f, 8, 8, yellowR, yellowG, yellowB);
    drawSphere( 4.3f, FLOOR_Y+1.3f, -19.8f, 0.06f, 8, 8, purpleR, purpleG, purpleB);
    drawSphere( 4.5f, FLOOR_Y+1.2f, -19.8f, 0.1f, 8, 8, greenR, greenG, greenB);
}

void drawProcessionalCross(){
    float goldR=0.9f, goldG=0.7f, goldB=0.2f;
    drawBox(2.5f, FLOOR_Y+0.1f, -21.0f, 0.3f, 0.2f, 0.3f, goldR, goldG, goldB);
    drawBox(2.5f, FLOOR_Y+2.0f, -21.0f, 0.05f, 4.0f, 0.05f, goldR, goldG, goldB);
    drawBox(2.5f, FLOOR_Y+4.2f, -21.0f, 0.05f, 0.4f, 0.05f, goldR, goldG, goldB);
    drawBox(2.5f, FLOOR_Y+4.0f, -21.0f, 0.3f, 0.05f, 0.05f, goldR, goldG, goldB);
    drawBox(2.5f, FLOOR_Y+4.2f, -20.95f, 0.02f, 0.15f, 0.02f, goldR, goldG, goldB);
    drawBox(2.5f, FLOOR_Y+4.15f,-20.95f, 0.1f, 0.02f, 0.02f, goldR, goldG, goldB);
}

// --- Símbolo do ambão ---
void drawAmbaoSymbol(float xCenter, float yCenter, float zFrontFace){
    const float M_R=0.94f, M_G=0.94f, M_B=0.96f, K=0.06f, zSym=zFrontFace+0.01f;
    drawBox(xCenter, yCenter, zFrontFace, 0.42f, 0.65f, 0.02f, M_R,M_G,M_B);
    drawBox(xCenter, yCenter+0.18f, zSym, 0.03f, 0.42f, 0.02f, K,K,K);
    drawBox(xCenter, yCenter+0.30f, zSym, 0.16f, 0.03f, 0.02f, K,K,K);
    drawBox(xCenter, yCenter-0.33f, zSym, 0.26f, 0.03f, 0.02f, K,K,K);
    drawBox(xCenter, yCenter-0.10f, zSym, 0.15f, 0.10f, 0.02f, K,K,K);
    drawBox(xCenter, yCenter-0.22f, zSym, 0.03f, 0.12f, 0.02f, K,K,K);
    glPushMatrix(); glTranslatef(xCenter-0.12f, yCenter-0.08f, zSym); glRotatef(20,0,0,1);
    drawBox(0,0,0, 0.05f, 0.12f, 0.02f, K,K,K); glPopMatrix();
    glPushMatrix(); glTranslatef(xCenter+0.12f, yCenter-0.08f, zSym); glRotatef(-20,0,0,1);
    drawBox(0,0,0, 0.05f, 0.12f, 0.02f, K,K,K); glPopMatrix();
    drawSphere(xCenter+0.19f, yCenter-0.04f, zSym+0.01f, 0.05f, 14,14, K,K,K);
}

void drawAmbao(){
    float woodR=0.4f, woodG=0.25f, woodB=0.15f;
    float marbleR=0.95f, marbleG=0.95f, marbleB=0.97f;
    float x = -2.5f, z = -18.0f;
    drawBox(x, FLOOR_Y+0.15f, z, 0.8f, 0.3f, 0.6f, woodR, woodG, woodB);
    drawBox(x, FLOOR_Y+0.7f,  z, 0.7f, 1.4f, 0.5f, woodR, woodG, woodB);
    drawBox(x, FLOOR_Y+1.4f, z, 0.9f, 0.1f, 0.6f, marbleR, marbleG, marbleB);
    drawBox(x, FLOOR_Y+0.05f, z+0.2f, 0.6f, 0.1f, 0.25f, woodR*0.8f, woodG*0.8f, woodB*0.8f);
    float zFront = z + 0.5f;
    drawAmbaoSymbol(x, FLOOR_Y + 0.95f, zFront);
}

//================== PORTA (recuada para dentro) ==================
void drawDoor(){
    const float zDoor = 14.75f;           // porta recuada
    float doorR=0.6f, doorG=0.4f, doorB=0.2f;
    float handleR=0.9f, handleG=0.7f, handleB=0.2f;

    if (doorOpen) {
        glPushMatrix();
        glTranslatef(-1.0f, 0.0f, zDoor);
        glRotatef(90, 0, 1, 0);
        glTranslatef(1.0f, 0.0f, 0.0f);
        drawBox(-1.0f, 1.5f, 0.0f, 0.1f, 3.0f, 2.0f, doorR, doorG, doorB);
        glPopMatrix();

        glPushMatrix();
        glTranslatef( 1.0f, 0.0f, zDoor);
        glRotatef(-90, 0, 1, 0);
        glTranslatef(-1.0f, 0.0f, 0.0f);
        drawBox( 1.0f, 1.5f, 0.0f, 0.1f, 3.0f, 2.0f, doorR, doorG, doorB);
        glPopMatrix();

        drawBox(-1.8f, 1.5f, zDoor+0.05f, 0.05f, 0.1f, 0.05f, handleR, handleG, handleB);
        drawBox( 1.8f, 1.5f, zDoor+0.05f, 0.05f, 0.1f, 0.05f, handleR, handleG, handleB);
    } else {
        drawBox(-1.0f, 1.5f, zDoor, 2.0f, 3.0f, 0.1f, doorR, doorG, doorB);
        drawBox( 1.0f, 1.5f, zDoor, 2.0f, 3.0f, 0.1f, doorR, doorG, doorB);
        drawBox(-0.2f, 1.5f, zDoor+0.05f, 0.05f, 0.1f, 0.05f, handleR, handleG, handleB);
        drawBox( 0.2f, 1.5f, zDoor+0.05f, 0.05f, 0.1f, 0.05f, handleR, handleG, handleB);

        drawBox(-1.0f, 1.5f, zDoor+0.02f, 1.8f, 0.1f, 0.05f, doorR*0.8f, doorG*0.8f, doorB*0.8f);
        drawBox(-1.0f, 1.5f, zDoor+0.02f, 0.1f, 2.8f, 0.05f, doorR*0.8f, doorG*0.8f, doorB*0.8f);
        drawBox(-1.0f, 2.5f, zDoor+0.02f, 1.8f, 0.1f, 0.05f, doorR*0.8f, doorG*0.8f, doorB*0.8f);
        drawBox(-1.0f, 0.5f, zDoor+0.02f, 1.8f, 0.1f, 0.05f, doorR*0.8f, doorG*0.8f, doorB*0.8f);

        drawBox( 1.0f, 1.5f, zDoor+0.02f, 1.8f, 0.1f, 0.05f, doorR*0.8f, doorG*0.8f, doorB*0.8f);
        drawBox( 1.0f, 1.5f, zDoor+0.02f, 0.1f, 2.8f, 0.05f, doorR*0.8f, doorG*0.8f, doorB*0.8f);
        drawBox( 1.0f, 2.5f, zDoor+0.02f, 1.8f, 0.1f, 0.05f, doorR*0.8f, doorG*0.8f, doorB*0.8f);
        drawBox( 1.0f, 0.5f, zDoor+0.02f, 1.8f, 0.1f, 0.05f, doorR*0.8f, doorG*0.8f, doorB*0.8f);
    }
}

//================== CADEIRAS ==================
void drawPlasticChairWhite(){
    const float body = 0.93f;
    const float frame= 0.88f;
    drawBox(0.0f, 0.44f, 0.0f,  0.48f, 0.05f, 0.46f,  body, body, body);
    drawBox(0.0f, 0.78f, -0.20f, 0.48f, 0.52f, 0.06f, body, body, body);
    drawBox(0.0f, 1.02f, -0.20f, 0.44f, 0.12f, 0.06f, body, body, body);
    const float legT = 0.06f, legH = 0.44f;
    drawBox(-0.20f, legH*0.5f, -0.20f, legT, legH, legT, frame, frame, frame);
    drawBox( 0.20f, legH*0.5f, -0.20f, legT, legH, legT, frame, frame, frame);
    drawBox(-0.20f, legH*0.5f,  0.18f, legT, legH, legT, frame, frame, frame);
    drawBox( 0.20f, legH*0.5f,  0.18f, legT, legH, legT, frame, frame, frame);
    drawBox(0.0f, 0.18f, -0.20f, 0.42f, 0.04f, 0.04f, frame, frame, frame);
    drawBox(0.0f, 0.18f,  0.18f, 0.42f, 0.04f, 0.04f, frame, frame, frame);
    drawBox(-0.20f,0.18f, 0.0f,  0.04f, 0.04f, 0.36f, frame, frame, frame);
    drawBox( 0.20f,0.18f, 0.0f,  0.04f, 0.04f, 0.36f, frame, frame, frame);
}

void drawChairsLayout(){
    const float marginLR = 0.60f;    // margem das paredes
    const float pitchX    = 1.00f;   // passo entre colunas
    const float pitchZ    = 2.40f;   // passo entre fileiras
    const float startZ    = 8.0f;    // não encosta no vértice frontal
    const float endZ      = -16.0f;

    for (float z = startZ; z >= endZ; z -= pitchZ){
        float xL = xLeftAtZ(z)  + marginLR;
        float xR = xRightAtZ(z) - marginLR;
        float width = xR - xL;
        if (width < 0.90f) continue;

        int cols = (int)std::floor(width / pitchX);
        if (cols < 1) continue;

        float mid = (xL + xR) * 0.5f;
        int half = cols/2;
        for(int k=-half; k<=half; ++k){
            if (cols % 2 == 0 && k==0) continue; // simetria para número par
            float x = mid + k*pitchX;
            if (x - 0.30f < xL || x + 0.30f > xR) continue;
            glPushMatrix();
            glTranslatef(x, 0, z);
            glRotatef(180, 0,1,0); // voltadas para o altar
            drawPlasticChairWhite();
            glPopMatrix();
        }
    }
}

//================== VENTILADORES ==================
void drawCeilingFan(float x, float y, float z, bool isLeftSide = true){
    float fanR = 0.2f, fanG = 0.2f, fanB = 0.2f;
    glPushMatrix(); glTranslatef(x, y, z);
    if (isLeftSide) glRotatef(90, 0, 1, 0); else glRotatef(-90, 0, 1, 0);
    drawBox(0.0f, 0.0f, 0.0f, 0.3f, 0.2f, 0.1f, fanR, fanG, fanB);
    drawBox(0.0f, 0.0f, 0.05f, 0.2f, 0.15f, 0.1f, fanR*0.8f, fanG*0.8f, fanB*0.8f);
    drawBox(0.0f, 0.0f, 0.1f, 0.8f, 0.05f, 0.02f, fanR*0.9f, fanG*0.9f, fanB*0.9f);
    glPushMatrix(); glTranslatef(0.0f, 0.0f, 0.1f); glRotatef(120,0,0,1);
    drawBox(0.0f, 0.0f, 0.0f, 0.8f, 0.05f, 0.02f, fanR*0.9f, fanG*0.9f, fanB*0.9f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.0f, 0.1f); glRotatef(240,0,0,1);
    drawBox(0.0f, 0.0f, 0.0f, 0.8f, 0.05f, 0.02f, fanR*0.9f, fanG*0.9f, fanB*0.9f); glPopMatrix();
    glPopMatrix();
}

//================== LUZ/CÂMERA ==================
void setupLights(){
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat globalAmb[4] = {0.14f,0.14f,0.14f,1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);

    GLfloat pos0[4]  = { 0.0f, 7.0f, 0.0f, 1.0f };
    GLfloat dif0[4]  = { 0.65f,0.65f,0.65f,1.0f };
    GLfloat amb0[4]  = { 0.05f,0.05f,0.05f,1.0f };
    GLfloat spec0[4] = { 0.00f,0.00f,0.00f,1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  dif0);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  amb0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spec0);

    glEnable(GL_LIGHT2);
    GLfloat pos2[4] = { 0.0f, 4.0f, -22.0f, 1.0f };
    GLfloat dif2[4] = { 0.55f,0.55f,0.55f,1.0f };
    GLfloat amb2[4] = { 0.04f,0.04f,0.04f,1.0f };
    glLightfv(GL_LIGHT2, GL_POSITION, pos2);
    glLightfv(GL_LIGHT2, GL_DIFFUSE,  dif2);
    glLightfv(GL_LIGHT2, GL_AMBIENT,  amb2);

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
void collideAndMove(float& nx,float& ny,float& nz,float ox,float oy,float oz){
    if (flyingMode) return;
    const float halfW = CH_WIDTH*0.5f;
    const float backZ = -25.0f;
    const float frontZ = 15.0f;

    float tx=nx, ty=ny, tz=nz;
    if (ty < EYE_H) ty = EYE_H;

    if (oz > backZ - RADIUS && oz < frontZ + RADIUS){
        if (tx < -halfW + RADIUS) tx = -halfW + RADIUS;
        if (tx >  halfW - RADIUS) tx =  halfW - RADIUS;
    }
    if (tx > -halfW - RADIUS && tx < halfW + RADIUS){
        if (oz >= backZ + RADIUS && tz < backZ + RADIUS) tz = backZ + RADIUS;
        if (!doorOpen){
            if (oz <= frontZ - RADIUS && tz > frontZ - RADIUS) tz = frontZ - RADIUS;
            if (oz >= frontZ + RADIUS && tz < frontZ + RADIUS) tz = frontZ + RADIUS;
        }
    }
    nx=tx; ny=ty; nz=tz;
}

//================== RENDER LOOP ==================
void drawChurchOpaque(){
    // Exterior
    drawFrontPath();
    drawGarden();
    drawBox(0.0f, -0.06f, 30.0f, 120.0f, 0.1f, 120.0f, 0.70f,0.88f,0.72f);

    // Casco e piso
    drawChurchPentagonShell();
    drawPentagonFloor();

    // Fachada/porta
    drawAFrameFacade();
    drawPhotoStyleEntrance();
    drawDoor();

    // Interior
    drawChairsLayout();
    drawRealisticCrucifix();
    drawRealisticAltar();
    drawAmbao();
    drawRealisticStatues();
    drawRealisticFlowers();
    drawProcessionalCross();

    // Ventiladores laterais
    drawCeilingFan(-5.8f, 4.5f, 0.0f, true);
    drawCeilingFan( 5.8f, 4.5f, 0.0f, false);
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    applyCamera();
    updateFlashlight();

    // 1) opacos
    drawChurchOpaque();
    // 2) transparências (vitral)
    drawChurchWindows();

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
    keyDown[k]=true;
    if(k==27) std::exit(0);
    else if(k=='f'||k=='F') flashlightOn=!flashlightOn;
    else if(k=='v'||k=='V'){ flyingMode=!flyingMode; if(!flyingMode && camY < EYE_H) camY = EYE_H; }
    else if(k=='r'||k=='R'){ camX=0; camY=EYE_H; camZ=27.2f; yawDeg=0; pitchDeg=0; flyingMode=false; doorOpen=false; }
    else if(k=='m'||k=='M'){ mouseCaptured=!mouseCaptured; if(mouseCaptured) captureMouseCenter(); else glutSetCursor(GLUT_CURSOR_LEFT_ARROW); }
    else if(k=='e'||k=='E'){ doorOpen=!doorOpen; }
}
void keyUpCb(unsigned char k,int,int){ keyDown[k]=false; }
void spDownCb(int k,int,int){ spDown[k]=true; } void spUpCb(int k,int,int){ spDown[k]=false; }

void idle(){
    int now=glutGet(GLUT_ELAPSED_TIME);
    float dt=(now-lastMs)/1000.0f; lastMs=now;
    if (dt > 0.05f) dt = 0.05f;

    float fx,fy,fz,rx,rz; getLookVectors(fx,fy,fz,rx,rz);
    float mvx=0, mvy=0, mvz=0;

    if(keyDown['w']||keyDown['W']){ mvx+=fx; mvz+=fz; }
    if(keyDown['s']||keyDown['S']){ mvx-=fx; mvz-=fz; }
    if(keyDown['a']||keyDown['A']){ mvx-=rx; mvz-=rz; }
    if(keyDown['d']||keyDown['D']){ mvx+=rx; mvz+=rz; }

    if(flyingMode) {
        if(keyDown[' ']) mvy += 1.0f;
        if(keyDown['c']||keyDown['C']) mvy -= 1.0f;
    }

    float len=std::sqrt(mvx*mvx+mvz*mvz); if(len>0.0001f){ mvx/=len; mvz/=len; }
    float speed=baseSpeed; if(spDown[GLUT_KEY_SHIFT_L]||spDown[GLUT_KEY_SHIFT_R]) speed*=1.8f;

    float oldX=camX, oldY=camY, oldZ=camZ;
    float newX=camX+mvx*speed*dt, newY=camY+mvy*speed*dt, newZ=camZ+mvz*speed*dt;

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
