/*

Componentes: João Vitor de Carvalho, José Henrique Castro e Victor Cavalcanti

Teclado:
1          : Escolha (No jogo: Lixo tipo 1, No menu/game over: Escolhe opção)
2          : Escolha (No jogo: Lixo tipo 2, No menu/game over: Escolhe opção)
3          : Escolha (No jogo: Lixo tipo 3, No menu/game over: Escolhe opção)
4          : Escolha (No jogo: Lixo tipo 4, No menu/game over: Escolhe opção)
5          : Escolha (No jogo: Lixo tipo 5, No menu/game over: Escolhe opção)

Teclado Especial:
HOME        : Encerra partida (no jogo)
PAGE UP     : Aumenta velocidade da esteira (no jogo)
PAGE DOWN   : Diminui velocidade da esteira (no jogo)

Mouse:
LEFT        : Pausar (no jogo)
RIGHT       : Despausar (no jogo)

*/

#include <GL/freeglut.h>
#include <sstream>


// --- Variáveis de Estado do Jogo ---
enum GameState
{
    MENU_INICIAL,
    JOGO,
    GAME_OVER
};

// Variáveis do Jogo
GameState estadoAtual = MENU_INICIAL;
int pontuacao = 0;
int nivel = 1;
int vida = 5;
const int PONTOS_POR_NIVEL = 100;
const int PONTOS_POR_ACERTO = 10;
float velocidadesNivel[] = {0.002f, 0.004f, 0.006f, 0.008f, 0.01f};
const int MAX_NIVEL = 5;
GLboolean pause = false;
float velocidadeEsteira = velocidadesNivel[0];
float offsetEsteira = 0.0f;

// --- Inicialização do OpenGL ---
void Inicializa()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0 / 600.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// --- Funções para Desenhar Formas Básicas ---
void desenharCubo(float tamanhoX, float tamanhoY, float tamanhoZ, float r, float g, float b)
{
    glColor3f(r, g, b);
    glPushMatrix();
    glScalef(tamanhoX, tamanhoY, tamanhoZ);
    glutSolidCube(1.0);
    glPopMatrix();
}

void desenharCilindro(float raioBase, float raioTop, float altura, int fatias, int pilhas, float r, float g, float b)
{
    glColor3f(r, g, b);
    glPushMatrix();
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    glutSolidCylinder(raioBase, altura, fatias, pilhas);
    glPopMatrix();
}

