/**
Laboratorio FCG - Pratica 4
Ultima modificacao: 09/05/2016 - Acompanha bibliotecas: bitmap.c(.h), glm.cpp,
glmimg.cpp, Texture.cpp(Texture.h)

Implementa a movimentação simples de uma camera em primeira pessoa para um personagem que anda
sobre um plano. Utiliza a biblioteca bitmap para carregamento simples de texturas, glm
para modelos OBJ e glmimg + Texture para aplicacao de texturas TGA sobre os modelos carregados

Tarefas:

1 - Adicionar neblina à cena ao implementar a função enableFog() com os parâmetros de habilitação de fog em OpenGL;
2 - Usar uma imagem no formato BMP como mapa pra posicionar objetos 3D na cena. Use a cor de cada pixel para definir
qual modelo será colocado e a posição do pixel para definir a posição do modelo no espaço. Pixels pretos não representam
nenhum modelo, fazendo com que a posição fique vazia na cena;
3 - Mudar as configurações da fonte de luz da cena, já implementada no código base dado, para que
seja uma spotlight;
4 - Adicionar uma segunda fonte de luz que fique girando ao redor do cenário.
*/

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cmath>
#include <iostream>
#include <gl/glut.h>

#include "glm.h"

//openal (sound lib)
#include <al/alut.h>

//bitmap class to load bitmaps for textures
#include "bitmap.h"

#pragma comment(lib, "OpenAL32.lib")
#pragma comment(lib, "alut.lib")

#define PI 3.14159265

// sound stuff
#define NUM_BUFFERS 1
#define NUM_SOURCES 1
#define NUM_ENVIRONMENTS 1

#define SMOOTH 0
#define SMOOTH_MATERIAL 1
#define SMOOTH_MATERIAL_TEXTURE 2

#define FALSE 0
#define TRUE 1

#define ESCALA 0.5

#define MORTO 0
#define VIVO 1

#define DIRECAO_NORTE 0
#define DIRECAO_LESTE 1
#define DIRECAO_SUL 2
#define DIRECAO_OESTE 3

#define MIN_MOVIMENTOS_DIRECAO 5
#define MIN_DISTANCIA_PERSEGUICAO 4

// Constantes do mapa
#define VAZIO 0
#define BLOCO 1
#define RACHADURA 2
#define BURACO 3
#define INIMIGO 4
#define JOGADOR 5

#define GRUPO1 1
#define GRUPO2 2

#define COR_VAZIO_R 255
#define COR_VAZIO_G 255
#define COR_VAZIO_B 255

#define COR_BLOCO_R 0
#define COR_BLOCO_G 255
#define COR_BLOCO_B 0

#define COR_RACHADURA_R 128
#define COR_RACHADURA_G 64
#define COR_RACHADURA_B 64

#define COR_BURACO_R 128
#define COR_BURACO_G 0
#define COR_BURACO_B 0

#define COR_INIMIGO_R 255
#define COR_INIMIGO_G 0
#define COR_INIMIGO_B 0

#define COR_JOGADOR_R 0
#define COR_JOGADOR_G 0
#define COR_JOGADOR_B 255

#define MAXIMO_INIMIGOS 10
#define MAXIMO_TEXTURAS 5
#define TEXTURA_GRAMA 1
#define TEXTURA_AGUA 2
#define TEXTURA_BURACO 3
#define TEXTURA_RACHADURA 4

#define JOGO_1P 1
#define JOGO_2D 2
#define JOGO_3P 3

void mainInit();
void initSound();
void initTexture(char *textureName, int texturaValor);
void initMap();
void debugMap();
void initModel();
void initLight();
void enableFog();
void createGLUI();
void mainRender();
void mainCreateMenu();
void onMouseButton(int button, int state, int x, int y);
void onMouseMove(int x, int y);
void onKeyDown(unsigned char key, int x, int y);
void onKeyUp(unsigned char key, int x, int y);
void onGLUIEvent(int id);
void onWindowReshape(int x, int y);
void mainIdle();
int main(int argc, char **argv);
void setWindow();
void setViewport(GLint left, GLint right, GLint bottom, GLint top);
void updateState();
void renderFloor();
void updateCam();

int acaoCriarRachadura();
void gerenciarColisao(int posicaoCenario, int posicaoElemento);
void resetStage();
int floodFill(int x , int y, int grupoAlvo, int grupoNovo );
void gerenciarMapa();
void desmoronarMapa(int grupo);
int perseguirJogador(int inimigoIndice);

/**
Screen dimensions
*/
int windowWidth = 600;
int windowHeight = 480;

/**
Screen position
*/
int windowXPos = 100;
int windowYPos = 150;

int mainWindowId = 0;

double xOffset = -1.9;
double yOffset = -1.3;
int mouseLastX = 0;
int mouseLastY = 0;

float roty = 0.0f;
float rotx = 90.0f;

bool rightPressed = false;
bool leftPressed = false;
bool upPressed = false;
bool downPressed = false;

bool spacePressed = false;

float speedX = 0.0f;
float speedY = 0.0f;
float speedZ = 0.0f;

float posX = 0.0f;
float posY = 0.0f;
float posZ = 2.0f;
float nextPosX = 0.0f;
float nextPosZ = 2.0f;
float initialPosX = 0.0f;
float initialPosZ = 2.0f;

