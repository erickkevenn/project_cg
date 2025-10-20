#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <vector>

static const char* TITLE = "Igreja - Tabuleiro dos Martins ";
int   WIN_W = 1280, WIN_H = 720;

const float CH_WIDTH  = 12.0f;  
const float CH_DEPTH  = 40.0f;   
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

// Porta removida: sem animação/som

static float speedMul = 1.0f; // suavização de corrida
static inline float deg2rad(float d){ return d*3.1415926535f/180.0f; }
static inline void  clampPitch(){ pitchDeg = std::max(-89.0f, std::min(89.0f, pitchDeg)); }

//================== TEXTURAS (PROCEDURAIS) ==================
GLuint TEX_FLOOR = 0, TEX_GRASS = 0, TEX_MARBLE = 0, TEX_WOOD = 0, TEX_TILE = 0, TEX_PLASTER = 0, TEX_STONE = 0, TEX_REFLECTIVE_TILES = 0, TEX_CROSS = 0;

static void createTextureRGBA(GLuint &texId, int w, int h, const std::vector<unsigned char>& data){
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
}

static std::vector<unsigned char> genChecker(int w,int h,int step,
	unsigned char r1,unsigned char g1,unsigned char b1,
	unsigned char r2,unsigned char g2,unsigned char b2){
	std::vector<unsigned char> img(w*h*4);
	for(int y=0;y<h;++y){ for(int x=0;x<w;++x){
		int c = ((x/step) + (y/step)) & 1;
		unsigned char r = c? r1: r2;
		unsigned char g = c? g1: g2;
		unsigned char b = c? b1: b2;
		int i=(y*w+x)*4; img[i]=r; img[i+1]=g; img[i+2]=b; img[i+3]=255;
	}}
	return img;
}

static std::vector<unsigned char> genNoiseMarble(int w,int h){
	std::vector<unsigned char> img(w*h*4);
	for(int y=0;y<h;++y){ for(int x=0;x<w;++x){
		float u = x/(float)w, v = y/(float)h;
		
		// Veios principais do mármore
		float veins1 = std::sin((u*8.0f + v*2.0f) * 3.14159265f + std::sin(u*15.0f)*0.3f);
		float veins2 = std::sin((u*6.0f - v*1.8f) * 3.14159265f + std::cos(v*12.0f)*0.4f);
		float veins3 = std::sin((u*4.0f + v*3.0f) * 3.14159265f + std::sin(u*25.0f + v*8.0f)*0.2f);
		
		// Combinação dos veios
		float combined = (veins1 * 0.5f + veins2 * 0.3f + veins3 * 0.2f);
		
		// Base do mármore com variações sutis
		float base = 0.85f + 0.15f * std::sin(u*30.0f + v*20.0f) * 0.3f;
		
		// Aplicar os veios
		float final = base + combined * 0.25f;
		final = std::max(0.0f, std::min(1.0f, final));
		
		// Cores do mármore (branco com tons azulados/cinzentos)
		unsigned char r = (unsigned char)(final * 255);
		unsigned char g = (unsigned char)(final * 0.98f * 255);
		unsigned char b = (unsigned char)(final * 0.95f * 255);
		
		int i=(y*w+x)*4; img[i]=r; img[i+1]=g; img[i+2]=b; img[i+3]=255;
	}}
	return img;
}

static std::vector<unsigned char> genWood(int w,int h){
	std::vector<unsigned char> img(w*h*4);
	for(int y=0;y<h;++y){ for(int x=0;x<w;++x){
		float u = x/(float)w; float v = y/(float)h;
		
		// Anéis de crescimento da árvore
		float rings = std::sin((u*12.0f + v*0.3f)*6.28318f) * 0.5f + 0.5f;
		
		// Grãos da madeira (linhas verticais)
		float grain = std::sin(v*40.0f + std::sin(u*15.0f)*0.5f) * 0.3f + 0.7f;
		
		// Variações naturais da madeira
		float variation = std::sin(u*25.0f + v*18.0f) * 0.15f + 0.85f;
		
		// Combinação dos elementos
		float pattern = rings * 0.6f + grain * 0.4f;
		float final = pattern * variation;
		
		// Cores base da madeira (mogno)
		float baseR=0.65f, baseG=0.45f, baseB=0.28f;
		
		// Aplicar variações de cor
		float r = baseR * final;
		float g = baseG * final;
		float b = baseB * final;
		
		// Adicionar tons mais escuros nos anéis
		if (rings > 0.7f) {
			r *= 0.8f; g *= 0.8f; b *= 0.8f;
		}
		
		unsigned char red=(unsigned char)(std::min(1.0f, r)*255);
		unsigned char green=(unsigned char)(std::min(1.0f, g)*255);
		unsigned char blue=(unsigned char)(std::min(1.0f, b)*255);
		
		int i=(y*w+x)*4; img[i]=red; img[i+1]=green; img[i+2]=blue; img[i+3]=255;
	}}
	return img;
}

static std::vector<unsigned char> genGrass(int w,int h){
	std::vector<unsigned char> img(w*h*4);
	for(int y=0;y<h;++y){ for(int x=0;x<w;++x){
		float u=x/(float)w, v=y/(float)h;
		
		// Padrão de grama com variações
		float grass1 = (std::sin(u*60.0f)+std::cos(v*45.0f))*0.4f;
		float grass2 = (std::sin(u*30.0f + v*20.0f))*0.3f;
		float grass3 = (std::sin(u*90.0f + v*70.0f))*0.2f;
		
		// Combinação dos padrões
		float pattern = grass1 + grass2 + grass3;
		
		// Base da grama com variações naturais
		float base = 0.65f + 0.20f * (pattern * 0.5f + 0.5f);
		
		// Adicionar manchas escuras e claras
		float spots = std::sin(u*25.0f + v*18.0f) * 0.1f;
		base += spots;
		
		// Cores da grama (verde natural)
		float r = base * 0.4f;   // vermelho baixo
		float g = base * 0.9f;   // verde alto
		float b = base * 0.3f;   // azul baixo
		
		// Variações de cor para diferentes tons de verde
		if (pattern > 0.3f) {
			g *= 1.1f;  // mais verde nas áreas claras
		} else {
			r *= 0.8f; g *= 0.8f; b *= 0.8f;  // mais escuro nas áreas escuras
		}
		
		// Garantir que os valores estejam no range correto
		r = std::max(0.0f, std::min(1.0f, r));
		g = std::max(0.0f, std::min(1.0f, g));
		b = std::max(0.0f, std::min(1.0f, b));
		
		unsigned char red=(unsigned char)(r*255);
		unsigned char green=(unsigned char)(g*255);
		unsigned char blue=(unsigned char)(b*255);
		
		int i=(y*w+x)*4; img[i]=red; img[i+1]=green; img[i+2]=blue; img[i+3]=255;
	}}
	return img;
}

