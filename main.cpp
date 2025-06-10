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
g++ -o main.exe main.cpp lodepng.cpp -I"./include" -lfreeglut -lopengl32 -lglu32 -lwinmm

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
#include <map>
#include <cmath>

const float PI = 3.1415926535f;

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
std::map<std::string, GLuint> texturasDoJogo;
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
    int seed; // Semente para aleatoriedade na forma
};

std::vector<LixoItem> lixosNaEsteira;

std::random_device rd;      // Obtém uma semente do hardware (se disponível)
std::mt19937 gen(rd());     // Semeia o gerador Mersenne Twister
std::uniform_int_distribution<> distrib(0, NUM_TIPOS_LIXO - 1); // Cria uma distribuição uniforme

// --- Variáveis para controle de repetição ---
TipoLixo ultimoLixoGerado = METAL;
int contadorRepeticao = 0;

// --- Variáveis para o efeito de flash de acerto/erro ---
bool efeitoAuraAtivo = false;
float corAura[4] = {1.0f, 1.0f, 1.0f, 1.0f}; // Cor da aura (RGBA)
int duracaoAura = 0;
const int DURACAO_MAX_AURA = 20;
int lixeiraAuraIndex = -1;       // Qual lixeira terá a aura (-1 = nenhuma)

// --- Inicialização do OpenGL ---
void Inicializa()
{
    glClearColor(0.9f, 0.9f, 0.9f, 1.0f); // Um fundo cinza claro
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);

    // Configura a câmera
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0 / 600.0, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Semeia o gerador de números aleatórios
    srand(time(NULL));
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

void desenharLataAmassada() {
    int segments = 20;
    float height = 0.5f;
    float radius = 0.2f;

    glPushMatrix();
    glScalef(0.8f, 1.0f, 0.8f);

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; ++i) {
        float angle = (float)i / segments * 2.0f * PI;
        float x = cos(angle);
        float z = sin(angle);
        float dent_factor_top = 1.0f;
        float dent_factor_bottom = 0.6f + (cos(angle * 4) * 0.1f);

        // Variação de cor sutil para dar aspecto de sujeira/uso
        float colorVariation = 0.05f * (sin(angle * 7) + 1.0f);
        glColor3f(0.75f - colorVariation, 0.75f - colorVariation, 0.8f - colorVariation);

        glVertex3f(x * radius * dent_factor_top, height / 2.0f, z * radius * dent_factor_top);
        glVertex3f(x * radius * dent_factor_bottom, -height / 2.0f, z * radius * dent_factor_bottom);
    }
    glEnd();

    // Tampa e Fundo com a cor base
    glColor3f(0.75f, 0.75f, 0.8f);
    for (int j = -1; j <= 1; j += 2) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, (float)j * height / 2.0f, 0.0f);
        for (int i = 0; i <= segments; ++i) {
            float angle = (float)i / segments * 2.0f * PI;
            float dent_factor = (j > 0) ? 1.0f : 0.6f + (cos(angle * 4) * 0.1f);
            glVertex3f(cos(angle) * radius * dent_factor, (float)j * height / 2.0f, sin(angle) * radius * dent_factor);
        }
        glEnd();
    }
    glPopMatrix();
}