int direcao = DIRECAO_NORTE;

int modoJogo = JOGO_2D;

/*
variavel auxiliar pra dar variação na altura do ponto de vista ao andar.
*/
float headPosAux = 0.0f;

float maxSpeed = 0.25f;
float maxInimigoSpeed = 0.05f;

float planeSize = 20.0f;

// more sound stuff (position, speed and orientation of the listener)
ALfloat listenerPos[]={0.0,0.0,4.0};
ALfloat listenerVel[]={0.0,0.0,0.0};
ALfloat listenerOri[]={0.0,0.0,1.0,
						0.0,1.0,0.0};

// now the position and speed of the sound source
ALfloat source0Pos[]={ -2.0, 0.0, 0.0};
ALfloat source0Vel[]={ 0.0, 0.0, 0.0};

// buffers for openal stuff
ALuint  buffer[NUM_BUFFERS];
ALuint  source[NUM_SOURCES];
ALuint  environment[NUM_ENVIRONMENTS];
ALsizei size,freq;
ALenum  format;
ALvoid  *data;



// parte de código extraído de "texture.c" por Michael Sweet (OpenGL SuperBible)
// texture buffers and stuff
int i;                       /* Looping var */
BITMAPINFO	*info, *infoSuperior, *infoInferior;           /* Bitmap information */
GLubyte	    *bits, *bitsSuperior, *bitsInferior;           /* Bitmap RGB pixels */
GLubyte     *ptr;            /* Pointer into bit buffer */
GLubyte	    *rgba;           /* RGBA pixel buffer */
GLubyte	    *rgbaptr;        /* Pointer into RGBA buffer */
GLubyte     temp;            /* Swapping variable */
GLenum      type;            /* Texture type */
GLuint      texture[MAXIMO_TEXTURAS];         /* Texture object */
int quantidade_texturas = -1;


bool crouched = false;
bool running = false;
float jumpSpeed = 0.06;
float gravity = 0.004;
float heightLimit = 0.2;
float posYOffset = 0.2;

float backgrundColor[4] = {0.0f,0.0f,0.0f,1.0f};

GLMmodel *modelSphere;


int mapaElementos[20][20];
int mapaCenario[20][20];
int mapaGrupo[20][20];
int jPrimeiroQuadrado = -1;
int kPrimeiroQuadrado = -1;

typedef struct inimigo {
    float x;
    float y;
    float z;
    int direcao;
    int estado;
    int qntTurnoDirecao;
    GLMmodel *modelInimigo;
} inimigo_t;

inimigo_t *inimigo[MAXIMO_INIMIGOS];
int quantidade_inimigos = -1;

// Aux function to load the object using GLM and apply some functions
bool C3DObject_Load_New(const char *pszFilename, GLMmodel **model){
    char aszFilename[256];
    strcpy(aszFilename, pszFilename);

    if (*model) {

    free(*model);
    *model = NULL;
    }

    *model = glmReadOBJ(aszFilename);
    if (!(*model))
    return false;

    glmUnitize(*model);
    glmScale(*model,ESCALA); // USED TO SCALE THE OBJECT
    glmFacetNormals(*model);
    glmVertexNormals(*model, 90.0);

    return true;
}

void setWindow() {

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f,(GLfloat)windowWidth/(GLfloat)windowHeight,0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(posX,posY + posYOffset + 0.025 * std::abs(sin(headPosAux*PI/180)),posZ,
		posX + sin(roty*PI/180),posY + posYOffset + 0.025 * std::abs(sin(headPosAux*PI/180)) + cos(rotx*PI/180),posZ -cos(roty*PI/180),
		0.0,1.0,0.0);
}

/**
Atualiza a posição e orientação da camera
*/
void updateCam() {

    if(modoJogo == JOGO_1P){
        gluLookAt(posX,posY+2,posZ+1,
            posX + sin(roty*PI/180),posY+1+ 0.025 * std::abs(sin(headPosAux*PI/180)) + cos(rotx*PI/180),posZ -cos(roty*PI/180),
            0.0,1.0,0.0);
    }
    if(modoJogo == JOGO_2D){
        gluLookAt(posX,posY+15,posZ,
            posX + sin(roty*PI/180),posY+ 0.025 * std::abs(sin(headPosAux*PI/180)) + cos(rotx*PI/180),posZ -cos(roty*PI/180),
            0.0,1.0,0.0);
    }
    if(modoJogo == JOGO_3P){
        gluLookAt(posX,posY+10,posZ+7,
            posX + sin(roty*PI/180),posY+ 0.025 * std::abs(sin(headPosAux*PI/180)) + cos(rotx*PI/180),posZ -cos(roty*PI/180),
            0.0,1.0,0.0);
    }


	// atualiza a posição do listener e da origen do som, são as mesmas da camera, já que os passos vem de onde o personagem está
	listenerPos[0] = posX;
	listenerPos[1] = posY;
	listenerPos[2] = posZ;
	source0Pos[0] = posX;
	source0Pos[1] = posY;
	source0Pos[2] = posZ;

    GLfloat light_position1[] = {posX, posY, posZ, 1.0 };
    //glLightfv(GL_LIGHT0, GL_POSITION, light_position1);


}

