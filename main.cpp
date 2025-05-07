/*
Componentes: João Vitor de Carvalho e Victor Cavalcanti

Controles de Teclado:
a / d : rotaciona braço
w / s : rotaciona antebraço
q / e : abre/fecha garra
r     : reseta posições
↑ / ↓ : mover câmera para cima/baixo
← / → : mover câmera para esquerda/direita

Mouse:
Botão esquerdo: mover base para esquerda
Botão direito: mover base para direita

*/

#include <GL/freeglut.h>

// Sensibilidade dos controles
#define SENS_ROT 5.0
#define SENS_OBS 15.0
#define SENS_TRANSL 30.0

typedef struct
{
    float comprimento;
    float largura;
    float cor[3];
    float anguloZ;
} Segmento;

typedef struct
{
    float abertura;
    float comprimento;
    float cor[3];
} Garra;

// Variáveis globais
Segmento Braco = {1.0f, 0.2f, {1.0f, 0.0f, 0.0f}, 0};
Segmento Antebraco = {0.8f, 0.15f, {0.0f, 1.0f, 0.0f}, 0};
Garra Mao = {20.0f, 0.3f, {0.0f, 0.0f, 1.0f}};

GLfloat cameraAngleX = 0.0f;
GLfloat cameraAngleY = 0.0f;
GLfloat tx = 0.0f;
GLfloat angle = 60, fAspect;

// Desenho da mesa e base fixa
void DesenhaMesa()
{
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2);
    glBegin(GL_LINE_LOOP); // tampo
    glVertex2f(-1.5f, -0.3f);
    glVertex2f(-1.5f, -0.6f);
    glVertex2f(1.5f, -0.6f);
    glVertex2f(1.5f, -0.3f);
    glEnd();

    glBegin(GL_LINE_LOOP); // perna esquerda
    glVertex2f(-1.2f, -0.6f);
    glVertex2f(-1.2f, -1.0f);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(-1.0f, -0.6f);
    glEnd();

    glBegin(GL_LINE_LOOP); // perna direita
    glVertex2f(1.2f, -0.6f);
    glVertex2f(1.2f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, -0.6f);
    glEnd();
}

void DesenhaPlataforma()
{
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.4f, 0.0f);
    glVertex2f(-0.4f, 0.3f);
    glVertex2f(0.4f, 0.3f);
    glVertex2f(0.4f, 0.0f);
    glEnd();
}

void DesenhaSegmento(Segmento s)
{
    glColor3fv(s.cor);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-s.largura / 2, 0);
    glVertex2f(-s.largura / 2, s.comprimento);
    glVertex2f(s.largura / 2, s.comprimento);
    glVertex2f(s.largura / 2, 0);
    glEnd();
}

void DesenhaGarra(Garra g)
{
    // Garra esquerda
    glPushMatrix();
    glTranslatef(-0.1f, 0.0f, 0.0f);
    glRotatef(g.abertura, 0, 0, 1);
    glColor3fv(g.cor);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.025f, 0);
    glVertex2f(-0.025f, g.comprimento);
    glVertex2f(0.025f, g.comprimento);
    glVertex2f(0.025f, 0);
    glEnd();
    glPopMatrix();

    // Garra direita
    glPushMatrix();
    glTranslatef(0.1f, 0.0f, 0.0f);
    glRotatef(-g.abertura, 0, 0, 1);
    glColor3fv(g.cor);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.025f, 0);
    glVertex2f(-0.025f, g.comprimento);
    glVertex2f(0.025f, g.comprimento);
    glVertex2f(0.025f, 0);
    glEnd();
    glPopMatrix();
}

void Desenha()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -5.0f); // Afasta a câmera
    glRotatef(cameraAngleX, 1.0f, 0.0f, 0.0f);
    glRotatef(cameraAngleY, 0.0f, 1.0f, 0.0f);

    DesenhaMesa(); // A mesa está fixa

    glPushMatrix();
    glTranslatef(tx, 0.0f, 0.0f); // Apenas a base + braço se move

    glTranslatef(0.0f, -0.3f, 0.0f); // Sobe para a plataforma
    DesenhaPlataforma();

    glTranslatef(0.0f, 0.3f, 0.0f); // Sobe para o braço

    glPushMatrix(); // Braço
    glRotatef(Braco.anguloZ, 0, 0, 1);
    DesenhaSegmento(Braco);

    glTranslatef(0.0f, Braco.comprimento, 0.0f);
    glPushMatrix(); // Antebraço
    glRotatef(Antebraco.anguloZ, 0, 0, 1);
    DesenhaSegmento(Antebraco);

    glTranslatef(0.0f, Antebraco.comprimento, 0.0f);
    glScalef(1.2, 1.2, 0);
    DesenhaGarra(Mao);
    glPopMatrix();
    glPopMatrix();

    glPopMatrix();

    glFlush();
}

void Teclado(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:
        exit(0);
    case 'a':
        Braco.anguloZ += 5;
        break;
    case 'd':
        Braco.anguloZ -= 5;
        break;
    case 'w':
        Antebraco.anguloZ += 5;
        break;
    case 's':
        Antebraco.anguloZ -= 5;
        break;
    case 'q':
        Mao.abertura += 2;
        break;
    case 'e':
        Mao.abertura -= 2;
        break;
    case 'r':
        Braco.anguloZ = 0;
        Antebraco.anguloZ = 0;
        Mao.abertura = 20.0f;

        cameraAngleX = 0.0f;
        cameraAngleY = 0.0f;
        tx = 0.0f;

        glutPostRedisplay(); // Reforça o redesenho da tela
        break;
    }
    glutPostRedisplay();
}

void TecladoEspecial(int tecla, int x, int y)
{
    switch (tecla)
    {
    case GLUT_KEY_LEFT:
        cameraAngleY -= 5.0f;
        break;
    case GLUT_KEY_RIGHT:
        cameraAngleY += 5.0f;
        break;
    case GLUT_KEY_UP:
        cameraAngleX -= 5.0f;
        break;
    case GLUT_KEY_DOWN:
        cameraAngleX += 5.0f;
        break;
    }
    glutPostRedisplay();
}

void Mouse(int botao, int estado, int x, int y)
{
    if (estado == GLUT_DOWN)
    {
        if (botao == GLUT_LEFT_BUTTON)
        {
            tx -= 0.1f;
        }
        if (botao == GLUT_RIGHT_BUTTON)
        {
            tx += 0.1f;
        }
        glutPostRedisplay();
    }
}

void Inicializa()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void AlteraTamanhoJanela(GLsizei w, GLsizei h)
{
    if (h == 0)
        h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(angle, (GLfloat)w / (GLfloat)h, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Atividade 02 - Braco Robotico");
    glutDisplayFunc(Desenha);
    glutKeyboardFunc(Teclado);
    glutSpecialFunc(TecladoEspecial);
    glutMouseFunc(Mouse);
    glutReshapeFunc(AlteraTamanhoJanela);
    Inicializa();
    glutMainLoop();
    return 0;
}