void desenharPapelAmassado(int seed) {
    int segments = 9; // Menos segmentos para um visual mais "quadrado"
    float radius = 0.35f;
    srand(seed);

    for (int i = 0; i < segments; ++i) {
        for (int j = 0; j < segments; ++j) {
            glBegin(GL_POLYGON);
            // Varia a cor de cada polígono
             glColor3f(0.9f - (float)(rand()%10)/100.0f, 0.85f - (float)(rand()%10)/100.0f, 0.7f - (float)(rand()%10)/100.0f);

            // Vértice 1
            float phi1 = (float)i / segments * 2.0f * PI;
            float theta1 = (float)j / segments * PI;
            float r1 = radius + (float)(rand() % 100) / 700.0f; // Aumenta a irregularidade
            glVertex3f(r1*cos(phi1)*sin(theta1) * 1.2f, r1*sin(phi1)*sin(theta1) * 0.8f, r1*cos(theta1)); // Deforma a esfera

            // Vértice 2
            float phi2 = (float)(i + 1) / segments * 2.0f * PI;
            float r2 = radius + (float)(rand() % 100) / 700.0f;
            glVertex3f(r2*cos(phi2)*sin(theta1) * 1.2f, r2*sin(phi2)*sin(theta1) * 0.8f, r2*cos(theta1));

            // Vértice 3
            float theta2 = (float)(j + 1) / segments * PI;
            float r3 = radius + (float)(rand() % 100) / 700.0f;
            glVertex3f(r3*cos(phi2)*sin(theta2) * 1.2f, r3*sin(phi2)*sin(theta2) * 0.8f, r3*cos(theta2));

            // Vértice 4
            float r4 = radius + (float)(rand() % 100) / 700.0f;
            glVertex3f(r4*cos(phi1)*sin(theta2) * 1.2f, r4*sin(phi1)*sin(theta2) * 0.8f, r4*cos(theta2));

            glEnd();
        }
    }
}

void desenharGarrafaPlasticaAmassada() {
    int segments = 20;
    float heights[] = {-0.4f, 0.0f, 0.2f, 0.3f, 0.4f};
    float radii[]   = {0.2f, 0.2f, 0.15f, 0.08f, 0.09f};

    glPushMatrix();
    glScalef(1.0f, 1.0f, 0.5f); // Amassada no eixo Z
    glRotatef(20.0f, 1.0f, 0.0f, 0.0f); // Inclinada

    // Habilita a transparência para o plástico
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Define uma cor azul translúcida fixa
    glColor4f(0.5f, 0.7f, 1.0f, 0.6f);

    for (size_t h = 0; h < 4; ++h) {
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= segments; ++i) {
            float angle = (float)i / segments * 2.0f * PI;
            float x = cos(angle);
            float z = sin(angle);
            
            glVertex3f(x * radii[h+1], heights[h+1], z * radii[h+1]);
            glVertex3f(x * radii[h], heights[h], z * radii[h]);
        }
        glEnd();
    }
    
    glDisable(GL_BLEND); // Desabilita para não afetar outros objetos
    glPopMatrix();
}

void desenharGarrafaDeVidroQuebrada() {
    int segments = 18;
    float height = 0.3f;
    float radius = 0.2f;

    // Habilita o blending para transparência
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Cor verde com 70% de opacidade (alpha = 0.7)
    glColor4f(0.6f, 0.9f, 0.7f, 0.7f);

    // Base da garrafa
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; ++i) {
        float angle = (float)i / segments * 2.0f * PI;
        float x = cos(angle);
        float z = sin(angle);
        float top_height_modifier = (cos(angle * 5) + 1.0f) * 0.15f + 0.1f;
        glVertex3f(x * radius, height + top_height_modifier, z * radius);
        glVertex3f(x * radius, -height, z * radius);
    }
    glEnd();
    
    // Fundo da garrafa
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, -height, 0.0f);
    for (int i = 0; i <= segments; ++i) {
        float angle = (float)i / segments * 2.0f * PI;
        glVertex3f(cos(angle) * radius, -height, sin(angle) * radius);
    }
    glEnd();

    // Desabilita para não afetar outros objetos
    glDisable(GL_BLEND);
}

