/*

Componentes: João Vitor de Carvalho, José Henrique Castro e Victor Cavalcanti

Teclado:
1           : Escolhe lixo tipo 1
2           : Escolhe lixo tipo 2
3           : Escolhe lixo tipo 3
4           : Escolhe lixo tipo 4
5           : Escolhe lixo tipo 5
ENTER       : Escolhe opção
BACKSPACE   : Escolhe opção


Teclado Especial:
HOME        : Encerra partida
PAGE UP     : Aumenta velocidade da esteira
PAGE DOWN   : Diminui velocidade da esteira

Mouse:
LEFT        : Pausar
RIGHT       : Despausar

Compilar:
g++ -o main.exe main.cpp lodepng.cpp -I"./include" -L"./lib/x64" -lfreeglut -lopengl32 -lglu32 -lwinmm

Executar:
.\main.exe

*/

#include <GL/freeglut.h>
#include <sstream>
#include <vector>
#include "lodepng.h"
#include <algorithm>
#include <random>
#include <windows.h>
#include <mmsystem.h>
#include <ctime>

// --- Variáveis de Estado do Jogo ---
enum GameState
{
    MENU_INICIAL,
    JOGO,
    GAME_OVER
};

// --- Variáveis do Jogo ---
GameState estadoAtual = MENU_INICIAL;
int pontuacao = 0;
int nivel = 1;
int vida = 5;
const int PONTOS_POR_NIVEL = 100;
const int PONTOS_POR_ACERTO = 10;
float velocidadesNivel[] = {0.0010f, 0.0012f, 0.0014f, 0.0016f, 0.0018f};
const int MAX_NIVEL = 5;
const int MAX_PONTUACAO = 500;
GLboolean pause = false;
float velocidadeEsteira = velocidadesNivel[0];
float offsetEsteira = 0.0f;
GLuint texturaFundoMenu = 0;
int lixoSpawnCounter = 0;

// ---- CONSTANTES DA MÚSICA ----
const char *MUSICA_JOGO = "./resources/sound_game.wav";

// ---- CONSTANTES DO JOGO ----
const float LIXO_START_X = -10.0f;                                   // Posição X inicial do lixo
const float POSICAO_FINAL_ESTEIRA = 11.0f;                           // Posição X onde o lixo é considerado perdido (um pouco depois dos baldes)
const float ZONA_COLETA_INICIO = 7.5f;                               // Início da zona onde o jogador pode coletar o lixo
const float ZONA_COLETA_FIM = 11.5f;                                 // Fim da zona de coleta (próximo aos baldes)
const float TRILHA_Z_POSITIONS[] = {-2.0f, -1.0f, 0.0f, 1.0f, 2.0f}; // Posições Z das 5 trilhas de lixo

enum TipoLixo
{
    METAL,
    PAPEL,
    PLASTICO,
    VIDRO,
    ORGANICO,
    NUM_TIPOS_LIXO
};

struct LixoItem
{
    float posX; // Posição X na esteira (para onde ele se move)
    float posZ; // Posição Z (em qual "trilha" da esteira ele está)
    TipoLixo tipo;
    bool ativo;    // Se ainda está em jogo, não coletado/errado
    float tamanho; // Para variar visualmente
    float cor[3];  // Cor específica do lixo(atualmente não utilizado)
};

std::vector<LixoItem> lixosNaEsteira;

// --- Inicialização do OpenGL ---
void Inicializa()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);
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

void desenharFormaLixo(const LixoItem &lixo)
{
    float r = 0.5f, g = 0.5f, b = 0.5f; // Cor padrão cinza
    switch (lixo.tipo)
    {
    case METAL:
        r = 1.0f;
        g = 1.0f;
        b = 0.0f;
        break; // Amarelo
    case PAPEL:
        r = 0.0f;
        g = 0.0f;
        b = 1.0f;
        break; // Azul
    case PLASTICO:
        r = 1.0f;
        g = 0.0f;
        b = 0.0f;
        break; // Vermelho
    case VIDRO:
        r = 0.0f;
        g = 1.0f;
        b = 0.0f;
        break; // Verde
    case ORGANICO:
        r = 0.54f;
        g = 0.27f;
        b = 0.07f;
        break; // Marrom
    }
    desenharCubo(0.4f, 0.4f, 0.4f, r, g, b);
}