static std::vector<unsigned char> genPlaster(int w,int h){
	std::vector<unsigned char> img(w*h*4);
	for(int y=0;y<h;++y){ for(int x=0;x<w;++x){
		float u=x/(float)w, v=y/(float)h;
		
		// Textura irregular do reboco
		float noise1 = (std::sin(u*120.0f)+std::cos(v*95.0f))*0.08f;
		float noise2 = (std::sin(u*60.0f + v*40.0f))*0.05f;
		float noise3 = (std::sin(u*200.0f + v*150.0f))*0.03f;
		
		// Variações de cor do reboco (tons de bege/amarelo claro)
		float base = 0.88f + noise1 + noise2 + noise3;
		
		// Adicionar manchas e irregularidades
		float spots = std::sin(u*80.0f + v*60.0f) * 0.02f;
		base += spots;
		
		// Cores do reboco (bege claro com variações)
		float r = base * 0.98f;  // ligeiramente mais amarelado
		float g = base * 0.95f;
		float b = base * 0.92f;
		
		// Garantir que os valores estejam no range correto
		r = std::max(0.0f, std::min(1.0f, r));
		g = std::max(0.0f, std::min(1.0f, g));
		b = std::max(0.0f, std::min(1.0f, b));
		
		unsigned char red=(unsigned char)(r*255);
		unsigned char green=(unsigned char)(g*255);
		unsigned char blue=(unsigned char)(b*255);
		
		int i=(y*w+x)*4; img[i]=red; img[i+1]=green; img[i+2]=blue; img[i+3]=255;
	}}
	return img;
}

static std::vector<unsigned char> genStone(int w,int h){
	std::vector<unsigned char> img(w*h*4);
	for(int y=0;y<h;++y){ for(int x=0;x<w;++x){
		float u=x/(float)w, v=y/(float)h;
		
		// Textura de pedra com fissuras e irregularidades
		float crack1 = std::sin(u*40.0f + v*15.0f) * 0.3f;
		float crack2 = std::sin(u*25.0f - v*20.0f) * 0.2f;
		float crack3 = std::sin(u*60.0f + v*35.0f) * 0.15f;
		
		// Combinação das fissuras
		float cracks = crack1 + crack2 + crack3;
		
		// Base da pedra com variações
		float base = 0.75f + 0.15f * std::sin(u*80.0f + v*60.0f);
		
		// Aplicar as fissuras (mais escuras)
		float final = base - std::abs(cracks) * 0.2f;
		final = std::max(0.0f, std::min(1.0f, final));
		
		// Cores da pedra (cinza com tons azulados)
		float r = final * 0.85f;
		float g = final * 0.82f;
		float b = final * 0.78f;
		
		// Adicionar variações de cor
		float colorVar = std::sin(u*30.0f + v*25.0f) * 0.05f;
		r += colorVar;
		g += colorVar * 0.8f;
		b += colorVar * 0.6f;
		
		// Garantir que os valores estejam no range correto
		r = std::max(0.0f, std::min(1.0f, r));
		g = std::max(0.0f, std::min(1.0f, g));
		b = std::max(0.0f, std::min(1.0f, b));
		
		unsigned char red=(unsigned char)(r*255);
		unsigned char green=(unsigned char)(g*255);
		unsigned char blue=(unsigned char)(b*255);
		
		int i=(y*w+x)*4; img[i]=red; img[i+1]=green; img[i+2]=blue; img[i+3]=255;
	}}
	return img;
}

static std::vector<unsigned char> genReflectiveTiles(int w,int h){
	std::vector<unsigned char> img(w*h*4);
	for(int y=0;y<h;++y){ for(int x=0;x<w;++x){
		float u=x/(float)w, v=y/(float)h;
		
		// Padrão diagonal de azulejos (como na imagem)
		float tileSize = 0.15f; // tamanho dos azulejos
		float diagonal = (u + v) / tileSize;
		float antiDiagonal = (u - v) / tileSize;
		
		// Determinar se está em um azulejo ou na junta
		float tileX = std::fmod(diagonal, 1.0f);
		float tileY = std::fmod(antiDiagonal, 1.0f);
		
		// Juntas dos azulejos (mais escuras)
		float jointWidth = 0.08f;
		bool isJoint = (tileX < jointWidth || tileX > (1.0f - jointWidth) || 
		               tileY < jointWidth || tileY > (1.0f - jointWidth));
		
		// Base dos azulejos (cinza claro/off-white)
		float baseR = 0.92f, baseG = 0.92f, baseB = 0.94f;
		
		// Juntas (cinza escuro)
		float jointR = 0.35f, jointG = 0.35f, jointB = 0.37f;
		
		// Variações sutis nos azulejos
		float variation = std::sin(u*25.0f + v*30.0f) * 0.03f;
		
		float r, g, b;
		if (isJoint) {
			r = jointR;
			g = jointG;
			b = jointB;
		} else {
			r = baseR + variation;
			g = baseG + variation;
			b = baseB + variation;
			
			// Adicionar reflexos sutis
			float reflection = std::sin(u*40.0f + v*35.0f) * 0.05f;
			r += reflection;
			g += reflection;
			b += reflection;
		}
		
		// Garantir que os valores estejam no range correto
		r = std::max(0.0f, std::min(1.0f, r));
		g = std::max(0.0f, std::min(1.0f, g));
		b = std::max(0.0f, std::min(1.0f, b));
		
		unsigned char red=(unsigned char)(r*255);
		unsigned char green=(unsigned char)(g*255);
		unsigned char blue=(unsigned char)(b*255);
		
		int i=(y*w+x)*4; img[i]=red; img[i+1]=green; img[i+2]=blue; img[i+3]=255;
	}}
	return img;
}