void initLight() {
    glEnable(GL_LIGHTING );
	glEnable( GL_LIGHT0 );

	GLfloat light_ambient[] = { backgrundColor[0], backgrundColor[1], backgrundColor[2], backgrundColor[3] };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_position1[] = {0.0, 0.0, 0.0, 1.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position1);

}

void setViewport(GLint left, GLint right, GLint bottom, GLint top) {
	glViewport(left, bottom, right - left, top - bottom);
}


/**
Initialize
*/
void mainInit() {
	glClearColor(1.0,1.0,1.0,0.0);
	glColor3f(0.0f,0.0f,0.0f);
	setWindow();
	setViewport(0, windowWidth, 0, windowHeight);

	// habilita o z-buffer
	glEnable(GL_DEPTH_TEST);

    initSound();

    initTexture("textura-espaco.bmp", TEXTURA_AGUA);
    initTexture("textura-nave.bmp", TEXTURA_GRAMA);
    initTexture("textura-buraco-nave.bmp", TEXTURA_BURACO);
    initTexture("textura-rachadura-nave.bmp", TEXTURA_RACHADURA);

    initMap();

    //debugMap();

	initModel();

	initLight();

	enableFog();

	printf("w - andar \n");
	printf("s - ir pra tras \n");
	printf("mouse - direcao \n");
	printf("r - correr \n");
	printf("c - abaixar \n");
	printf("espaco - pular \n");

}

void initModel() {
	printf("Loading models.. \n");

	C3DObject_Load_New("luigi.obj",&modelSphere);

    int i;
    for(i = 0 ; i <= quantidade_inimigos ; i++){
        C3DObject_Load_New("mk_kart.obj",&inimigo[i]->modelInimigo);
    }
	printf("Models ok. \n \n \n");
}

/**
Initialize openal and check for errors
*/
void initSound() {

	printf("Initializing OpenAl \n");

	// Init openAL
	alutInit(0, NULL);

	alGetError(); // clear any error messages

    // Generate buffers, or else no sound will happen!
    alGenBuffers(NUM_BUFFERS, buffer);

    if(alGetError() != AL_NO_ERROR)
    {
        printf("- Error creating buffers !!\n");
        exit(1);
    }
    else
    {
        printf("init() - No errors yet.\n");
    }

	alutLoadWAVFile("Footsteps.wav",&format,&data,&size,&freq,false);
    alBufferData(buffer[0],format,data,size,freq);

	alGetError(); /* clear error */
    alGenSources(NUM_SOURCES, source);

    if(alGetError() != AL_NO_ERROR)
    {
        printf("- Error creating sources !!\n");
        exit(2);
    }
    else
    {
        printf("init - no errors after alGenSources\n");
    }

	listenerPos[0] = posX;
	listenerPos[1] = posY;
	listenerPos[2] = posZ;

	source0Pos[0] = posX;
	source0Pos[1] = posY;
	source0Pos[2] = posZ;

	alListenerfv(AL_POSITION,listenerPos);
    alListenerfv(AL_VELOCITY,listenerVel);
    alListenerfv(AL_ORIENTATION,listenerOri);

	alSourcef(source[0], AL_PITCH, 1.0f);
    alSourcef(source[0], AL_GAIN, 1.0f);
    alSourcefv(source[0], AL_POSITION, source0Pos);
    alSourcefv(source[0], AL_VELOCITY, source0Vel);
    alSourcei(source[0], AL_BUFFER,buffer[0]);
    alSourcei(source[0], AL_LOOPING, AL_TRUE);

	printf("Sound ok! \n\n");
}