void GerarNovoLixo()
{
    if (lixosNaEsteira.size() > 15)
        return;

    LixoItem novoLixo;
    novoLixo.tipo = static_cast<TipoLixo>(rand() % NUM_TIPOS_LIXO);

    switch (novoLixo.tipo)
    {
    case METAL:
        novoLixo.posZ = TRILHA_Z_POSITIONS[0]; // Trilha do Metal (-2.0f)
        break;
    case PAPEL:
        novoLixo.posZ = TRILHA_Z_POSITIONS[1]; // Trilha do Papel (-1.0f)
        break;
    case PLASTICO:
        novoLixo.posZ = TRILHA_Z_POSITIONS[2]; // Trilha do Plástico (0.0f)
        break;
    case VIDRO:
        novoLixo.posZ = TRILHA_Z_POSITIONS[3]; // Trilha do Vidro (1.0f)
        break;
    case ORGANICO:
        novoLixo.posZ = TRILHA_Z_POSITIONS[4]; // Trilha do Orgânico (2.0f)
        break;
    default:
        novoLixo.posZ = TRILHA_Z_POSITIONS[0];
        break;
    }

    novoLixo.posX = LIXO_START_X;
    novoLixo.ativo = true;
    novoLixo.tamanho = 0.4f;

    lixosNaEsteira.push_back(novoLixo);
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

// --- Funções para Carregar Mídia ---
void carregarTexturaMenu(const char *caminhoDaImagem)
{
    std::vector<unsigned char> image;
    unsigned int width, height;
    unsigned int error = lodepng::decode(image, width, height, caminhoDaImagem);

    if (error)
    {
        texturaFundoMenu = 0;
        return;
    }

    glGenTextures(1, &texturaFundoMenu);
    glBindTexture(GL_TEXTURE_2D, texturaFundoMenu);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

    glBindTexture(GL_TEXTURE_2D, 0);
}

// --- Funções de Desenho de Telas ---
void DesenhaMenuInicial()
{
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); // Salva a matriz de projeção atual
    glLoadIdentity();
    gluOrtho2D(0.0, 800.0, 0.0, 600.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // --- Desenhar a imagem de fundo ---
    if (texturaFundoMenu != 0)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaFundoMenu);

        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(0.0f, 0.0f); // Canto inferior esquerdo
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(800.0f, 0.0f); // Canto inferior direito
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(800.0f, 600.0f); // Canto superior direito
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(0.0f, 600.0f); // Canto superior esquerdo
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
    else
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

    glEnable(GL_BLEND);                                // Habilita o blending para transparência
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Define a função de blending
    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(300.0f, 270.0f);
    glVertex2f(500.0f, 270.0f);
    glVertex2f(500.0f, 325.0f);
    glVertex2f(300.0f, 325.0f);
    glEnd();
    glDisable(GL_BLEND); // Desabilita o blending

    desenharTexto(310, 300, "ENTER - INICIAR", 1.0, 1.0, 1.0);
    desenharTexto(310, 280, "BACKSPACE - SAIR", 1.0, 1.0, 1.0);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix(); // Restaura a matriz de projeção 3D
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

void DesenhaTelaFim()
{
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 800.0, 0.0, 600.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // --- Desenhar a imagem de fundo ---
    if (texturaFundoMenu != 0)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaFundoMenu);

        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(0.0f, 0.0f); // Canto inferior esquerdo
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(800.0f, 0.0f); // Canto inferior direito
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(800.0f, 600.0f); // Canto superior direito
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(0.0f, 600.0f); // Canto superior esquerdo
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
    else
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    }

    glEnable(GL_BLEND);                                // Habilita o blending para transparência
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Define a função de blending
    glColor4f(0.0f, 0.0f, 0.0f, 0.6f);
    glBegin(GL_QUADS);
    glVertex2f(270.0f, 220.0f);
    glVertex2f(540.0f, 220.0f);
    glVertex2f(540.0f, 328.0f);
    glVertex2f(270.0f, 328.0f);
    glEnd();
    glDisable(GL_BLEND); // Desabilita o blending

    std::stringstream ssPontuacaoFinal;
    ssPontuacaoFinal << "PONTUACAO FINAL: " << pontuacao;
    desenharTexto(305, 300, ssPontuacaoFinal.str().c_str(), 0.0, 1.0, 0.0);

    desenharTexto(280, 250, "ENTER - VOLTAR AO INICIO", 1.0, 1.0, 1.0);
    desenharTexto(280, 230, "BACKSPACE - SAIR", 1.0, 1.0, 1.0);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

void DesenhaJogo()
{
    glEnable(GL_DEPTH_TEST);
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
    for (const auto &lixo : lixosNaEsteira)
    {
        if (lixo.ativo)
        {
            glPushMatrix();
            glTranslatef(lixo.posX, 0.5f, lixo.posZ); // 0.5f é a altura na esteira
            desenharFormaLixo(lixo);                  // Nova função
            glPopMatrix();
        }
    }

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
        desenharTexto(350, 300, "PAUSADO", 1.0, 0.0, 0.0);
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

        // Movimentar lixos
        for (auto &lixo : lixosNaEsteira)
        {
            if (lixo.ativo)
            {
                lixo.posX += velocidadeEsteira * 20; // O multiplicador para a velocidade desejada do lixo
            }
        }

        // Remover lixos inativos (coletados ou perdidos) para otimizar
        lixosNaEsteira.erase(std::remove_if(lixosNaEsteira.begin(), lixosNaEsteira.end(),
                                            [](const LixoItem &l)
                                            { return !l.ativo; }),
                             lixosNaEsteira.end());

        // Lógica de gerar novo lixo
        lixoSpawnCounter++;
        int intervaloSpawn = 120 - (nivel * 15);
        if (intervaloSpawn < 30)
            intervaloSpawn = 30; // Mínimo intervalo para não ficar impossível

        if (lixoSpawnCounter > intervaloSpawn)
        {
            GerarNovoLixo();
            lixoSpawnCounter = 0;
        }

        // Lógica de lixo perdido (passou do fim da esteira)
        for (auto &lixo : lixosNaEsteira)
        {
            if (lixo.ativo && lixo.posX > POSICAO_FINAL_ESTEIRA)
            { // POSICAO_FINAL_ESTEIRA um pouco depois dos baldes
                vida--;
                lixo.ativo = false;
                if (vida < 1)
                {
                    estadoAtual = GAME_OVER;
                    Inicializa(); // Ou uma função SetupGameOver()
                }
            }
        }
    }
    glutPostRedisplay();
    glutTimerFunc(1000 / 120, Anima, 0);
}