static std::vector<unsigned char> genCrossTexture(int w,int h){
	std::vector<unsigned char> img(w*h*4);
	for(int y=0;y<h;++y){ for(int x=0;x<w;++x){
		float u=x/(float)w, v=y/(float)h;
		
		// Base de madeira escura (marrom escuro)
		float woodR = 0.35f, woodG = 0.25f, woodB = 0.15f;
		
		// Grãos da madeira
		float grain1 = std::sin(v*60.0f + std::sin(u*20.0f)*0.3f) * 0.15f;
		float grain2 = std::sin(u*40.0f + v*15.0f) * 0.1f;
		
		// Variações naturais da madeira
		float variation = std::sin(u*80.0f + v*60.0f) * 0.08f;
		
		// Aplicar grãos e variações
		float r = woodR + grain1 + grain2 + variation;
		float g = woodG + grain1*0.8f + grain2*0.6f + variation*0.7f;
		float b = woodB + grain1*0.6f + grain2*0.4f + variation*0.5f;
		
		// Adicionar detalhes dourados nas bordas (como na imagem)
		float edgeDistance = std::min(std::min(u, 1.0f-u), std::min(v, 1.0f-v));
		if (edgeDistance < 0.1f) {
			float goldIntensity = (0.1f - edgeDistance) / 0.1f;
			r += goldIntensity * 0.3f; // dourado
			g += goldIntensity * 0.25f;
			b += goldIntensity * 0.1f;
		}
		
		// Garantir que os valores estejam no range correto
		r = std::max(0.0f, std::min(1.0f, r));
		g = std::max(0.0f, std::min(1.0f, g));
		b = std::max(0.0f, std::min(1.0f, b));
		
		unsigned char red=(unsigned char)(r*255);
		unsigned char green=(unsigned char)(g*255);
		unsigned char blue=(unsigned char)(b*255);
		
		int i=(y*w+x)*4; img[i]=red; img[i+1]=green; img[i+2]=blue; img[i+3]=255;
	}}
	return img;
}

void initTextures(){
	const int W=256,H=256;
	createTextureRGBA(TEX_TILE,   W,H, genChecker(W,H,32, 210,210,215, 160,160,165));
	createTextureRGBA(TEX_FLOOR,  W,H, genChecker(W,H,24, 200,200,205, 170,170,175));
	createTextureRGBA(TEX_GRASS,  W,H, genGrass(W,H));
	createTextureRGBA(TEX_MARBLE, W,H, genNoiseMarble(W,H));
	createTextureRGBA(TEX_WOOD,   W,H, genWood(W,H));
	createTextureRGBA(TEX_PLASTER,W,H, genPlaster(W,H));
	createTextureRGBA(TEX_STONE,  W,H, genStone(W,H));
	createTextureRGBA(TEX_REFLECTIVE_TILES, W,H, genReflectiveTiles(W,H));
	createTextureRGBA(TEX_CROSS,  W,H, genCrossTexture(W,H));
}

// Quad com repetição de UV
void drawTexturedQuad(float x0,float y0,float z0, float x1,float y1,float z1,
	float x2,float y2,float z2, float x3,float y3,float z3,
	float uRepeat, float vRepeat){
	glBegin(GL_QUADS);
		// normal aproximada por cruz do triângulo 0-1-2
		float ux=x1-x0, uy=y1-y0, uz=z1-z0;
		float vx=x2-x0, vy=y2-y0, vz=z2-z0;
		float nx=uy*vz-uz*vy, ny=uz*vx-ux*vz, nz=ux*vy-uy*vx;
		float len = std::sqrt(nx*nx+ny*ny+nz*nz); if(len>0){ nx/=len; ny/=len; nz/=len; }
		glNormal3f(nx,ny,nz);
		glTexCoord2f(0,0); glVertex3f(x0,y0,z0);
		glTexCoord2f(uRepeat,0); glVertex3f(x1,y1,z1);
		glTexCoord2f(uRepeat,vRepeat); glVertex3f(x2,y2,z2);
		glTexCoord2f(0,vRepeat); glVertex3f(x3,y3,z3);
	glEnd();
}

void getLookVectors(float& fx,float& fy,float& fz,float& rx,float& rz){
    float yaw = deg2rad(yawDeg), pitch = deg2rad(pitchDeg);
    float cp=std::cos(pitch), sp=std::sin(pitch);
    float sy=std::sin(yaw),   cy=std::cos(yaw);
    fx =  cp*sy; fy = sp; fz = -cp*cy; rx = cy; rz = sy;
}

//================== FUNÇÃO PRINCIPAL PARA OS BLOCOS ==================
void drawBox(float cx,float cy,float cz,float sx,float sy,float sz,
             float r,float g,float b,bool wire=false){
    glPushMatrix();
    glTranslatef(cx,cy,cz); glScalef(sx,sy,sz); glColor3f(r,g,b);
    if (wire) glutWireCube(1.0); else glutSolidCube(1.0);
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
    
    // Aplicar textura da cruz
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, TEX_CROSS);
    
    // Braço vertical da cruz
    drawBox(x,y,z, th,2.0f*s,th, r,g,b);
    // Braço horizontal da cruz
    drawBox(x,y+0.4f*s,z, 1.2f*s,th,th, r,g,b);
    
    glDisable(GL_TEXTURE_2D);
}
void drawPortalAFrame(float zCenter, bool addCross, bool groundLevel = false){
    const float fr=0.82f, fg=0.84f, fb=0.86f; // cinza claro

    // Aplicar textura de pedra nos elementos estruturais
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, TEX_STONE);
    
    glPushMatrix(); glTranslatef(-3.9f, 3.6f, zCenter); glRotatef(-17,0,0,1);
    drawBox(0,0,0, 0.45f, 5.2f, 0.50f, fr,fg,fb); glPopMatrix();
    glPushMatrix(); glTranslatef( 3.9f, 3.6f, zCenter); glRotatef( 17,0,0,1);
    drawBox(0,0,0, 0.45f, 5.2f, 0.50f, fr,fg,fb); glPopMatrix();
    drawBox(0.0f, 5.2f, zCenter, 6.6f, 0.45f, 0.50f, fr,fg,fb);

    glDisable(GL_TEXTURE_2D);

    if (addCross){
        // Cruz com textura de mármore
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, TEX_MARBLE);
        drawBox(0.0f, 4.6f, zCenter+0.2f, 1.6f, 1.0f, 0.06f, 0.94f,0.95f,0.97f);
        glDisable(GL_TEXTURE_2D);
        drawCross(0.0f, 6.2f, zCenter+0.1f, 0.9f, 0.95f,0.95f,0.97f);
    }
    
    if (groundLevel){
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, TEX_STONE);
        drawBox(0.0f, 0.1f, zCenter, 6.6f, 0.2f, 0.50f, fr*0.9f, fg*0.9f, fb*0.9f);
        glDisable(GL_TEXTURE_2D);
    }
}