void initMap(void){
    printf ("\nLoading map..\n");
    // Load a texture object (256x256 true color)
    bitsInferior = LoadDIBitmap("mapaInferior.bmp", &infoInferior);
    if (bitsInferior == (GLubyte *)0) {
		printf ("Error loading file!\n\n");
		return;
	}

    bitsSuperior = LoadDIBitmap("mapaSuperior.bmp", &infoSuperior);
    if (bitsSuperior == (GLubyte *)0) {
		printf ("Error loading file!\n\n");
		return;
	}

    int iSuperior = infoSuperior->bmiHeader.biWidth * info->bmiHeader.biHeight - 1;
    GLubyte *ptrSuperior = bitsSuperior;

    int iInferior = infoInferior->bmiHeader.biWidth * info->bmiHeader.biHeight - 1;
    GLubyte *ptrInferior = bitsInferior;


    int j,k;
    for(j = 0 ; j < 20 ; j++){
        for(k = 0 ; k < 20 ; k++){
                    mapaElementos[j][k] = VAZIO; // Valor padrao do elemento
                    mapaCenario[j][k] = VAZIO; // Valor padrao do cenario
                    mapaGrupo[j][k] = VAZIO;

                    if( ptrInferior[2] == COR_BLOCO_R && ptrInferior[1] == COR_BLOCO_G && ptrInferior[0] == COR_BLOCO_B){
                        mapaCenario[j][k] = BLOCO;
                        mapaGrupo[j][k] = GRUPO1;
                        if(jPrimeiroQuadrado == -1){ //Primeiro quadrado do mapa que possui bloco
                            jPrimeiroQuadrado = j;
                            kPrimeiroQuadrado = k;
                        }
                    }
                    if( ptrSuperior[2] == COR_RACHADURA_R && ptrSuperior[1] == COR_RACHADURA_G && ptrSuperior[0] == COR_RACHADURA_B){
                        mapaElementos[j][k] = RACHADURA;
                        mapaGrupo[j][k] = VAZIO;
                    }
                    if( ptrSuperior[2] == COR_INIMIGO_R && ptrSuperior[1] == COR_INIMIGO_G && ptrSuperior[0] == COR_INIMIGO_B){
                        mapaElementos[j][k] = INIMIGO; // Apenas precisamos das coordendas, não é necessario setar um elemento no mapa
                        quantidade_inimigos++;
                        inimigo[quantidade_inimigos] = (inimigo_t*)malloc(sizeof(inimigo_t));
                        inimigo[quantidade_inimigos]->modelInimigo = NULL;
                        inimigo[quantidade_inimigos]->x = (j - 10)+0.5;
                        inimigo[quantidade_inimigos]->z = (k - 10)+0.5;
                        inimigo[quantidade_inimigos]->y = 1.0;
                        inimigo[quantidade_inimigos]->direcao = DIRECAO_SUL;
                        inimigo[quantidade_inimigos]->estado = VIVO;
                        inimigo[quantidade_inimigos]->qntTurnoDirecao = MIN_MOVIMENTOS_DIRECAO;
                        //printf("Inimigo: %d \t X: %d \t Z: %d \t J: %d \t K: %d \n",quantidade_inimigos,inimigo[quantidade_inimigos]->x,inimigo[quantidade_inimigos]->z,j,k);
                    }
                    if( ptrSuperior[2] == COR_BURACO_R && ptrSuperior[1] == COR_BURACO_G && ptrSuperior[0] == COR_BURACO_B){
                        mapaElementos[j][k] = BURACO;
                        mapaGrupo[j][k] = VAZIO;
                    }
                    if( ptrSuperior[2] == COR_JOGADOR_R && ptrSuperior[1] == COR_JOGADOR_G && ptrSuperior[0] == COR_JOGADOR_B){
                        mapaElementos[j][k] = VAZIO; // Apenas precisamos das coordendas, não é necessario setar um elemento no mapa
                        initialPosX = nextPosX = posX = (j - 10)+0.5;
                        initialPosZ = nextPosZ = posZ = (k - 10)+0.5;
                    }
                    iSuperior--;
                    iInferior--;
                    ptrSuperior += 3;
                    ptrInferior += 3;
        }
    }
    printf("map ok! \n\n");

}

void debugMap(){
    int j,k;
    for(j = 0 ; j < 20 ; j++){
        for(k = 0 ; k < 20 ; k++){
            if(mapaGrupo[j][k] != VAZIO){
                printf("%d",mapaGrupo[j][k]);
            }
        }
        printf("\n");
    }
    printf("\n");
}

/**
Initialize the texture using the library bitmap
*/
void initTexture(char *textureName, int texturaValor){
    printf ("\nLoading texture..\n");
    // Load a texture object (256x256 true color)
    bits = LoadDIBitmap(textureName, &info);
    if (bits == (GLubyte *)0) {
		printf ("Error loading texture!\n\n");
		return;
	}
    // Figure out the type of texture
    if (info->bmiHeader.biHeight == 1)
      type = GL_TEXTURE_1D;
    else
      type = GL_TEXTURE_2D;

    // Create and bind a texture object
    quantidade_texturas++;

    glGenTextures(2, &texture[quantidade_texturas]);
    texture[quantidade_texturas] = texturaValor;
	glBindTexture(type, texture[quantidade_texturas]);


    // Create an RGBA image
    rgba = (GLubyte *)malloc(info->bmiHeader.biWidth * info->bmiHeader.biHeight * 4);

    i = info->bmiHeader.biWidth * info->bmiHeader.biHeight;
    for( rgbaptr = rgba, ptr = bits;  i > 0; i--, rgbaptr += 4, ptr += 3)
    {
            rgbaptr[0] = ptr[2];     // windows BMP = BGR
            rgbaptr[1] = ptr[1];
            rgbaptr[2] = ptr[0];
            rgbaptr[3] = (ptr[0] + ptr[1] + ptr[2]) / 3;
    }

	// Set texture parameters
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(type, 0, 4, info->bmiHeader.biWidth, info->bmiHeader.biHeight,0, GL_RGBA, GL_UNSIGNED_BYTE, rgba );


    printf("Textura %d\n", texture[quantidade_texturas]);

}

void enableFog(void){

}

