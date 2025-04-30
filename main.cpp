/*

g++ -o main.exe main.cpp -I"./include" -L"./lib/x64" -lfreeglut -lopengl32 -lglu32
.\main.exe

*/

// Estrutura para representar um segmento do braço robótico
typedef struct
{
	float comprimento; // Comprimento do segmento
	float largura;	   // Largura do segmento
	float altura;	   // Altura do segmento
	float anguloX;	   // Ângulo de rotação no eixo X
	float anguloY;	   // Ângulo de rotação no eixo Y
	float anguloZ;	   // Ângulo de rotação no eixo Z
	float cor[3];	   // Cor RGB do segmento
} SegmentoRobo;

// Estrutura para representar a garra
typedef struct
{
	float abertura;	   // Ângulo de abertura da garra
	float comprimento; // Comprimento da garra
	float largura;	   // Largura da garra
	float cor[3];	   // Cor RGB da garra
} Garra;

#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#define SENS_ROT 5.0
#define SENS_OBS 15.0
#define SENS_TRANSL 30.0

// Declaração de variáveis globais
GLfloat tx = 0, ty = 0;
GLfloat win = 25;
GLfloat angle, fAspect;
GLfloat rotX, rotY, rotX_ini, rotY_ini;
GLfloat obsX, obsY, obsZ, obsX_ini, obsY_ini, obsZ_ini;
int x_ini, y_ini, bot;
SegmentoRobo Braco, Antebraco;
Garra Mao;

// FALTA FAZER ------------------------------------------------------------------------------------------------------------------
// Função para desenhar um segmento do robô
void DesenhaSegmento()
{
	glLineWidth(2);
	glBegin(GL_LINE_LOOP);
	glVertex2f(1.0, 4.6);
	glVertex2f(1.0, -0.8);
	glVertex2f(-1.0, -0.8);
	glVertex2f(-1.0, 4.6);
	glEnd();
	glPointSize(2);
	glBegin(GL_POINTS);
	glVertex2i(0, 0);
	glEnd();
}

// FALTA FAZER ------------------------------------------------------------------------------------------------------------------
// Função para desenhar uma garra do robô
void DesenhaGarra()
{
	glLineWidth(2);
	glBegin(GL_LINE_LOOP);
	glVertex2f(1.0, 4.6);
	glVertex2f(1.0, -0.8);
	glVertex2f(-1.0, -0.8);
	glVertex2f(-1.0, 4.6);
	glEnd();
	glPointSize(2);
	glBegin(GL_POINTS);
	glVertex2i(0, 0);
	glEnd();
}

// FALTA FAZER ------------------------------------------------------------------------------------------------------------------
// Função para desenhar a base do objeto
void DesenhaBase()
{
	glLineWidth(2);
	glBegin(GL_LINE_LOOP);
	glVertex2f(1.8, 1);
	glVertex2f(1.8, -1.5);
	glVertex2f(1.0, -1.5);
	glVertex2f(1.0, -1);
	glVertex2f(-1.0, -1);
	glVertex2f(-1.0, -1.5);
	glVertex2f(-1.8, -1.5);
	glVertex2f(-1.8, 1);
	glEnd();
}