//================== FACHADA EM A-FRAME (V INVERTIDO) ==================
void drawAFrameFacade(){
    // Base de pedra com RECORTE central
    const float baseW = 14.0f, baseH = 2.2f, baseZ = 16.8f;
    const float gapW  = 4.8f;
    const float sideW = (baseW - gapW) * 0.5f;
    const float stoneR=0.62f, stoneG=0.58f, stoneB=0.54f;

    drawBox(-(gapW*0.5f + sideW*0.5f), baseH*0.5f, baseZ, sideW, baseH, 0.30f, stoneR,stoneG,stoneB);
    drawBox( (gapW*0.5f + sideW*0.5f), baseH*0.5f, baseZ, sideW, baseH, 0.30f, stoneR,stoneG,stoneB);
    drawBox(0.0f, 0.12f, baseZ, gapW, 0.24f, 0.28f, stoneR*0.9f, stoneG*0.9f, stoneB*0.9f);

    const float zFront = 16.8f; // arco da frente
    const float zBack  = 16.0f; // arco de trás

    drawPortalAFrame(zFront, true,  false); // frente: com cruz/placa
    drawPortalAFrame(zBack,  false, true ); // trás: sem cruz/placa
}

//================== ENTRADA ==================
void drawPhotoStyleEntrance(){
    // Moldura recuada que NÃO fecha o vão
    const float doorW = DOOR_HALF*2.0f;  
    const float doorH = 3.0f;
    const float frameW= 0.35f;            
    const float zFace = 14.90f;          
    const float fr=0.72f, fg=0.74f, fb=0.77f;

    // ombreiras
    drawBox(-(doorW*0.5f + frameW*0.5f), doorH*0.5f, zFace, frameW, doorH, 0.20f, fr,fg,fb);
    drawBox( (doorW*0.5f + frameW*0.5f), doorH*0.5f, zFace, frameW, doorH, 0.20f, fr,fg,fb);
    // verga
    drawBox(0.0f, doorH + frameW*0.5f, zFace, doorW + frameW*2.0f, frameW, 0.20f, fr,fg,fb);

    drawBox(0.0f, 3.1f, 15.0f, 0.8f, 0.25f, 0.25f, 0.90f,0.90f,0.92f);

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

//================== VITRAIS ==================
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

//================== OBJETOS ==================
void drawRealisticAltar(){
    // Degrau de escada com textura de pedra
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, TEX_STONE);
    drawBox(0.0f, FLOOR_Y+0.15f, -22.5f, 5.0f, 0.3f, 2.0f, 0.1f, 0.1f, 0.1f);
    
    // Segundo degrau menor
    drawBox(0.0f, FLOOR_Y+0.35f, -22.3f, 4.5f, 0.2f, 1.6f, 0.15f, 0.15f, 0.15f);
    glDisable(GL_TEXTURE_2D);
    
    // Base do altar com textura de mármore
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, TEX_MARBLE);
    float marbleR=0.95f, marbleG=0.95f, marbleB=0.97f;
    drawBox(0.0f, FLOOR_Y+0.75f, -22.5f, 3.8f, 1.1f, 1.4f, marbleR, marbleG, marbleB);
    glDisable(GL_TEXTURE_2D);
    
    // Painel frontal (mármore preto com Agnus Dei dourado)
    drawBox(0.0f, FLOOR_Y+0.75f, -21.9f, 3.0f, 1.0f, 0.05f, 0.12f, 0.12f, 0.15f);
    
    // Detalhe dourado do Agnus Dei (Cordeiro de Deus) 
    drawBox(0.0f, FLOOR_Y+1.0f, -21.85f, 1.2f, 0.4f, 0.02f, 0.9f, 0.7f, 0.2f);
    
    // Toalha de altar (branca com renda nas bordas) 
    drawBox(0.0f, FLOOR_Y+1.3f, -22.5f, 4.0f, 0.05f, 1.5f, 1.0f, 1.0f, 1.0f);
    
    // Bíblia aberta (capa avermelhada/marrom) 
    drawBox(-0.8f, FLOOR_Y+1.35f, -22.3f, 0.5f, 0.03f, 0.3f, 0.7f, 0.4f, 0.3f);
    drawBox(-0.8f, FLOOR_Y+1.38f, -22.3f, 0.48f, 0.01f, 0.28f, 0.95f, 0.95f, 0.98f);
}

void drawRealisticCrucifix(){
    // Parede de mármore cinza claro com textura
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, TEX_MARBLE);
    float marbleR=0.88f, marbleG=0.88f, marbleB=0.90f;
    drawBox(0.0f, 3.0f, -24.9f, 4.0f, 6.0f, 0.1f, marbleR, marbleG, marbleB);
    glDisable(GL_TEXTURE_2D);
    
    // Cruz com nova textura personalizada
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, TEX_CROSS);
    float crossR=0.45f, crossG=0.30f, crossB=0.20f;
    drawBox(0.0f, 3.8f, -24.8f, 0.2f, 3.5f, 0.08f, crossR, crossG, crossB);
    drawBox(0.0f, 4.5f, -24.8f, 1.8f, 0.2f, 0.08f, crossR, crossG, crossB);
    glDisable(GL_TEXTURE_2D);
    
    // Cristo (corpo realista com tons de pele)
    float skinR=0.95f, skinG=0.85f, skinB=0.75f;
    drawBox(0.0f, 4.2f, -24.75f, 0.15f, 1.5f, 0.05f, skinR, skinG, skinB);
    drawBox(0.0f, 4.7f, -24.75f, 0.12f, 0.12f, 0.05f, skinR, skinG, skinB);
    
    // Braços
    drawBox(-0.45f, 4.5f, -24.75f, 0.6f, 0.08f, 0.05f, skinR, skinG, skinB);
    drawBox( 0.45f, 4.5f, -24.75f, 0.6f, 0.08f, 0.05f, skinR, skinG, skinB);
    
    // Pano branco cobrindo a cintura
    drawBox(0.0f, 4.1f, -24.7f, 0.25f, 0.3f, 0.03f, 1.0f, 1.0f, 1.0f);
}

