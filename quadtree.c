#include "quadtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h> /* OpenGL functions */
#endif

unsigned int first = 1;
char desenhaBorda = 1;

int maxHeight = 0; // eu odeio matriz em C
int maxWidth = 0;

QuadNode* newNode(int x, int y, int width, int height)
{
    QuadNode* n = malloc(sizeof(QuadNode));
    n->x = x;
    n->y = y;
    n->width = width;
    n->height = height;
    n->NW = n->NE = n->SW = n->SE = NULL;
    n->color[0] = n->color[1] = n->color[2] = 0;
    n->id = first++;
    return n;
}


QuadNode* geraQuadtree(Img* pic, float minError)
{
    //CONVERTE P RGBPIXEL
    RGBPixel (*pixels)[pic->width] = (RGBPixel(*)[pic->height]) pic->img;

    int width = pic->width;
    int height = pic->height;

    maxHeight = height;
    maxWidth = width; // colocar isso p poder passar a imagem cinza como parametro pqp

    int imagemCinza[maxHeight][maxWidth];

    //deixa a imagem cinza pq o olho humano ve mto verde
    for(int i =0; i< height; i++){
        for(int j = 0; j< width; j++){
            imagemCinza[i][j] =  pixels[i][j].r * 0.3 + pixels[i][j].g * 0.59 + pixels[i][j].b * 0.11;
        }
    }

    QuadNode* raiz = recursao(pic,0,0,width,height,minError,imagemCinza); //cria o primeiro nodo o pai de todos amém

    return raiz;
}

QuadNode* recursao(Img* pic, int x, int y, int width, int height, float minError, int imagemCinza[maxHeight][maxWidth]){
    //bota dnv isso
    RGBPixel (*pixels)[pic->width] = (RGBPixel(*)[pic->height]) pic->img;

    int totalPixeis = height * width;

    int meiaWidth = width/2;
    int meiaHeight = height/2;
    
    //calcula cor media
    int totalR = 0;
    int totalG = 0;
    int totalB = 0;

    for(int i = y; i<height; i++){
        for(int j = x; x<width; j++){
            totalR = totalR + pixels[i][j].r;
            totalG = totalG + pixels[i][j].g;
            totalB = totalB + pixels[i][j].b;
        }
    }
    
    totalR = totalR / totalPixeis;
    totalG = totalG / totalPixeis;
    totalB = totalB / totalPixeis;
    
    //na duvida coloca rgbpixel -> mudaria se fosse array? nos outros testes não...
    RGBPixel mediaCores;
    mediaCores.r = totalR;
    mediaCores.g = totalG;
    mediaCores.b = totalB;


    //calcula o erro
    int histograma[256] = 0;
    long erro = 0; 

    for(int i = y; i< height + y; i++){
        for(int j = x; j<width + x; j++){
            int valorDoPixel = imagemCinza[i][j];
            histograma[valorDoPixel] = histograma[valorDoPixel] + 1;
        }
    }

    int intensidade = 0;
    // Para tanto, deve-se fazer um somatório de cada entrada do histograma multiplicada por sua frequência -> tem que multiplica por i, não somar 
    for(int i =0; i<256; i++) {
        intensidade = i * histograma[i];
    }

    intensidade = intensidade / totalPixeis;

    for(int i = y; i< height + y; i++){
        for(int j = x; j<width + x; j++){
            erro += (imagemCinza[i][j] - intensidade) * (imagemCinza[i][j] - intensidade); // me recuso a importar math pra uma potenciação
        }
    }


    //A seguir, divide-se essa soma pelo total de pixels da região -> entra raiz no meio 

    erro = sqrt(erro / totalPixeis); // o mal venceu

    QuadNode* raiz = newNode(x,y,width,height);

    if(erro > minError){
        raiz->status = PARCIAL;
        raiz->NW = recursao(pic, x, y, meiaWidth, meiaHeight, minError, imagemCinza[maxHeight][maxWidth]);
        raiz->NE = recursao(pic, x + meiaWidth, y, meiaWidth, meiaHeight, minError, imagemCinza[maxHeight][maxWidth]);
        raiz->SW = recursao(pic, x, y + meiaHeight, meiaWidth, meiaHeight, minError, imagemCinza[maxHeight][maxWidth]);
        raiz->NE = recursao(pic, x + meiaWidth, y + meiaHeight, meiaWidth, meiaHeight, minError, imagemCinza[maxHeight][maxWidth]);
    }
    else {
        raiz->status = CHEIO;
    }
    return raiz;
}

// Limpa a memória ocupada pela árvore
void clearTree(QuadNode* n)
{
    if(n == NULL) return;
    if(n->status == PARCIAL)
    {
        clearTree(n->NE);
        clearTree(n->NW);
        clearTree(n->SE);
        clearTree(n->SW);
    }
    //printf("Liberando... %d - %.2f %.2f %.2f %.2f\n", n->status, n->x, n->y, n->width, n->height);
    free(n);
}

// Ativa/desativa o desenho das bordas de cada região
void toggleBorder() {
    desenhaBorda = !desenhaBorda;
    printf("Desenhando borda: %s\n", desenhaBorda ? "SIM" : "NÃO");
}

// Desenha toda a quadtree
void drawTree(QuadNode* raiz) {
    if(raiz != NULL)
        drawNode(raiz);
}

// Grava a árvore no formato do Graphviz
void writeTree(QuadNode* raiz) {
    FILE* fp = fopen("quad.dot", "w");
    fprintf(fp, "digraph quadtree {\n");
    if (raiz != NULL)
        writeNode(fp, raiz);
    fprintf(fp, "}\n");
    fclose(fp);
    printf("\nFim!\n");
}

void writeNode(FILE* fp, QuadNode* n)
{
    if(n == NULL) return;

    if(n->NE != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->NE->id);
    if(n->NW != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->NW->id);
    if(n->SE != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->SE->id);
    if(n->SW != NULL) fprintf(fp, "%d -> %d;\n", n->id, n->SW->id);
    writeNode(fp, n->NE);
    writeNode(fp, n->NW);
    writeNode(fp, n->SE);
    writeNode(fp, n->SW);
}

// Desenha todos os nodos da quadtree, recursivamente
void drawNode(QuadNode* n)
{
    if(n == NULL) return;

    glLineWidth(0.1);

    if(n->status == CHEIO) {
        glBegin(GL_QUADS);
        glColor3ubv(n->color);
        glVertex2f(n->x, n->y);
        glVertex2f(n->x+n->width-1, n->y);
        glVertex2f(n->x+n->width-1, n->y+n->height-1);
        glVertex2f(n->x, n->y+n->height-1);
        glEnd();
    }

    else if(n->status == PARCIAL)
    {
        if(desenhaBorda) {
            glBegin(GL_LINE_LOOP);
            glColor3ubv(n->color);
            glVertex2f(n->x, n->y);
            glVertex2f(n->x+n->width-1, n->y);
            glVertex2f(n->x+n->width-1, n->y+n->height-1);
            glVertex2f(n->x, n->y+n->height-1);
            glEnd();
        }
        drawNode(n->NE);
        drawNode(n->NW);
        drawNode(n->SE);
        drawNode(n->SW);
    }
    // Nodos vazios não precisam ser desenhados... nem armazenados!
}