void desenharCascaDeBanana() {
    int segments = 10;
    glPushMatrix();
    glScalef(0.6, 0.6, 0.6);

    // 1. Desenha as 3 pétalas da casca
    for (int p = 0; p < 3; ++p) {
        glPushMatrix();
        glRotatef(p * 120.0f, 0.0f, 1.0f, 0.0f); // Gira cada pétala
        glBegin(GL_QUAD_STRIP);
        for (int i = 0; i <= segments; ++i) {
            float t = (float)i / segments;
            float y = -0.5f * t * t;
            float z = 1.5f * t;
            
            // Gradiente de cor: amarelo para marrom
            float r = 0.9f - (0.5f * t);
            float g = 0.8f - (0.6f * t);
            float b = 0.2f - (0.1f * t);

            glColor3f(r, g, b);
            glVertex3f(0.1f, y, z);
            glVertex3f(-0.1f, y, z);
        }
        glEnd();
        glPopMatrix();
    }
    
    // 2. Adiciona a base marrom no CENTRO
    glPushMatrix();
    glColor3f(0.3f, 0.15f, 0.05f);
    glutSolidSphere(0.15f, 8, 8);
    glPopMatrix();

    glPopMatrix();
}

// --- Função de desenho do lixo ---
void desenharFormaLixo(const LixoItem &lixo) {
    float r, g, b;

    // Define cores que representam melhor cada material
    switch (lixo.tipo) {
        case METAL:
            r = 0.75f; g = 0.75f; b = 0.8f; // Cinza prateado
            break;
        case PAPEL:
            r = 0.9f; g = 0.85f; b = 0.7f; // Bege/Branco sujo
            break;
        case PLASTICO:
            r = 1.0f; g = 0.1f; b = 0.1f;
            break;
        case VIDRO:
            r = 0.6f; g = 0.9f; b = 0.7f; // Verde claro
            break;
        case ORGANICO:
            r = 0.9f; g = 0.8f; b = 0.2f; // Amarelo da banana
            break;
        default:
            r = 0.5f; g = 0.5f; b = 0.5f;
            break;
    }
    glColor3f(r, g, b); // Define a cor base do objeto

    // Desenha a geometria correspondente
    switch (lixo.tipo) {
        case METAL:    desenharLataAmassada(); break;
        case PAPEL:    desenharPapelAmassado(lixo.seed); break;
        case PLASTICO: desenharGarrafaPlasticaAmassada(); break;
        case VIDRO:    desenharGarrafaDeVidroQuebrada(); break;
        case ORGANICO: desenharCascaDeBanana(); break;
    }
}