void drawRealisticStatues(){
    const float DY = 2.20f;
    // Estátua da Virgem Maria
    float blueR=0.2f, blueG=0.4f, blueB=0.8f;
    float whiteR=0.95f, whiteG=0.95f, whiteB=0.98f;
    float skinR=0.95f, skinG=0.85f, skinB=0.75f;

    drawBox(-2.5f, FLOOR_Y+0.8f + DY, -24.8f, 0.8f, 0.1f, 0.4f, 1.0f, 1.0f, 1.0f);
    drawBox(-2.5f, FLOOR_Y+1.4f + DY, -24.8f, 0.3f, 1.2f, 0.2f, blueR, blueG, blueB);
    drawSphere(-2.5f, FLOOR_Y+2.2f + DY, -24.8f, 0.12f, 12, 12, skinR, skinG, skinB);
    drawBox(-2.5f, FLOOR_Y+1.7f + DY, -24.8f, 0.4f, 0.8f, 0.15f, whiteR, whiteG, whiteB);
    
    // Estátua de São José
    float brownR=0.6f, brownG=0.4f, brownB=0.2f;
    drawBox( 2.5f, FLOOR_Y+0.8f + DY, -24.8f, 0.8f, 0.1f, 0.4f, 1.0f, 1.0f, 1.0f);
    drawBox( 2.5f, FLOOR_Y+1.4f + DY, -24.8f, 0.3f, 1.2f, 0.2f, brownR, brownG, brownB);
    drawSphere( 2.5f, FLOOR_Y+2.2f + DY, -24.8f, 0.12f, 12, 12, skinR, skinG, skinB);
    drawBox( 2.7f, FLOOR_Y+1.6f + DY, -24.8f, 0.03f, 1.0f, 0.03f, 0.4f, 0.3f, 0.2f);
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

// Símbolo do ambão
void drawAmbaoSymbol(float xCenter, float yCenter, float zFrontFace){
    const float M_R=0.94f, M_G=0.94f, M_B=0.96f;      
    const float K    =0.06f;                         
    const float zSym = zFrontFace + 0.01f;           

    // Placa de mármore
    drawBox(xCenter, yCenter, zFrontFace, 0.42f, 0.65f, 0.02f, M_R,M_G,M_B);

    // Cruz
    drawBox(xCenter, yCenter+0.18f, zSym, 0.03f, 0.42f, 0.02f, K,K,K);  
    drawBox(xCenter, yCenter+0.30f, zSym, 0.16f, 0.03f, 0.02f, K,K,K);   

    // Base inferior
    drawBox(xCenter, yCenter-0.33f, zSym, 0.26f, 0.03f, 0.02f, K,K,K);

    // Cálice
    drawBox(xCenter, yCenter-0.10f, zSym, 0.15f, 0.10f, 0.02f, K,K,K);  
    drawBox(xCenter, yCenter-0.22f, zSym, 0.03f, 0.12f, 0.02f, K,K,K); 

    glPushMatrix(); glTranslatef(xCenter-0.12f, yCenter-0.08f, zSym); glRotatef(20,0,0,1);
    drawBox(0,0,0, 0.05f, 0.12f, 0.02f, K,K,K); glPopMatrix();
    glPushMatrix(); glTranslatef(xCenter+0.12f, yCenter-0.08f, zSym); glRotatef(-20,0,0,1);
    drawBox(0,0,0, 0.05f, 0.12f, 0.02f, K,K,K); glPopMatrix();

    drawSphere(xCenter+0.19f, yCenter-0.04f, zSym+0.01f, 0.05f, 14,14, K,K,K);
}


void drawAmbao(){
    // Ambão
    float woodR=0.4f, woodG=0.25f, woodB=0.15f;
    float marbleR=0.95f, marbleG=0.95f, marbleB=0.97f;
    
    // Posição do ambão: mais para o lado e virado para as cadeiras
    float ambaoX = -2.5f;  
    float ambaoZ = -18.0f; 
    
    // Base do ambão com textura de madeira
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, TEX_WOOD);
    drawBox(ambaoX, FLOOR_Y+0.15f, ambaoZ, 0.8f, 0.3f, 0.6f, woodR, woodG, woodB);
    
    // Estrutura principal do ambão
    drawBox(ambaoX, FLOOR_Y+0.7f, ambaoZ, 0.7f, 1.4f, 0.5f, woodR, woodG, woodB);
    glDisable(GL_TEXTURE_2D);
    
    // Mesa do ambão com textura de mármore
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, TEX_MARBLE);
    drawBox(ambaoX, FLOOR_Y+1.4f, ambaoZ, 0.9f, 0.1f, 0.6f, marbleR, marbleG, marbleB);
    glDisable(GL_TEXTURE_2D);
    
    // Apoio para os pés (pequeno degrau) 
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, TEX_WOOD);
    drawBox(ambaoX, FLOOR_Y+0.05f, ambaoZ+0.2f, 0.6f, 0.1f, 0.25f, woodR*0.8f, woodG*0.8f, woodB*0.8f);
    glDisable(GL_TEXTURE_2D);
    float zFront = ambaoZ + 0.5f;
    drawAmbaoSymbol(ambaoX, FLOOR_Y + 0.95f, zFront);
}

// drawDoor removida (porta não desenhada)

//================== CADEIRAS  ==================
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
    // Cadeiras orientadas para o altar (cruz)
    for (float z=8.0f; z>=-16.0f; z-=2.5f){
        for(int c=0;c<4;++c){
            float off=1.0f*c;
            glPushMatrix(); 
            glTranslatef(2.5f+off,0,z); 
            glRotatef(180, 0, 1, 0);  
            drawPlasticChairWhite(); 
    glPopMatrix();
        
            glPushMatrix(); 
            glTranslatef(-2.5f-off,0,z); 
            glRotatef(180, 0, 1, 0);  
            drawPlasticChairWhite(); 
    glPopMatrix();
        }
    }
}