// FALTA FAZER ------------------------------------------------------------------------------------------------------------------
// Função callback de redesenho da janela de visualização
void Desenha(void)
{
	// Limpa a janela de visualização com a cor de fundo definida previamente
	glClear(GL_COLOR_BUFFER_BIT);

	// Desenha um objeto modelado com transformações hierárquicas
	glPushMatrix();
	glTranslatef(tx, 0.0f, 0.0f);
	glPushMatrix();
	glScalef(2.5f, 2.5f, 1.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	DesenhaBase();
	glPopMatrix();
	glTranslatef(0.0f, 1.5f, 0.0f);
	glRotatef(0.0f, 0.0f, 0.0f, 1.0f);
	glScalef(1.4f, 1.4f, 1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	DesenhaSegmento();
	glTranslatef(0.4f, 2.6f, 0.0f);
	glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
	glColor3f(0.0f, 0.0f, 1.0f);
	DesenhaSegmento();
	glPopMatrix();

	// Executa os comandos OpenGL
	glFlush();
}

// Função usada para especificar a posição do observador virtual
void PosicionaObservador(void)
{
	// Especifica sistema de coordenadas do modelo
	glMatrixMode(GL_MODELVIEW);

	// Inicializa sistema de coordenadas do modelo
	glLoadIdentity();

	// Posiciona e orienta o observador
	glTranslatef(-obsX, -obsY, -obsZ);
	glRotatef(rotX, 1, 0, 0);
	glRotatef(rotY, 0, 1, 0);
}

// Função usada para especificar o volume de visualização
void EspecificaParametrosVisualizacao(void)
{
	// Especifica sistema de coordenadas de projeção
	glMatrixMode(GL_PROJECTION);

	// Inicializa sistema de coordenadas de projeção
	glLoadIdentity();

	// Especifica a projeção perspectiva(angulo, aspecto, zMin, zMax)
	gluPerspective(angle, fAspect, 0.1, 1200);

	PosicionaObservador();
}

// Função callback chamada quando o tamanho da janela é alterado
void AlteraTamanhoJanela(GLsizei w, GLsizei h)
{
	// Evita a divisao por zero
	if (h == 0)
		h = 1;

	// Especifica as dimensões da Viewport
	glViewport(0, 0, w, h);

	// Calcula a correção de aspecto
	fAspect = (GLfloat)w / (GLfloat)h;

	EspecificaParametrosVisualizacao();
}

// Função callback chamada para gerenciar eventos de teclas especiais
void TeclasEspeciais(int key, int x, int y)
{
	// Move a base
	if (key == GLUT_KEY_LEFT)
	{
		tx -= 2;
		if (tx < -win)
			tx = -win;
	}
	if (key == GLUT_KEY_RIGHT)
	{
		tx += 2;
		if (tx > win)
			tx = win;
	}
	if (key == GLUT_KEY_UP)
	{
		ty -= 2;
		if (ty < -win)
			ty = -win;
	}
	if (key == GLUT_KEY_DOWN)
	{
		ty += 2;
		if (ty > win)
			ty = win;
	}

	// Rotaciona antebraço
	if (key == GLUT_KEY_END)
		Antebraco.anguloX -= 5;
	if (key == GLUT_KEY_HOME)
		Antebraco.anguloX += 5;
	if (key == GLUT_KEY_PAGE_DOWN)
		Antebraco.anguloY -= 5;
	if (key == GLUT_KEY_PAGE_UP)
		Antebraco.anguloY += 5;

	glutPostRedisplay();
}

// Função callback chamada para gerenciar eventos de teclas
void Teclado(unsigned char key, int x, int y)
{
	// Sair
	if (key == 27)
		exit(0);

	// Abre garra
	if (key == 81)
		Mao.abertura += 5;
	// Fecha garra
	if (key == 69)
		Mao.abertura -= 5;

	// Rotaciona braço
	if (key == 65)
		Braco.anguloX -= 5;
	if (key == 68)
		Braco.anguloX += 5;
	if (key == 83)
		Braco.anguloY -= 5;
	if (key == 87)
		Braco.anguloY += 5;
}

// Função callback chamada para gerenciar botões do mouse
void Mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		// Salva os parâmetros atuais
		x_ini = x;
		y_ini = y;
		obsX_ini = obsX;
		obsY_ini = obsY;
		obsZ_ini = obsZ;
		rotX_ini = rotX;
		rotY_ini = rotY;
		bot = button;
	}
	else
		bot = -1;
}

// Função callback chamada para gerenciar movimentos do mouse
void MouseMove(int x, int y)
{
	// Gira tela
	if (bot == GLUT_LEFT_BUTTON)
	{
		// Calcula diferenças
		int deltax = x_ini - x;
		int deltay = y_ini - y;
		// E modifica ângulos
		rotY = rotY_ini - deltax / SENS_ROT;
		rotX = rotX_ini - deltay / SENS_ROT;
	}
	// Zoom
	else if (bot == GLUT_RIGHT_BUTTON)
	{
		// Calcula diferença
		int deltaz = y_ini - y;
		// E modifica distância do observador
		obsZ = obsZ_ini + deltaz / SENS_OBS;
	}
	// Move tela
	else if (bot == GLUT_MIDDLE_BUTTON)
	{
		// Calcula diferenças
		int deltax = x_ini - x;
		int deltay = y_ini - y;
		// E modifica posições
		obsX = obsX_ini + deltax / SENS_TRANSL;
		obsY = obsY_ini - deltay / SENS_TRANSL;
	}

	PosicionaObservador();
	glutPostRedisplay();
}

// Função responsável por inicializar parâmetros e variáveis
void Inicializa(void)
{
	// Define a cor de fundo da janela de visualização como branca
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	// Especifica o ângulo da projeção perspectiva
	angle = 60;

	// Altera a posição do observador virtual
	rotX = 0;
	rotY = 0;
	obsX = obsY = 0;
	obsZ = 5;
}

// Programa Principal
int main(int argc, char **argv)
{

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowPosition(5, 5);
	glutInitWindowSize(450, 450);
	glutCreateWindow("Atividade 02");

	// Registra a função callback de redesenho da janela de visualização
	glutDisplayFunc(Desenha);

	// Registra a função callback de redimensionamento da janela de visualização
	glutReshapeFunc(AlteraTamanhoJanela);

	// Registra a função callback para tratamento das teclas especiais
	glutSpecialFunc(TeclasEspeciais);

	// Registra a função callback para tratamento das teclas ASCII
	glutKeyboardFunc(Teclado);

	// Registra a função callback para tratamento do clique no mouse
	glutMouseFunc(Mouse);

	// Registra a função callback para eventos de movimento do mouse
	glutMotionFunc(MouseMove);

	// Chama a função responsável por fazer as inicializações
	Inicializa();

	// Inicia o processamento e aguarda interações do usuário
	glutMainLoop();

	return 0;
}