// --- Funções para Desenhar Texto ---
void desenharTexto(float x, float y, const char *texto, float r, float g, float b)
{
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (const char *c = texto; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

void desenharTextoGrande(float x, float y, const char *texto, float r, float g, float b)
{
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (const char *c = texto; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c);
    }
}

// --- Funções de Desenho de Telas ---
void DesenhaMenuInicial()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 800.0, 0.0, 600.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    desenharTextoGrande(300, 450, "CLEANING HERO", 1.0, 1.0, 1.0);
    desenharTexto(350, 300, "1 - INICIAR", 1.0, 1.0, 1.0);
    desenharTexto(350, 250, "2 - SAIR", 1.0, 1.0, 1.0);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

void DesenhaTelaFim()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 800.0, 0.0, 600.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    desenharTextoGrande(300, 450, "FIM DE JOGO", 1.0, 1.0, 1.0);

    std::stringstream ssPontuacaoFinal;
    ssPontuacaoFinal << "PONTUACAO FINAL: " << pontuacao;
    desenharTextoGrande(250, 350, ssPontuacaoFinal.str().c_str(), 0.0, 1.0, 0.0); // Verde

    desenharTexto(300, 300, "1 - VOLTAR AO INICIO", 1.0, 1.0, 1.0);
    desenharTexto(300, 250, "2 - SAIR", 1.0, 1.0, 1.0);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

void DesenhaJogo()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(16, 4, 1,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);

    // --- Desenhar a Esteira Rolante ---
    glPushMatrix();
    glTranslatef(0.0, -0.5, 0.0);
    desenharCubo(20.0, 1.0, 5.0, 0.5, 0.5, 0.5);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0, 0.0, 0.0);
    glTranslated(offsetEsteira, 0.0, 0.0);
    desenharCubo(19.0, 0.2, 4.0, 0.3, 0.3, 0.3);
    glPopMatrix();

    // --- Desenhar os Baldes de Lixo ---
    float espacamentoBaldes = 1.0;
    float posXInicialBaldes = -2.0;
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
    desenharCilindro(0.5, 0.5, 1.0, 32, 32, 0.54, 0.27, 0.07);
    glPopMatrix();

    // --- Desenhar Lixo (Exemplo de um cubo para lixo) ---
    glPushMatrix();
    glTranslatef(5.0 - offsetEsteira * 20, 0.5, 0.0);
    desenharCubo(0.5, 0.5, 0.5, 0.6, 0.4, 0.2);
    glPopMatrix();

    // --- Exibir Pontuação e Nível ---
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 800.0, 0.0, 600.0); // Projeção 2D para texto HUD
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    std::stringstream ssPontuacao;
    ssPontuacao << "PONTOS: " << pontuacao;
    desenharTexto(10, 580, ssPontuacao.str().c_str(), 0.0, 1.0, 0.0); // Verde

    std::stringstream ssNivel;
    ssNivel << "NIVEL: " << nivel;
    desenharTexto(10, 560, ssNivel.str().c_str(), 0.0, 0.0, 1.0); // Azul

    std::stringstream ssVida;
    ssVida << "VIDAS: " << vida;
    desenharTexto(10, 540, ssVida.str().c_str(), 1.0, 0.0, 0.0); // Vermelho

    // Exibir status de pausa
    if (pause)
    {
        desenharTextoGrande(350, 300, "PAUSADO", 1.0, 0.0, 0.0);
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

// --- Função de Display Principal (Gerencia os estados) ---
void Desenha()
{
    switch (estadoAtual)
    {
    case MENU_INICIAL:
        DesenhaMenuInicial();
        break;
    case JOGO:
        DesenhaJogo();
        break;
    case GAME_OVER:
        DesenhaTelaFim();
        break;
    }
}

// --- Função de Animação (Idle) ---
void Anima(int valor)
{
    if (estadoAtual == JOGO && !pause)
    {
        offsetEsteira += velocidadeEsteira;
        if (offsetEsteira > 1.0f)
        { // Reinicia o offset para simular movimento contínuo
            offsetEsteira -= 1.0f;
            // AQUI: Lógica de Lixo
        }
    }

    glutPostRedisplay();
    glutTimerFunc(1000 / 120, Anima, 0);
}

// --- Manipulação de Teclado/Mouse ---
void Teclado(unsigned char key, int x, int y)
{
    switch (estadoAtual)
    {
    case MENU_INICIAL:
        if (key == '1')
        {
            estadoAtual = JOGO;
            pontuacao = 0;                           // Zera a pontuação
            nivel = 1;                               // Volta para o nível 1
            vida = 5;                                // Reseta as vidas
            velocidadeEsteira = velocidadesNivel[0]; // Reseta a velocidade para o nível 1
            Inicializa();
        }
        else if (key == '2')
        {
            exit(0);
        }
        break;
    case JOGO:
        switch (key)
        {
        case '1': // Lixo tipo 1

            pontuacao += PONTOS_POR_ACERTO;
            if (nivel < MAX_NIVEL && pontuacao >= nivel * PONTOS_POR_NIVEL)
            {
                nivel++;
                velocidadeEsteira = velocidadesNivel[nivel - 1];
            }
            if (nivel == MAX_NIVEL && pontuacao >= nivel * PONTOS_POR_NIVEL)
            {
                estadoAtual = GAME_OVER;
                Inicializa();
            }
            break;

        case '2': // Lixo tipo 2
            break;
        case '3': // Lixo tipo 3
            break;
        case '4': // Lixo tipo 4
            break;
        case '5': // Lixo tipo 5
            vida -= 1;
            if (vida < 0)
            {
                estadoAtual = GAME_OVER;
                Inicializa();
            }
            break;
        }
        break;
    case GAME_OVER:
        if (key == '1')
        {
            estadoAtual = MENU_INICIAL;
            Inicializa();
        }
        else if (key == '2')
            exit(0); // Sai do jogo
        break;
    }
    glutPostRedisplay();
}

void TecladoEspecial(int tecla, int x, int y)
{
    if (estadoAtual == JOGO)
    {
        switch (tecla)
        {
        case GLUT_KEY_PAGE_UP:
            velocidadeEsteira += 0.01f; // Aumenta velocidade
            if (velocidadeEsteira > 0.5f)
                velocidadeEsteira = 0.5f;
            break;
        case GLUT_KEY_PAGE_DOWN:
            velocidadeEsteira -= 0.01f; // Diminui velocidade
            if (velocidadeEsteira < 0.01f)
                velocidadeEsteira = 0.01f;
            break;
        case GLUT_KEY_HOME:
            estadoAtual = GAME_OVER;
            break;
        }
    }
    glutPostRedisplay();
}

void Mouse(int botao, int estado, int x, int y)
{
    if (estadoAtual == JOGO)
    {
        if (botao == GLUT_LEFT_BUTTON && estado == GLUT_DOWN && pause == false)
            pause = !pause;
        if (botao == GLUT_RIGHT_BUTTON && estado == GLUT_DOWN && pause == true)
            pause = !pause;
    }
    glutPostRedisplay();
}

// --- Função Principal ---
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

    glutTimerFunc(0, Anima, 0);
    glutMainLoop();
    return 0;
}