void drawCeilingFan(float x, float y, float z, bool isLeftSide = true){
    float fanR = 0.2f, fanG = 0.2f, fanB = 0.2f; 
    
    glPushMatrix();
    glTranslatef(x, y, z);
    
    if (isLeftSide) {
        glRotatef(90, 0, 1, 0);  
    } else {
        glRotatef(-90, 0, 1, 0); 
    }

    drawBox(0.0f, 0.0f, 0.0f, 0.3f, 0.2f, 0.1f, fanR, fanG, fanB);
    drawBox(0.0f, 0.0f, 0.05f, 0.2f, 0.15f, 0.1f, fanR*0.8f, fanG*0.8f, fanB*0.8f);
    drawBox(0.0f, 0.0f, 0.1f, 0.8f, 0.05f, 0.02f, fanR*0.9f, fanG*0.9f, fanB*0.9f);
    
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.1f);
    glRotatef(120, 0, 0, 1);
    drawBox(0.0f, 0.0f, 0.0f, 0.8f, 0.05f, 0.02f, fanR*0.9f, fanG*0.9f, fanB*0.9f);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.1f);
    glRotatef(240, 0, 0, 1);
    drawBox(0.0f, 0.0f, 0.0f, 0.8f, 0.05f, 0.02f, fanR*0.9f, fanG*0.9f, fanB*0.9f);
    glPopMatrix();
    
    glPopMatrix();
}

void drawFrontPath(){
    const float pathW = 4.8f;   
    const float pathL = 22.0f;  
    const float z0    = 16.2f;  

    drawBox(0.0f, FLOOR_Y-0.005f, z0 + pathL*0.5f,
            pathW, 0.02f, pathL, 0.72f, 0.72f, 0.74f);

    drawBox(-pathW*0.5f + 0.15f, FLOOR_Y-0.003f, z0 + pathL*0.5f,
            0.30f, 0.015f, pathL, 0.60f, 0.60f, 0.62f);
    drawBox( pathW*0.5f - 0.15f, FLOOR_Y-0.003f, z0 + pathL*0.5f,
            0.30f, 0.015f, pathL, 0.60f, 0.60f, 0.62f);

    const float tileW = 0.80f, tileL = 0.45f, gap = 0.10f;
    for (float z = z0 + 0.35f; z < z0 + pathL - 0.35f; z += tileL + gap){
        for (float x = -pathW*0.5f + 0.40f; x <= pathW*0.5f - 0.40f; x += tileW + gap){
            drawBox(x, FLOOR_Y-0.001f, z, tileW, 0.01f, tileL, 0.80f, 0.80f, 0.83f);
        }
    }
}

//================== IGREJA PRINCIPAL ==================
void drawChurchOpaque(){
	float wallR=0.80f, wallG=0.74f, wallB=0.68f;  
	float ceilR=0.92f, ceilG=0.92f, ceilB=0.94f;  

	// Piso texturizado com azulejos reflexivos
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, TEX_REFLECTIVE_TILES);
	drawTexturedQuad(
		-CH_WIDTH*0.5f, FLOOR_Y-0.05f, 15.0f - CH_DEPTH,
		 CH_WIDTH*0.5f, FLOOR_Y-0.05f, 15.0f - CH_DEPTH,
		 CH_WIDTH*0.5f, FLOOR_Y-0.05f, 15.0f,
		-CH_WIDTH*0.5f, FLOOR_Y-0.05f, 15.0f,
		8.0f, CH_DEPTH*0.3f);

	// Teto
	glDisable(GL_TEXTURE_2D);
	drawBox(0.0f, CH_HEIGHT, -5.0f,   CH_WIDTH, 0.1f, CH_DEPTH,  ceilR,ceilG,ceilB);

	// Paredes laterais e fundo (reboco)
	glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, TEX_PLASTER);
	// Parede esquerda
	drawTexturedQuad(
		-CH_WIDTH*0.5f, 0.0f, 15.0f - CH_DEPTH,
		-CH_WIDTH*0.5f, CH_HEIGHT, 15.0f - CH_DEPTH,
		-CH_WIDTH*0.5f, CH_HEIGHT, 15.0f,
		-CH_WIDTH*0.5f, 0.0f, 15.0f,
		 CH_DEPTH*0.25f, CH_HEIGHT*0.25f);
	// Parede direita
	drawTexturedQuad(
		 CH_WIDTH*0.5f, 0.0f, 15.0f - CH_DEPTH,
		 CH_WIDTH*0.5f, CH_HEIGHT, 15.0f - CH_DEPTH,
		 CH_WIDTH*0.5f, CH_HEIGHT, 15.0f,
		 CH_WIDTH*0.5f, 0.0f, 15.0f,
		 CH_DEPTH*0.25f, CH_HEIGHT*0.25f);
	// Parede do fundo
	drawTexturedQuad(
		-CH_WIDTH*0.5f, 0.0f, -25.0f,
		 CH_WIDTH*0.5f, 0.0f, -25.0f,
		 CH_WIDTH*0.5f, CH_HEIGHT, -25.0f,
		-CH_WIDTH*0.5f, CH_HEIGHT, -25.0f,
		 CH_WIDTH*0.3f, CH_HEIGHT*0.3f);
	glDisable(GL_TEXTURE_2D);

    drawBox(-(CH_WIDTH*0.5f+DOOR_HALF)*0.5f, CH_HEIGHT*0.5f, 15.0f,
            (CH_WIDTH*0.5f-DOOR_HALF), CH_HEIGHT, WALL_T, wallR,wallG,wallB);
    drawBox( (CH_WIDTH*0.5f+DOOR_HALF)*0.5f, CH_HEIGHT*0.5f, 15.0f,
            (CH_WIDTH*0.5f-DOOR_HALF), CH_HEIGHT, WALL_T, wallR,wallG,wallB);
    drawBox(0.0f, (3.0f+CH_HEIGHT)*0.5f, 15.0f, DOOR_HALF*2.0f, (CH_HEIGHT-3.0f), WALL_T, wallR,wallG,wallB);

    drawChairsLayout();
    drawRealisticCrucifix();
    drawRealisticAltar();
    drawAmbao();
    drawRealisticStatues();

    drawProcessionalCross();
    
    drawCeilingFan(-5.8f, 4.5f, 0.0f, true);   
    drawCeilingFan(5.8f, 4.5f, 0.0f, false);   

    drawAFrameFacade();
    drawPhotoStyleEntrance();

	// Caminho frontal com textura no topo das lajotas
	glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, TEX_TILE);
	const float pathW = 4.8f; const float pathL = 22.0f; const float z0 = 16.2f;
	drawTexturedQuad(
		-pathW*0.5f, FLOOR_Y-0.005f, z0,
		 pathW*0.5f, FLOOR_Y-0.005f, z0,
		 pathW*0.5f, FLOOR_Y-0.005f, z0+pathL,
		-pathW*0.5f, FLOOR_Y-0.005f, z0+pathL,
		 3.5f, 10.0f);
	glDisable(GL_TEXTURE_2D);

	// Gramado externo texturizado
	glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, TEX_GRASS);
	drawTexturedQuad(
		-60.0f, -0.06f, -30.0f,
		 60.0f, -0.06f, -30.0f,
		 60.0f, -0.06f, 90.0f,
		-60.0f, -0.06f, 90.0f,
		 24.0f, 48.0f);
	glDisable(GL_TEXTURE_2D);
}