void renderFloor(int tipoElemento, int texturaValor, int elementOverFloor) {
    glBindTexture(type, texturaValor);

	// set things up to render the floor with the texture
	glShadeModel(GL_SMOOTH);
	glEnable(type);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glPushMatrix();

    glTranslatef(-(float)planeSize/2.0f, 0.0f, -(float)planeSize/2.0f);

	float textureScaleX = 10.0;
	float textureScaleY = 10.0;
    glColor4f(1.0f,1.0f,1.0f,1.0f);
    int xQuads = 20;
    int zQuads = 20;
    for (int i = 0; i < xQuads; i++) {
        for (int j = 0; j < zQuads; j++) {

            if(elementOverFloor == TRUE){
                if(mapaElementos[i][j] != tipoElemento) continue;
            }else{
                if(mapaCenario[i][j] != tipoElemento || mapaElementos[i][j] != VAZIO) continue;
            }

            glBegin(GL_QUADS);
                glTexCoord2f(1.0f, 0.0f);   // coords for the texture
                glNormal3f(0.0f,1.0f,0.0f);
                glVertex3f(i * (float)planeSize/xQuads, 0.0f, (j+1) * (float)planeSize/zQuads);

                glTexCoord2f(0.0f, 0.0f);  // coords for the texture
                glNormal3f(0.0f,1.0f,0.0f);
                glVertex3f((i+1) * (float)planeSize/xQuads, 0.0f, (j+1) * (float)planeSize/zQuads);

                glTexCoord2f(0.0f, 1.0f);  // coords for the texture
                glNormal3f(0.0f,1.0f,0.0f);
                glVertex3f((i+1) * (float)planeSize/xQuads, 0.0f, j * (float)planeSize/zQuads);

                glTexCoord2f(1.0f, 1.0f);  // coords for the texture
                glNormal3f(0.0f,1.0f,0.0f);
                glVertex3f(i * (float)planeSize/xQuads, 0.0f, j * (float)planeSize/zQuads);

            glEnd();
        }
    }

	glDisable(type);

	glPopMatrix();
}

