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

// Declaração de variáveis globais
GLfloat tx = 0, ty = 0;
GLfloat win = 25;
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
	// Muda para o sistema de coordenadas do modelo
	glMatrixMode(GL_MODELVIEW);
	// Inicializa a matriz de transformação corrente
	glLoadIdentity();

	// Limpa a janela de visualização com a cor
	// de fundo definida previamente
	glClear(GL_COLOR_BUFFER_BIT);

	// Desenha o "chão"
	glColor3f(0.0f, 0.0f, 0.0f);
	glLineWidth(4);
	glBegin(GL_LINE_LOOP);
	glVertex2f(-win, -3.9);
	glVertex2f(win, -3.9);
	glEnd();

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

// FALTA FAZER ------------------------------------------------------------------------------------------------------------------
// Função callback chamada quando o tamanho da janela é alterado
void AlteraTamanhoJanela(GLsizei w, GLsizei h)
{
	GLsizei largura, altura;

	// Evita a divisao por zero
	if (h == 0)
		h = 1;

	// Atualiza as variáveis
	largura = w;
	altura = h;

	// Especifica as dimensões da Viewport
	glViewport(0, 0, largura, altura);

	// Inicializa o sistema de coordenadas
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Estabelece a janela de seleção (esquerda, direita, inferior,
	// superior) mantendo a proporção com a janela de visualização
	if (largura <= altura)
	{
		gluOrtho2D(-25.0f, 25.0f, -25.0f * altura / largura, 25.0f * altura / largura);
		win = 25.0f;
	}
	else
	{
		gluOrtho2D(-25.0f * largura / altura, 25.0f * largura / altura, -25.0f, 25.0f);
		win = 25.0f * largura / altura;
	}
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

// Função callback chamada para gerenciar eventos do mouse
void Mouse(int button, int state, int x, int y)
{
	// Abrir garra
	if (button == GLUT_LEFT_BUTTON)
		if (state == GLUT_DOWN)
			Mao.abertura += 5;

	// Fechar garra
	if (button == GLUT_RIGHT_BUTTON)
		if (state == GLUT_DOWN)
			Mao.abertura -= 5;

	glutPostRedisplay();
}

// Função responsável por inicializar parâmetros e variáveis
void Inicializa(void)
{
	// Define a cor de fundo da janela de visualização como branca
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

// Programa Principal
int main(int argc, char **argv)
{
	SegmentoRobo Braco, Antebraco;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowPosition(5, 5);
	glutInitWindowSize(450, 300);
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

	// Chama a função responsável por fazer as inicializações
	Inicializa();

	// Inicia o processamento e aguarda interações do usuário
	glutMainLoop();

	return 0;
}