void drawChurchWindows(){
    const float inset = 0.18f;      
    const float xL_in  = -CH_WIDTH*0.5f + inset;   // esquerda (dentro)
    const float xL_out = -CH_WIDTH*0.5f - inset;   // esquerda (fora)
    const float xR_in  =  CH_WIDTH*0.5f - inset;   // direita (dentro)
    const float xR_out =  CH_WIDTH*0.5f + inset;   // direita (fora)

    auto put = [&](float x, float y, float z){
        glPushMatrix();
        glTranslatef(x, y, z);
        if (x > 0) glRotatef( 90, 0,1,0);
        else       glRotatef(-90, 0,1,0);
        drawStainedGlassXY();   
        glPopMatrix();
    };

    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

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

    GLfloat globalAmb[4] = {0.14f,0.14f,0.14f,1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);

    GLfloat pos0[4]  = { 0.0f, 7.0f, 0.0f, 1.0f };
    GLfloat dif0[4]  = { 0.65f,0.65f,0.65f,1.0f };
    GLfloat amb0[4]  = { 0.05f,0.05f,0.05f,1.0f };
    GLfloat spec0[4] = { 0.20f,0.20f,0.20f,1.0f };
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

    // Materiais mais realistas
    GLfloat specMat[4] = {0.18f,0.18f,0.18f,1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specMat);
    glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 18.0f);

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    // Luz direcional suave do "sol" vindo da entrada -> interior
    glEnable(GL_LIGHT3);
    GLfloat dirDif[4] = { 0.35f, 0.35f, 0.38f, 1.0f };
    GLfloat dirAmb[4] = { 0.02f, 0.02f, 0.03f, 1.0f };
    GLfloat sunDir[4] = { 0.0f, -1.0f, -0.25f, 0.0f }; // direcional
    glLightfv(GL_LIGHT3, GL_POSITION, sunDir);
    glLightfv(GL_LIGHT3, GL_DIFFUSE,  dirDif);
    glLightfv(GL_LIGHT3, GL_AMBIENT,  dirAmb);
    glLightfv(GL_LIGHT3, GL_SPECULAR, spec0);

    // Luz quente no altar
    glEnable(GL_LIGHT4);
    GLfloat pos4[4] = { 0.0f, 5.0f, -23.0f, 1.0f };
    GLfloat dif4[4] = { 0.85f, 0.70f, 0.55f, 1.0f };
    GLfloat amb4[4] = { 0.05f, 0.04f, 0.03f, 1.0f };
    glLightfv(GL_LIGHT4, GL_POSITION, pos4);
    glLightfv(GL_LIGHT4, GL_DIFFUSE,  dif4);
    glLightfv(GL_LIGHT4, GL_AMBIENT,  amb4);
    glLightf (GL_LIGHT4, GL_LINEAR_ATTENUATION, 0.04f);
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
bool checkChairCollision(float x, float z){
    const float startZ = 8.0f;
    const float endZ   = -16.0f;
    const float pitchZ = 2.5f;
    const float baseX  = 2.5f;   
    const float pitchX = 1.0f;  
    const int   cols   = 4;      
    const float chairRadius = 0.35f; 
    const float sumR2 = (RADIUS + chairRadius) * (RADIUS + chairRadius);

    int idx = (int)std::round((startZ - z) / pitchZ);
    int minIdx = std::max(idx - 2, 0);
    int maxIdx = std::min(idx + 2, (int)std::floor((startZ - endZ) / pitchZ));

    for (int i = minIdx; i <= maxIdx; ++i){
        float zChair = startZ - i * pitchZ;
        if (zChair < endZ || zChair > startZ) continue;

        for(int c=0;c<cols;++c){
            float off = pitchX * c;
            float chairXR =  baseX + off;  
            float chairXL = -baseX - off;  

            float dx = x - chairXR;
            float dz = z - zChair;
            if (dx*dx + dz*dz < sumR2) return true;

            dx = x - chairXL; dz = z - zChair;
            if (dx*dx + dz*dz < sumR2) return true;
        }
    }
    return false;
}

// Função para verificar colisão com objetos do altar
bool checkAltarCollision(float x, float z){
    if (x >= -2.5f && x <= 2.5f && z >= -23.5f && z <= -21.5f) return true;
    
    if (x >= -2.9f && x <= -2.1f && z >= -18.3f && z <= -17.7f) return true;
    
    if (x >= -4.9f && x <= -4.1f && z >= -20.2f && z <= -19.8f) return true; // esquerda
    if (x >= 4.1f && x <= 4.9f && z >= -20.2f && z <= -19.8f) return true;  // direita

    if (x >= 2.35f && x <= 2.65f && z >= -21.15f && z <= -20.85f) return true;
    
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
    
    if (ty < EYE_H) ty = EYE_H;
    
    // Colisão com paredes laterais
    if (oz>backZ-RADIUS && oz<frontZ+RADIUS){
        if (tx<-halfW+RADIUS) tx=-halfW+RADIUS;
        if (tx> halfW-RADIUS) tx= halfW-RADIUS;
    }
    
    // Colisão com paredes frontal e traseira
    if (tx > -halfW-RADIUS && tx < halfW+RADIUS){
        // parede de trás
        if (tz < backZ + RADIUS) tz = backZ + RADIUS;

        // Porta removida: passagem sempre liberada no vão central
        bool bloqueia = (std::fabs(tx) > DOOR_HALF);

        if (bloqueia) {
            const float eps = 0.0005f;
            bool estavaFora   = (oz >= frontZ + RADIUS - eps);
            bool estavaDentro = (oz <= frontZ - RADIUS + eps);

            if (estavaFora) {
                if (tz < frontZ + RADIUS) tz = frontZ + RADIUS;
            } else if (estavaDentro) {
                if (tz > frontZ - RADIUS) tz = frontZ - RADIUS;
            } else {
                if (tz < frontZ) tz = frontZ - RADIUS;
                else             tz = frontZ + RADIUS;
            }
        }
    }
    
    if (checkChairCollision(tx, tz) || checkAltarCollision(tx, tz)) {
        float tryX = tx, tryZ = oz;
        if (!checkChairCollision(tryX, tryZ) && !checkAltarCollision(tryX, tryZ)) {
            tz = tryZ;
        } else {
            tryX = ox; tryZ = tz;
            if (!checkChairCollision(tryX, tryZ) && !checkAltarCollision(tryX, tryZ)) {
                tx = tryX;
            } else {
                tx = ox; tz = oz;
            }
        }
    }
    
    nx=tx; ny=ty; nz=tz;
}

