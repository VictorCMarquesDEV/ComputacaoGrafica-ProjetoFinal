/*

Componentes: João Vitor de Carvalho, José Henrique Castro e Victor Cavalcanti

Teclado:
1           : Escolhe
2           : Escolhe
3           : Escolhe
4           : Escolhe
5           : Escolhe

Teclado Especial:
HOME        : Abre menu
PAGE UP     : Aumenta velocidade da esteira
PAGE DOWN   : Diminui velocidade da esteira

Mouse:
LEFT        : Pausar
RIGHT       : Pronto

*/

#include <GL/freeglut.h>

// Variáveis
GLboolean pause = false;

// Funções para desenhar formas básicas
void desenharCubo(float tamanhoX, float tamanhoY, float tamanhoZ, float r, float g, float b)
{
    glColor3f(r, g, b);
    glPushMatrix();
    glScalef(tamanhoX, tamanhoY, tamanhoZ);
    glutSolidCube(1.0); // Ou uma função similar para desenhar um cubo
    glPopMatrix();
}

void desenharCilindro(float raioBase, float raioTop, float altura, int fatias, int pilhas, float r, float g, float b)
{
    glColor3f(r, g, b);
    glPushMatrix();
    glRotatef(-90.0, 1.0, 0.0, 0.0);                     // Alinha o cilindro verticalmente
    glutSolidCylinder(raioBase, altura, fatias, pilhas); // Ou uma função similar
    glPopMatrix();
}

void Desenha()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Configurar a câmera (exemplo)
    gluLookAt(16, 4, 1, // Posição da câmera
              0.0, 0.0, 0.0,   // Para onde a câmera está olhando
              0.0, 1.0, 0.0);  // Vetor "up"

    // --- Desenhar a Esteira Rolante ---
    // Base da esteira
    glPushMatrix();
    glTranslatef(0.0, -0.5, 0.0);                // Posição
    desenharCubo(20.0, 1.0, 5.0, 0.5, 0.5, 0.5); // Cinza escuro
    glPopMatrix();

    // Superfície móvel da esteira (poderia ter textura e animação aqui)
    glPushMatrix();
    glTranslatef(0.0, 0.0, 0.0);                 // Posição
    desenharCubo(19.0, 0.2, 4.0, 0.3, 0.3, 0.3); // Cinza mais claro
    glPopMatrix();

    // --- Desenhar os Baldes de Lixo ---
    float espacamentoBaldes = 1.0;
    float posXInicialBaldes = -2.0; // Posição inicial no eixo X
    float posYBaldes = -0.9;
    float posZBaldes = 10.0;

    // Balde para Metal (Amarelo)
    glPushMatrix();
    glTranslatef(posZBaldes, posYBaldes, posXInicialBaldes);
    desenharCilindro(0.5, 0.5, 1.0, 32, 32, 1.0, 1.0, 0.0);
    glPopMatrix();

    // Balde para Papel (Azul)
    glPushMatrix();
    glTranslatef(posZBaldes, posYBaldes, posXInicialBaldes + espacamentoBaldes);
    desenharCilindro(0.5, 0.5, 1.0, 32, 32, 0.0, 0.0, 1.0);
    glPopMatrix();

    // Balde para Plástico (Vermelho)
    glPushMatrix();
    glTranslatef(posZBaldes, posYBaldes, posXInicialBaldes + 2 * espacamentoBaldes);
    desenharCilindro(0.5, 0.5, 1.0, 32, 32, 1.0, 0.0, 0.0);
    glPopMatrix();

    // Balde para Vidro (Verde)
    glPushMatrix();
    glTranslatef(posZBaldes, posYBaldes, posXInicialBaldes + 3 * espacamentoBaldes);
    desenharCilindro(0.5, 0.5, 1.0, 32, 32, 0.0, 1.0, 0.0);
    glPopMatrix();

    // Balde para Orgânico (Marrom)
    glPushMatrix();
    glTranslatef(posZBaldes, posYBaldes, posXInicialBaldes + 4 * espacamentoBaldes);
    desenharCilindro(0.5, 0.5, 1.0, 32, 32, 0.5, 0.3, 0.1);
    glPopMatrix();

    glutSwapBuffers(); // Troca os buffers para exibir o que foi desenhado
}

void Teclado(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 49:
        // Escolhe - 1
        break;
    case 50:
        // Escolhe - 2
        break;
    case 51:
        // Escolhe - 3
        break;
    case 52:
        // Escolhe - 4
        break;
    case 53:
        // Escolhe - 5
        break;
    }

    glutPostRedisplay();
}

void TecladoEspecial(int tecla, int x, int y)
{
    switch (tecla)
    {
    case GLUT_KEY_HOME:
        // Abre menu
        break;
    case GLUT_KEY_PAGE_UP:
        // Aumenta velocidade da esteira
        break;
    case GLUT_KEY_PAGE_DOWN:
        // Diminui velocidade da esteira
        break;
    }

    glutPostRedisplay();
}

// Pausar o jogo
void Mouse(int botao, int estado, int x, int y)
{
    if (botao == GLUT_LEFT_BUTTON && pause == false)
        !pause;
    if (botao == GLUT_RIGHT_BUTTON && pause == true)
        !pause;
    glutPostRedisplay();
}

void Inicializa()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST); // Habilita o teste de profundidade para objetos 3D
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0 / 600.0, 0.1, 100.0); // Perspectiva
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Projeto Final - Cleaning Hero");
    glutDisplayFunc(Desenha);

    glutKeyboardFunc(Teclado);
    glutSpecialFunc(TecladoEspecial);
    glutMouseFunc(Mouse);

    Inicializa();
    glutMainLoop();
    return 0;
}