void renderScene() {
	glClearColor(backgrundColor[0],backgrundColor[1],backgrundColor[2],backgrundColor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // limpar o depth buffer

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	updateCam();

    glPushMatrix();
        glTranslatef(posX,1.0,posZ);
        glRotatef((180 - (direcao*90)),0,1,0);
        glmDraw(modelSphere, GLM_SMOOTH | GLM_MATERIAL | GLM_TEXTURE);
	glPopMatrix();

    int i;
    for(i = 0 ; i <= quantidade_inimigos ; i++){
        glPushMatrix();
            glTranslatef(inimigo[i]->x,inimigo[i]->y,inimigo[i]->z);
            glRotatef((180 - (inimigo[i]->direcao*90)),0,1,0);
            glmDraw(inimigo[i]->modelInimigo, GLM_SMOOTH | GLM_MATERIAL | GLM_TEXTURE);
        glPopMatrix();
    }

    // binds the bmp file already loaded to the OpenGL parameters

	renderFloor(BLOCO, TEXTURA_GRAMA, FALSE);
	renderFloor(VAZIO, TEXTURA_AGUA, FALSE);
	renderFloor(INIMIGO, TEXTURA_GRAMA, TRUE);
	renderFloor(JOGADOR, TEXTURA_GRAMA, TRUE);
	renderFloor(BURACO, TEXTURA_BURACO, TRUE);
    renderFloor(RACHADURA, TEXTURA_RACHADURA, TRUE);
}

void updateInimigoState(){
    int i, j, k, l, novaDirecao, movimentoInvalido = FALSE;
    float iSpeedX, iSpeedZ;

    for(i = 0 ; i <= quantidade_inimigos; i++){
        movimentoInvalido = FALSE;
        if(inimigo[i]->estado == MORTO){
            if(inimigo[i]->y < 20){
                inimigo[i]->y += 0.5;
            }
            continue;
        }

        // Verifica se o inimigo foi morto
        j = inimigo[i]->x + 10;
        k = inimigo[i]->z + 10;
        if(mapaCenario[j][k] == VAZIO){
            inimigo[i]->estado = MORTO;
            continue;
        }


        if(abs(j - (int)(posX+10)) < MIN_DISTANCIA_PERSEGUICAO && abs(k - (int)(posZ+10)) < MIN_DISTANCIA_PERSEGUICAO){
            novaDirecao = perseguirJogador(i);
        }else{
            inimigo[i]->qntTurnoDirecao--;
            if(inimigo[i]->qntTurnoDirecao > 0){
                novaDirecao = inimigo[i]->direcao;
            }else{
                novaDirecao = rand() % 4;
                inimigo[i]->qntTurnoDirecao = MIN_MOVIMENTOS_DIRECAO + rand() % 10;
            }
        }


        iSpeedX = iSpeedZ = 0.0f;

        inimigo[i]->direcao = novaDirecao;
        if(inimigo[i]->direcao == DIRECAO_NORTE) iSpeedZ = -maxInimigoSpeed;
        if(inimigo[i]->direcao == DIRECAO_SUL) iSpeedZ = maxInimigoSpeed;
        if(inimigo[i]->direcao == DIRECAO_OESTE) iSpeedX = -maxInimigoSpeed;
        if(inimigo[i]->direcao == DIRECAO_LESTE) iSpeedX = maxInimigoSpeed;


        // Verifica se a nova posicao calculada é valida
        j = inimigo[i]->x + iSpeedX + 10;
        k = inimigo[i]->z + iSpeedZ + 10;
        if(mapaCenario[j][k] == VAZIO || mapaElementos[j][k] == BURACO || mapaElementos[j][k] == RACHADURA){
            continue;
        }
        for(l = 0 ; l <= quantidade_inimigos; l++){
            //printf("I: %d \t %d \t %d \t L: %d \t %d \t %d \n",i,(int)inimigo[i]->x,(int)inimigo[i]->z,l,(int)inimigo[l]->x,(int)inimigo[l]->z);
            if( i != l && (int)(inimigo[i]->x + iSpeedX) == (int)inimigo[l]->x && (int)(inimigo[i]->z + iSpeedZ) == (int)inimigo[l]->z ){
                movimentoInvalido = TRUE;
            }
        }
        if(movimentoInvalido == TRUE){
            printf("Invalido\n");
            continue;
        }


        // Remove referencia do INIMIGO da posicao anterior
        j = inimigo[i]->x + 10;
        k = inimigo[i]->z + 10;
        if(mapaElementos[j][k] == INIMIGO){
            mapaElementos[j][k] = VAZIO;
        }
        //////


        inimigo[i]->x += iSpeedX;
        inimigo[i]->z += iSpeedZ;

        j = inimigo[i]->x + 10;
        k = inimigo[i]->z + 10;
        mapaElementos[j][k] = INIMIGO; // Posiciona o INIMIGO na nova posicao
        if(j == (int)(posX+10) && k == (int)(posZ+10)){ // Se o INIMIGO atingiiu o JOGADOR
            gerenciarColisao(mapaCenario[j][k],mapaElementos[j][k]);
        }
    }

}

double distanciaPontos(float x1, float z1, float x2, float z2){
    double d;
    d = sqrt(pow((x2 - x1),2)+pow((z2 - z1),2));
    return d;
}

int perseguirJogador (int inimigoIndice){
    int melhorDirecao, novaDirecao, j, k;
    float melhorDistancia, novaDistancia, novoX, novoZ, iSpeedX, iSpeedZ;

    melhorDistancia = distanciaPontos(inimigo[inimigoIndice]->x,inimigo[inimigoIndice]->z,posX,posZ);
    melhorDirecao = inimigo[inimigoIndice]->direcao;

    for(novaDirecao = 0; novaDirecao < 4; novaDirecao++){
        iSpeedX = iSpeedZ = 0.0f;

        if(novaDirecao == DIRECAO_NORTE) iSpeedZ = -maxInimigoSpeed;
        if(novaDirecao == DIRECAO_SUL) iSpeedZ = maxInimigoSpeed;
        if(novaDirecao == DIRECAO_OESTE) iSpeedX = -maxInimigoSpeed;
        if(novaDirecao == DIRECAO_LESTE) iSpeedX = maxInimigoSpeed;


        // Verifica se a nova posicao calculada é valida
        j = inimigo[inimigoIndice]->x + iSpeedX + 10;
        k = inimigo[inimigoIndice]->z + iSpeedZ + 10;
        if(mapaCenario[j][k] == VAZIO || mapaElementos[j][k] == BURACO || mapaElementos[j][k] == RACHADURA){
            continue;
        }

        novoX = inimigo[inimigoIndice]->x + iSpeedX;
        novoZ = inimigo[inimigoIndice]->z + iSpeedZ;
        //printf("Direcao %d: \t %f \t %f \t %f \n",novaDirecao,novoX, novoZ, distanciaPontos(novoX,novoZ,posX,posZ));
        novaDistancia = distanciaPontos(novoX,novoZ,posX,posZ);
        if(novaDistancia < melhorDistancia){
            melhorDistancia = novaDistancia;
            melhorDirecao = novaDirecao;
        }
    }

    return melhorDirecao;
}



void updateState() {

    if (leftPressed) {
        leftPressed = false; // Simula um "delay" pois a rotacao estava muito rapida.

        if(modoJogo == JOGO_1P || modoJogo == JOGO_3P) {
            roty -= 90.0f;
        }

        if(direcao == DIRECAO_NORTE) direcao = DIRECAO_OESTE;
        else direcao--;
	}

	if (rightPressed) {
        rightPressed = false; // Simula um "delay" pois a rotacao estava muito rapida.

        if(modoJogo == JOGO_1P || modoJogo == JOGO_3P) {
            roty += 90.0f;
        }

        if(direcao == DIRECAO_OESTE) direcao = DIRECAO_NORTE;
        else direcao++;
	}

    //printf("Direcao: %d", direcao);

	if (upPressed || downPressed) {

        /*if(modoJogo == JOGO_1P || modoJogo == JOGO_3P) {
            speedX = maxSpeed * sin(roty*PI/180);
            speedZ = -maxSpeed * cos(roty*PI/180);
        }else{ //Se JOGO_2D*/

            if(direcao == DIRECAO_NORTE){
                speedX = 0;
                speedZ = -maxSpeed * cos(roty*PI/180);
            }

            if(direcao == DIRECAO_SUL){
                speedX = 0;
                speedZ = maxSpeed * cos(roty*PI/180);
            }

            if(direcao == DIRECAO_LESTE){
                speedX = maxSpeed * cos(roty*PI/180);
                speedZ = 0;
            }

            if(direcao == DIRECAO_OESTE){
                speedX = -maxSpeed * cos(roty*PI/180);
                speedZ = 0;
            }

        //}

		// efeito de "sobe e desce" ao andar
		headPosAux += 8.5f;
		if (headPosAux > 180.0f) {
			headPosAux = 0.0f;
		}

        if (upPressed) {
            nextPosX += speedX;
            nextPosZ += speedZ;
        } else {
            nextPosX -= speedX;
            nextPosZ -= speedZ;
        }

        int j = nextPosX + 10;
        int k = nextPosZ + 10;
        if(mapaCenario[j][k] == BLOCO){
            //posX = floor(nextPosX)+0.5;
            posX = nextPosX;
            //posZ = floor(nextPosZ)+0.5;
            posZ = nextPosZ;
        }else{
            gerenciarColisao(mapaCenario[j][k], mapaElementos[j][k]);
            nextPosX = posX;
            nextPosZ = posZ;
        }

	} else {
		// parou de andar, para com o efeito de "sobe e desce"
		headPosAux = fmod(headPosAux, 90) - 1 * headPosAux / 90;
		headPosAux -= 4.0f;
		if (headPosAux < 0.0f) {
			headPosAux = 0.0f;
		}
	}

	posY += speedY;
	if (posY < heightLimit) {
		posY = heightLimit;
		speedY = 0.0f;
	} else {
		speedY -= gravity;
	}

}

/**
Render scene
*/
void mainRender() {
	updateState();
	updateInimigoState();
	renderScene();
	glFlush();
	glutPostRedisplay();
	Sleep(30);
}

/**
Handles events from the mouse right button menu
*/
void mainHandleMouseRightButtonMenuEvent(int option) {
	switch (option) {
		case 1 :
			exit(0);
			break;
		default:
			break;
	}
}

/**
Create mouse button menu
*/
void mainCreateMenu() {
	glutCreateMenu(mainHandleMouseRightButtonMenuEvent);
	glutAddMenuEntry("Quit", 1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

/**
Mouse button event handler
*/
void onMouseButton(int button, int state, int x, int y) {
	//printf("onMouseButton button: %d \n", button);
	glutPostRedisplay();
}

/**
Mouse move while button pressed event handler
*/
void onMouseMove(int x, int y) {

	/*mouseLastX = x;
	mouseLastY = y;*/

	glutPostRedisplay();
}

/**
Mouse move with no button pressed event handler
*/
void onMousePassiveMove(int x, int y) {
/*
	roty += (x - mouseLastX);

	rotx -= (y - mouseLastY);

	if (rotx < -128.0) {
		rotx = -128.0;
	}

	if (rotx > -45.0) {
		rotx = -45.0;
	}

	mouseLastX = x;
	mouseLastY = y;
*/
	//glutPostRedisplay();
}

/**
Key press event handler
*/
void onKeyDown(unsigned char key, int x, int y) {
	//printf("%d \n", key);
	switch (key) {
		case 32: //space
			spacePressed = true;
			acaoCriarRachadura();
			break;
		case 119: //w
        case 87: //W
			if (!upPressed) {
				alSourcePlay(source[0]);
			}
			upPressed = true;
			break;
		case 115: //s
        case 83: //S
			downPressed = true;
			break;
		case 97: //a
		case 65: //A
			leftPressed = true;
			break;
		case 100: //d
		case 68: //D
			rightPressed = true;
			break;
		case 99: //c
			//crouched = true;
			break;
		case 114: //r
			running = true;
			break;
		case 118: //v
        case 86: //V
            switch(modoJogo){
                case JOGO_1P:
                    modoJogo = JOGO_2D;
                    break;
                case JOGO_2D:
                    modoJogo = JOGO_3P;
                    break;
                case JOGO_3P:
                    modoJogo = JOGO_1P;
                    break;
            }

		default:
			break;
	}

	int j = posX+10;
	int k = posZ+10;

}

/**
Key release event handler
*/
void onKeyUp(unsigned char key, int x, int y) {
	switch (key) {
		case 32: //space
			spacePressed = false;
			break;
		case 119: //w
        case 87: //W
			if (upPressed) {
				alSourceStop(source[0]);
			}
			upPressed = false;
			break;
		case 115: //s
        case 83: //S
			downPressed = false;
			break;
		case 97: //a
		case 65: //A
			leftPressed = false;
			break;
		case 100: //d
		case 68: //D
			rightPressed = false;
			break;
		case 99: //c
			//crouched = false;
			break;
		case 114: //r
			running = false;
			break;
		case 27:
			exit(0);
			break;
		default:
			break;
	}

	//glutPostRedisplay();
}


int acaoCriarRachadura(){
	int jBuraco = posX+10;
	int kBuraco = posZ+10;
	if(mapaElementos[jBuraco][kBuraco] != BURACO){
        return FALSE;
	}

    int direcaoRachadura = direcao;
    int rSpeedX, rSpeedZ;
    rSpeedX = rSpeedZ = 0;

    if(direcaoRachadura == DIRECAO_NORTE) rSpeedZ--;
    if(direcaoRachadura == DIRECAO_SUL) rSpeedZ++;
    if(direcaoRachadura == DIRECAO_LESTE) rSpeedX++;
    if(direcaoRachadura == DIRECAO_OESTE) rSpeedX--;

    int j = jBuraco;
    int k = kBuraco;
    while (mapaCenario[j][k] == BLOCO){
        j += rSpeedX;
        k += rSpeedZ;
        if(mapaCenario[j][k] == VAZIO || mapaElementos[j][k] == BURACO) {
            break;
        }
        mapaElementos[j][k] = RACHADURA;
        mapaGrupo[j][k] = VAZIO;

    }

    gerenciarMapa();

}

void gerenciarMapa(){

    if(mapaGrupo[jPrimeiroQuadrado][kPrimeiroQuadrado] == GRUPO1){
        floodFill(jPrimeiroQuadrado, kPrimeiroQuadrado, GRUPO1, GRUPO2);
    }else{
        floodFill(jPrimeiroQuadrado, kPrimeiroQuadrado, GRUPO2, GRUPO1);
    }

    debugMap();

    int j, k, jPrimeiroGrupo1, kPrimeiroGrupo1, jPrimeiroGrupo2, kPrimeiroGrupo2, grupo1, grupo2;
    jPrimeiroGrupo1 = kPrimeiroGrupo1 = jPrimeiroGrupo2 = kPrimeiroGrupo2 = -1;
    grupo1 = grupo2 = 0;
    for(j = 0 ; j < 20 ; j++){
        for(k = 0 ; k < 20 ; k++){
            if(mapaGrupo[j][k] == GRUPO1){
                grupo1++;
                if(jPrimeiroGrupo1 == -1){
                    jPrimeiroGrupo1 = j;
                    kPrimeiroGrupo1 = k;
                }
            }
            if(mapaGrupo[j][k] == GRUPO2){
                grupo2++;
                if(jPrimeiroGrupo2 == -1){
                    jPrimeiroGrupo2 = j;
                    kPrimeiroGrupo2 = k;
                }
            }
        }
    }

    if(grupo1 > 0 && grupo2 > 0){
        if(grupo1 < grupo2){
            jPrimeiroQuadrado = jPrimeiroGrupo2;
            kPrimeiroQuadrado = kPrimeiroGrupo2;
            desmoronarMapa(GRUPO1);
        }else{
            jPrimeiroQuadrado = jPrimeiroGrupo1;
            kPrimeiroQuadrado = kPrimeiroGrupo1;
            desmoronarMapa(GRUPO2);
        }
    }
}

void desmoronarMapa(int grupo){
    int j,k;
    for(j = 0 ; j < 20 ; j++){
        for(k = 0 ; k < 20 ; k++){
            if(mapaGrupo[j][k] == grupo){
                mapaCenario[j][k] = VAZIO;
                mapaElementos[j][k] = VAZIO;
                mapaGrupo[j][k] = VAZIO;
            }
        }

    }

    for(j = 0 ; j < 20 ; j++){
        for(k = 0 ; k < 20 ; k++){
            if(mapaElementos[j][k] == RACHADURA || mapaElementos[j][k] == BURACO){
                if(mapaCenario[j - 1][k] == VAZIO && mapaCenario[j + 1][k] == VAZIO){
                    mapaCenario[j][k] = VAZIO;
                    mapaElementos[j][k] = VAZIO;
                    mapaGrupo[j][k] = VAZIO;
                }
                if(mapaCenario[j][k - 1] == VAZIO && mapaCenario[j][k + 1] == VAZIO){
                    mapaCenario[j][k] = VAZIO;
                    mapaElementos[j][k] = VAZIO;
                    mapaGrupo[j][k] = VAZIO;
                }
            }
        }
    }

}

void gerenciarColisao(int posicaoCenario, int posicaoElemento){
    //printf("Colisao \t\t Cenario: %d \t Elemento: %d \n",posicaoCenario, posicaoElemento);
    if(posicaoCenario == VAZIO || posicaoElemento == INIMIGO){
        resetStage();
    }

}

void resetStage(){
    nextPosX = posX = initialPosX;
    nextPosZ = posZ = initialPosZ;
}

int floodFill(int x , int y, int grupoAlvo, int grupoNovo ){
    if(grupoAlvo == grupoNovo) return 0;
    if(mapaGrupo[x][y] != grupoAlvo) return 0;
    mapaGrupo[x][y] = grupoNovo;
    if(mapaGrupo[x-1][y] != VAZIO) floodFill(x-1, y,   grupoAlvo, grupoNovo);
    if(mapaGrupo[x+1][y] != VAZIO) floodFill(x+1, y,   grupoAlvo, grupoNovo);
    if(mapaGrupo[x][y-1] != VAZIO) floodFill(x,   y-1, grupoAlvo, grupoNovo);
    if(mapaGrupo[x][y+1] != VAZIO) floodFill(x,   y+1, grupoAlvo, grupoNovo);
    return 0;
}


void onWindowReshape(int x, int y) {
	windowWidth = x;
	windowHeight = y;
	setWindow();
	setViewport(0, windowWidth, 0, windowHeight);
}

/**
Glut idle funtion
*/
void mainIdle() {
	/**
	Set the active window before send an glutPostRedisplay call
	so it wont be accidently sent to the glui window
	*/

	glutSetWindow(mainWindowId);
	glutPostRedisplay();
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(windowWidth,windowHeight);
	glutInitWindowPosition(windowXPos,windowYPos);

	/**
	Store main window id so that glui can send it redisplay events
	*/
	mainWindowId = glutCreateWindow("FPS");

	glutDisplayFunc(mainRender);

	glutReshapeFunc(onWindowReshape);

	/**
	Register mouse events handlers
	*/
	glutMouseFunc(onMouseButton);
	glutMotionFunc(onMouseMove);
	glutPassiveMotionFunc(onMousePassiveMove);

	/**
	Register keyboard events handlers
	*/
	glutKeyboardFunc(onKeyDown);
	glutKeyboardUpFunc(onKeyUp);

	mainInit();

	glutMainLoop();

    return 0;
}