void ResetarJogo()
{
    // Reseta as estatísticas do jogador
    pontuacao = 0;
    nivel = 1;
    vida = 5;

    // Reseta as variáveis de controle do jogo
    velocidadeEsteira = velocidadesNivel[0];
    lixoSpawnCounter = 0;
    pause = false;

    // Limpa os lixos na lixeira
    lixosNaEsteira.clear();

    // Para a música de fundo e a reinicia, garantindo um começo limpo
    PlaySound(NULL, NULL, 0);
    PlaySound(MUSICA_JOGO, NULL, SND_ASYNC | SND_LOOP | SND_FILENAME);
}

// --- Manipulação de Teclado/Mouse ---
void Teclado(unsigned char key, int x, int y)
{
    bool acertoRealizado = false;
    TipoLixo tipoSelecionado;
    switch (estadoAtual)
    {
    case MENU_INICIAL:
        if (key == 13)
        {
            estadoAtual = JOGO;
            ResetarJogo();
            Inicializa();
        }
        else if (key == 8)
        {
            PlaySound(NULL, NULL, 0);
            exit(0);
        }
        break;
    case JOGO:

        if (key >= '1' && key <= '5')
        {
            switch (key)
            {
            case '1':
                tipoSelecionado = ORGANICO;
                break;
            case '2':
                tipoSelecionado = VIDRO;
                break;
            case '3':
                tipoSelecionado = PLASTICO;
                break;
            case '4':
                tipoSelecionado = PAPEL;
                break;
            case '5':
                tipoSelecionado = METAL;
                break;
            default:
                return;
            }
            for (auto &lixo : lixosNaEsteira)
            {
                if (lixo.ativo && lixo.posX >= ZONA_COLETA_INICIO && lixo.posX <= ZONA_COLETA_FIM)
                {
                    if (lixo.tipo == tipoSelecionado)
                    {
                        // ACERTO
                        pontuacao += PONTOS_POR_ACERTO;
                        lixo.ativo = false;
                        acertoRealizado = true;
                        if (nivel < MAX_NIVEL && pontuacao >= nivel * PONTOS_POR_NIVEL)
                        {
                            nivel++;
                            if (nivel <= MAX_NIVEL)
                            {
                                velocidadeEsteira = velocidadesNivel[nivel - 1];
                            }
                        }
                        if (nivel == MAX_NIVEL && pontuacao >= MAX_PONTUACAO)
                        {
                            estadoAtual = GAME_OVER;
                            Inicializa();
                        }
                        break;
                    }
                }
            }
            if (!acertoRealizado && (key >= '1' && key <= '5'))
            {
                bool lixoNaZona = false;
                bool tipoErradoPressionadoParaLixoPresente = false;

                for (auto &lixo : lixosNaEsteira)
                {
                    if (lixo.ativo && lixo.posX >= ZONA_COLETA_INICIO && lixo.posX <= ZONA_COLETA_FIM)
                    {
                        lixoNaZona = true;
                        if (lixo.tipo != tipoSelecionado)
                        {
                            tipoErradoPressionadoParaLixoPresente = true;
                        }
                        break;
                    }
                }

                if (tipoErradoPressionadoParaLixoPresente)
                {
                    vida--;
                }
                else if (lixoNaZona)
                {
                    // Apertou a tecla certa, mas já foi processado como acerto. Não deveria chegar aqui se acertoRealizado=true
                }
                else
                {
                    vida--; // Penalidade por apertar "em falso".(A decidir)
                }
            }
            if (vida < 1 && estadoAtual == JOGO)
            {
                estadoAtual = GAME_OVER;
                Inicializa();
            }
            break;

        case GAME_OVER:
            if (key == 13)
            {
                estadoAtual = MENU_INICIAL;
                srand(time(NULL));
                Inicializa();
            }
            else if (key == 8)
            {
                PlaySound(NULL, NULL, 0);
                exit(0);
            }
            break;
        }
        glutPostRedisplay();
    }
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

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Projeto Final - Cleaning Hero");
    carregarTexturaMenu("./resources/background.png");
    glutDisplayFunc(Desenha);

    glutKeyboardFunc(Teclado);
    glutSpecialFunc(TecladoEspecial);
    glutMouseFunc(Mouse);
    PlaySound(MUSICA_JOGO, NULL, SND_ASYNC | SND_LOOP);

    glutTimerFunc(0, Anima, 0);
    glutMainLoop();
    return 0;
}