void GerarNovoLixo() {
    if (lixosNaEsteira.size() > 15) return;
    LixoItem novoLixo;
    TipoLixo tipoGerado;

    // Lógica para evitar mais de 4 repetições seguidas
    do {
        tipoGerado = static_cast<TipoLixo>(distrib(gen));
    } while (tipoGerado == ultimoLixoGerado && contadorRepeticao >= 4);

    // Atualiza o contador de repetição
    if (tipoGerado == ultimoLixoGerado) {
        contadorRepeticao++;
    } else {
        contadorRepeticao = 1;
    }
    
    ultimoLixoGerado = tipoGerado;
    
    novoLixo.tipo = tipoGerado;
    novoLixo.posZ = TRILHA_Z_POSITIONS[novoLixo.tipo];
    novoLixo.posX = LIXO_START_X;
    novoLixo.ativo = true;
    novoLixo.seed = distrib(gen);
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
void carregarTextura(const std::string &nome, const std::string &caminhoDaImagem)
{
    std::vector<unsigned char> image;
    unsigned int width, height;
    unsigned int error = lodepng::decode(image, width, height, caminhoDaImagem);

    if (error)
    {
        texturasDoJogo[nome] = 0; // Store 0 to indicate failure
        return;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Changed to GL_REPEAT for things like conveyor
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Changed to GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture

    texturasDoJogo[nome] = textureID; // Store the loaded texture ID
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
    if (texturasDoJogo["backgroundMenu"] != 0)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturasDoJogo["backgroundMenu"]);

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
    if (texturasDoJogo["backgroundMenu"] != 0)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturasDoJogo["backgroundMenu"]);

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

    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 800.0, 0.0, 600.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (texturasDoJogo["backgroundGame"] != 0)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturasDoJogo["backgroundGame"]);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(0.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f(800.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f(800.0f, 600.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(0.0f, 600.0f);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);

    gluLookAt(18.0, 7.0, 0.0,
          0.0, 0.0, 0.0,   
          0.0, 1.0, 0.0);
    // --- Desenhar a Esteira Rolante (Base) ---
    glPushMatrix();
    glTranslatef(0.0, -0.5, 0.0);
    desenharCubo(19.0, 1.0, 6.0, 0.6, 0.6, 0.6);
    glPopMatrix();

    // --- Desenhar a Esteira Rolante (Parte Superior com Transparência) - Trecho gerado por IA ---
    glPushMatrix();
    glTranslatef(0.0, 0.0, 0.0);
    glTranslated(0.0, 0.0, 0.0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float beltLength = 19.0f;
    float beltWidth = 5.0f;
    float beltHeight = 0.2f;
    float startX = -beltLength / 2.0f;
    float endX = beltLength / 2.0f;

    int numSegments = 1000;
    float segmentLength = beltLength / numSegments;

    for (int i = 0; i < numSegments; ++i)
    {
        float currentSegmentX = startX + i * segmentLength;
        float nextSegmentX = currentSegmentX + segmentLength;
        float alpha = 1.0f;
        if (currentSegmentX < -5.0f)
        {
            alpha = (currentSegmentX - LIXO_START_X) / (-5.0f - LIXO_START_X);
            if (alpha < 0.0f)
                alpha = 0.0f;
            if (alpha > 1.0f)
                alpha = 1.0f;
        }

        glColor4f(0.3f, 0.3f, 0.3f, alpha);

        glBegin(GL_QUADS);
        glVertex3f(currentSegmentX, beltHeight / 2.0f, -beltWidth / 2.0f);
        glVertex3f(nextSegmentX, beltHeight / 2.0f, -beltWidth / 2.0f);
        glVertex3f(nextSegmentX, beltHeight / 2.0f, beltWidth / 2.0f);
        glVertex3f(currentSegmentX, beltHeight / 2.0f, beltWidth / 2.0f);

        glColor4f(1.0f, 1.0f, 1.0f, alpha);
        glVertex3f(currentSegmentX, beltHeight / 2.0f, beltWidth / 2.0f);
        glVertex3f(nextSegmentX, beltHeight / 2.0f, beltWidth / 2.0f);
        glVertex3f(nextSegmentX, -beltHeight / 2.0f, beltWidth / 2.0f);
        glVertex3f(currentSegmentX, -beltHeight / 2.0f, beltWidth / 2.0f);

        glVertex3f(currentSegmentX, -beltHeight / 2.0f, -beltWidth / 2.0f);
        glVertex3f(nextSegmentX, -beltHeight / 2.0f, -beltWidth / 2.0f);
        glVertex3f(nextSegmentX, beltHeight / 2.0f, -beltWidth / 2.0f);
        glVertex3f(currentSegmentX, beltHeight / 2.0f, -beltWidth / 2.0f);

        if (i == 0)
        {
            glColor4f(0.2f, 0.2f, 0.2f, alpha);
            glVertex3f(startX, beltHeight / 2.0f, -beltWidth / 2.0f);
            glVertex3f(startX, beltHeight / 2.0f, beltWidth / 2.0f);
            glVertex3f(startX, -beltHeight / 2.0f, beltWidth / 2.0f);
            glVertex3f(startX, -beltHeight / 2.0f, -beltWidth / 2.0f);
        }

        if (i == numSegments - 1)
        {
            glColor4f(0.2f, 0.2f, 0.2f, alpha);
            glVertex3f(endX, beltHeight / 2.0f, -beltWidth / 2.0f);
            glVertex3f(endX, beltHeight / 2.0f, beltWidth / 2.0f);
            glVertex3f(endX, -beltHeight / 2.0f, beltWidth / 2.0f);
            glVertex3f(endX, -beltHeight / 2.0f, -beltWidth / 2.0f);
        }
        glEnd();
    }

    glDisable(GL_BLEND);
    glPopMatrix();

    // --- Lixeiras com texturas em 3D ---
    const float posYBaldes = -0.7f;
    const float posZBaldes = 10.0f;
    const float posXInicialBaldes = -2.0f;
    const float espacamentoBaldes = 1.0f;
    const float raio = 0.4f;
    const float altura = 0.8f;
    const float raioBuraco = 0.35f;

    const char *nomesCorpos[] = {
        "metal", "papel", "plastico", "vidro", "organico"};

    for (int i = 0; i < 5; i++)
    {
        glPushMatrix();
        glTranslatef(posZBaldes, posYBaldes, posXInicialBaldes + i * espacamentoBaldes);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

        GLUquadric *quad = gluNewQuadric();
        gluQuadricTexture(quad, GL_TRUE);
        gluQuadricNormals(quad, GLU_SMOOTH);

        if (texturasDoJogo[nomesCorpos[i]] != 0)
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texturasDoJogo[nomesCorpos[i]]);
        }
        glColor3f(1.0f, 1.0f, 1.0f);
        gluCylinder(quad, raio, raio, altura, 32, 32);
        glDisable(GL_TEXTURE_2D);

        glPushMatrix();
        glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
        gluDisk(quad, 0.0, raio, 32, 1);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.0f, 0.0f, altura);
        glColor3f(1.0f, 1.0f, 1.0f);
        gluDisk(quad, raioBuraco, raio, 32, 1);
        glPopMatrix();
        glDisable(GL_TEXTURE_2D);

        gluDeleteQuadric(quad);
        glPopMatrix();
    }

    if (efeitoAuraAtivo && lixeiraAuraIndex != -1)
    {
        // Calcula o progresso do efeito (de 0.0 a 1.0)
        float progresso = 1.0f - ((float)duracaoAura / DURACAO_MAX_AURA);
        
        // A aura começa do tamanho da lixeira e expande
        float raioAura = 0.4f + progresso * 0.4f; 
        // A aura começa visível e desaparece (fade out)
        float alphaAura = (1.0f - progresso) * 0.7f;

        // Pega a posição da lixeira que terá o efeito
        float posX = 10.0f;
        float posY = -0.7f;
        float posZ = TRILHA_Z_POSITIONS[lixeiraAuraIndex];

        glPushMatrix();
        glTranslatef(posX, posY, posZ);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); // Alinha o cilindro com a lixeira

        // Configurações para desenhar objeto transparente
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE); // Desabilita a escrita no Z-buffer

        glColor4f(corAura[0], corAura[1], corAura[2], alphaAura);
        
        GLUquadric* quad = gluNewQuadric();
        gluCylinder(quad, raioAura, raioAura, 0.8f, 20, 1); // Desenha o cilindro da aura
        gluDeleteQuadric(quad);

        // Restaura as configurações normais
        glDepthMask(GL_TRUE); // Reabilita a escrita no Z-buffer
        glDisable(GL_BLEND);
        glPopMatrix();
    }


    // --- Desenhar Lixo (Exemplo de um cubo para lixo) ---
    for (const auto &lixo : lixosNaEsteira)
    {
        if (lixo.ativo)
        {
            glPushMatrix();
            glTranslatef(lixo.posX, 0.5f, lixo.posZ); // 0.5f é a altura na esteira
            desenharFormaLixo(lixo);
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
    desenharTexto(10, 575, ssPontuacao.str().c_str(), 0.0, 1.0, 0.0); // Verde

    std::stringstream ssNivel;
    ssNivel << "NIVEL: " << nivel;
    desenharTexto(10, 555, ssNivel.str().c_str(), 1.0, 1.0, 1.0); // Branco

    std::stringstream ssVida;
    ssVida << "VIDAS: " << vida;
    desenharTexto(10, 535, ssVida.str().c_str(), 1.0, 0.0, 0.0); // Vermelho

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
    // Lógica para controlar a duração do flash
    if (efeitoAuraAtivo) {
        duracaoAura--;
        if (duracaoAura <= 0) {
            efeitoAuraAtivo = false;
            lixeiraAuraIndex = -1;
        }
    }

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
    bool erroCometido = false;
    switch (estadoAtual)
    {
    case MENU_INICIAL:
        if (key == 13) { estadoAtual = JOGO; ResetarJogo(); }
        else if (key == 8) { PlaySound(NULL, NULL, 0); exit(0); }
        break;

    case JOGO:
        TipoLixo tipoSelecionado;
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

            // Procura por um lixo na zona de coleta para interagir
            for (auto &lixo : lixosNaEsteira)
            {
                if (lixo.ativo && lixo.posX >= ZONA_COLETA_INICIO && lixo.posX <= ZONA_COLETA_FIM)
                {
                    if (lixo.tipo == tipoSelecionado)
                    {
                        // --- ACERTO ---
                        pontuacao += PONTOS_POR_ACERTO;
                        lixo.ativo = false;
                        acertoRealizado = true;

                        // Ativa a aura VERDE na lixeira CORRETA
                        efeitoAuraAtivo = true;
                        duracaoAura = DURACAO_MAX_AURA;
                        lixeiraAuraIndex = lixo.tipo;
                        corAura[0] = 0.6f; corAura[1] = 1.0f; corAura[2] = 0.6f;

                        if (nivel < MAX_NIVEL && pontuacao >= nivel * PONTOS_POR_NIVEL) {
                            nivel++;
                            velocidadeEsteira = velocidadesNivel[nivel - 1];
                        }
                        if (pontuacao >= MAX_PONTUACAO) estadoAtual = GAME_OVER;
                        
                        break; // Sai do loop pois já processou o acerto
                    }
                    else
                    {
                        // --- ERRO (tecla errada para o lixo na zona) ---
                        erroCometido = true;
                        break;
                    }
                }
            }

            // Processa o resultado da jogada
            if (acertoRealizado) {

            } else if (erroCometido) {
                vida--;
                // Ativa a aura VERMELHA na lixeira que o jogador apertou
                efeitoAuraAtivo = true;
                duracaoAura = DURACAO_MAX_AURA;
                lixeiraAuraIndex = tipoSelecionado;
                corAura[0] = 1.0f; corAura[1] = 0.5f; corAura[2] = 0.5f;
            } else {
                // Erro por apertar sem ter lixo na zona
                vida--;
                // Ativa a aura VERMELHA na lixeira que o jogador apertou
                efeitoAuraAtivo = true;
                duracaoAura = DURACAO_MAX_AURA;
                lixeiraAuraIndex = tipoSelecionado;
                corAura[0] = 1.0f; corAura[1] = 0.5f; corAura[2] = 0.5f;
            }

            if (vida < 1) estadoAtual = GAME_OVER;
        }
        break;

    case GAME_OVER:
        if (key == 13) { estadoAtual = MENU_INICIAL; }
        else if (key == 8) { PlaySound(NULL, NULL, 0); exit(0); }
        break;
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
    Inicializa();
    carregarTextura("backgroundMenu", "./resources/backgroundMenu.png");
    carregarTextura("backgroundGame", "./resources/backgroundGame.png");
    carregarTextura("metal", "./resources/lixeiraAmarela.png");
    carregarTextura("papel", "./resources/lixeiraAzul.png");
    carregarTextura("plastico", "./resources/lixeiraVermelha.png");
    carregarTextura("vidro", "./resources/lixeiraVerde.png");
    carregarTextura("organico", "./resources/lixeiraMarrom.png");

    glutDisplayFunc(Desenha);

    glutKeyboardFunc(Teclado);
    glutSpecialFunc(TecladoEspecial);
    glutMouseFunc(Mouse);
    PlaySound(MUSICA_JOGO, NULL, SND_ASYNC | SND_LOOP);

    glutTimerFunc(0, Anima, 0);
    glutMainLoop();
    return 0;
}