//================== OVERLAY 2D (MIRA) ==================
void drawCrosshair(){
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    int cx = WIN_W / 2;
    int cy = WIN_H / 2;

    glLineWidth(2.0f);
    glColor3f(1.0f, 1.0f, 1.0f); 
    glBegin(GL_LINES);
        glVertex2i(cx - 8, cy); glVertex2i(cx + 8, cy); 
        glVertex2i(cx, cy - 8); glVertex2i(cx, cy + 8);
    glEnd();

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();              
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();              
    glMatrixMode(GL_MODELVIEW);
}

// (Botão de porta removido)

//================== RENDER LOOP ==================
void display(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    applyCamera();
    updateFlashlight();

    // 1) opacos
    drawChurchOpaque();
    // 2) vitrais
    drawChurchWindows();

    drawCrosshair();

    // botão de porta removido

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
    unsigned char kk = (k>='A' && k<='Z') ? (k-'A'+'a') : k;
    keyDown[kk]=true;
    shiftHeld = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0;

    if(kk==27) std::exit(0);
    else if(kk=='f') flashlightOn=!flashlightOn;
    else if(kk=='v'){
        flyingMode=!flyingMode;
        if(!flyingMode && camY < EYE_H) camY = EYE_H;
    }
    else if(kk=='r'){ camX=0; camY=EYE_H; camZ=27.2f; yawDeg=0; pitchDeg=0; flyingMode=false; doorOpen=false; }
    else if(kk=='m'){ mouseCaptured=!mouseCaptured; if(mouseCaptured) captureMouseCenter(); else glutSetCursor(GLUT_CURSOR_LEFT_ARROW); }
    else if(kk=='e'){ /* tecla E sem efeito para porta removida */ }
}
void keyUpCb(unsigned char k,int,int){
    unsigned char kk = (k>='A' && k<='Z') ? (k-'A'+'a') : k; 
    keyDown[kk]=false;
    shiftHeld = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0;
}

// Callback de mouse: clique no botão da porta
void mouseCb(int button, int state, int x, int y){
    (void)button; (void)state; (void)x; (void)y; // sem interação especial
}
void spDownCb(int k,int,int){ spDown[k]=true; shiftHeld = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0; }
void spUpCb(int k,int,int){ spDown[k]=false; shiftHeld = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0; }

void idle(){
    int now=glutGet(GLUT_ELAPSED_TIME); 
    float dt=(now-lastMs)/1000.0f; 
    lastMs=now;
    
    if (dt > 0.05f) dt = 0.05f;

    // Porta removida: nenhuma animação

    float fx,fy,fz,rx,rz; 
    getLookVectors(fx,fy,fz,rx,rz);
    float mvx=0, mvy=0, mvz=0;
    
    bool anyKeyPressed =
        keyDown['w']||keyDown['W']||keyDown['s']||keyDown['S']||
        keyDown['a']||keyDown['A']||keyDown['d']||keyDown['D']||
        (flyingMode && (keyDown[' ']||keyDown['c']||keyDown['C']));

    glutPostRedisplay();

    if(keyDown['w']||keyDown['W']){ mvx+=fx; mvz+=fz; anyKeyPressed = true; }
    if(keyDown['s']||keyDown['S']){ mvx-=fx; mvz-=fz; anyKeyPressed = true; }
    if(keyDown['a']||keyDown['A']){ mvx-=rx; mvz-=rz; anyKeyPressed = true; }
    if(keyDown['d']||keyDown['D']){ mvx+=rx; mvz+=rz; anyKeyPressed = true; }
    
    if(flyingMode) {
        if(keyDown[' ']) { mvy += 1.0f; anyKeyPressed = true; } 
        if(keyDown['c']||keyDown['C']) { mvy -= 1.0f; anyKeyPressed = true; } 
    }
    
    float len = std::sqrt(mvx*mvx + mvz*mvz);
    if (len > 0.0001f) { mvx/=len; mvz/=len; }

    bool shiftHeld = (spDown[GLUT_KEY_SHIFT_L] || spDown[GLUT_KEY_SHIFT_R]);

    float targetMul = shiftHeld ? 1.8f : 1.0f;
    float k = 1.0f - std::exp(-dt * 12.0f);
    speedMul += (targetMul - speedMul) * k;

    float speed = baseSpeed * speedMul;

    if (!anyKeyPressed) {
        // sem movimento: não atualiza posição (mas já chamamos redisplay)
        return;
    }

    float maxDt = 0.05f;
    float usedDt = (dt > maxDt ? maxDt : dt);
    float oldX = camX, oldY = camY, oldZ = camZ;
    float newX = camX + mvx*speed*usedDt;
    float newY = camY + mvy*speed*usedDt;
    float newZ = camZ + mvz*speed*usedDt;

    collideAndMove(newX,newY,newZ,oldX,oldY,oldZ);
    camX = newX; camY = newY; camZ = newZ;

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

	// Fog leve para profundidade atmosférica
	glEnable(GL_FOG);
	GLfloat fogColor[4] = {0.85f,0.86f,0.88f,1.0f};
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_MODE, GL_EXP2);
	glFogf(GL_FOG_DENSITY, 0.008f);
	glHint(GL_FOG_HINT, GL_NICEST);

	// Texturas procedurais
	glEnable(GL_TEXTURE_2D);
	initTextures();
	glDisable(GL_TEXTURE_2D);
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
    glutMouseFunc(mouseCb);

    lastMs=glutGet(GLUT_ELAPSED_TIME); captureMouseCenter();

    const GLubyte* r=glGetString(GL_RENDERER);
    const GLubyte* v=glGetString(GL_VENDOR);
    const GLubyte* ver=glGetString(GL_VERSION);
    if(r&&v&&ver) std::printf("Renderer: %s\nVendor  : %s\nVersion : %s\n", r,v,ver);

    glutMainLoop(); return 0;
}