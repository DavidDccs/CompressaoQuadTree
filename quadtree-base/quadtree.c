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


QuadNode* criaQuadTree(QuadNode* inicio,float minErro,int height2, int width2, unsigned char matrizCinza[height2][width2], Img* pic){
    int width = inicio->width;
    int height = inicio->height;
    if(minErro<= minErro && width/2<=1 && height/2<=1) {
        inicio->NW = newNode(inicio->x, inicio->y, width / 2, height / 2);
        inicio->NE = newNode(inicio->x + width / 2, inicio->y, width / 2, height / 2);
        inicio->SW = newNode(inicio->x, inicio->y + height / 2, width / 2, height / 2);
        inicio->SE = newNode(inicio->x + width / 2, inicio->y + height / 2, width / 2, height / 2);

        //histograma
        unsigned char histograma[256];
        for (int i = inicio->y; i < height; i++) {
            for (int j = inicio->x; j < width; j++) {
                unsigned char hist = matrizCinza[i][j];
                histograma[hist]++;
            }
        }

        int totalPixels = height * width;

        //total RGB da região
        RGBPixel (*pixels)[pic->width] = (RGBPixel(*)[pic->height]) pic->img;
        int somatorioR = 0;
        int somatorioG = 0;
        int somatorioB = 0;

        for (int j = 0; j < pic->height; j++) {
            for (int i = 0; i < pic->width; i++) {
                somatorioR += pixels[0][i].r;
                somatorioG += pixels[0][i].g;
                somatorioB += pixels[0][i].b;
            }
        }
        int totalR = round(somatorioR / totalPixels);
        int totalG = round(somatorioG / totalPixels);
        int totalB = round(somatorioB / totalPixels);

        unsigned char color[3] = {totalR, totalG, totalB};
        inicio->color[0] = color[0];
        inicio->color[1] = color[1];
        inicio->color[2] = color[2];


        int somatorioIntensidade = 0;
        for (int i = 0; i < 256; i++) {
            somatorioIntensidade += i * ((int) histograma[i]);
        }

        double intensidadeMedia = somatorioIntensidade / totalPixels;


        double somatorioErro = 0.0;
        for (int i = inicio->y; i < height; i++) {
            for (int j = inicio->x; j < width; j++) {
                unsigned char aux = matrizCinza[i][j];
                somatorioErro += ((double) aux - intensidadeMedia) * ((double) aux - intensidadeMedia);
            }
        }

        double erro = sqrt(somatorioErro / totalPixels);


        inicio->status = PARCIAL;
        criaQuadTree(inicio->NW, minErro, height2, width2, matrizCinza, pic);
        criaQuadTree(inicio->NE, minErro, height2, width2, matrizCinza, pic);
        criaQuadTree(inicio->SW, minErro, height2, width2, matrizCinza, pic);
        criaQuadTree(inicio->SE, minErro, height2, width2, matrizCinza, pic);
        return inicio;
    }
    else{
        inicio->NW= NULL;
        inicio->NE= NULL;
        inicio->SW= NULL;
        inicio->SE= NULL;
        inicio->status= CHEIO;
        return;
    }
}

QuadNode* geraQuadtree(Img* pic, float minError)
{
    // Converte o vetor RGBPixel para uma MATRIZ que pode acessada por pixels[linha][coluna]
    RGBPixel (*pixels)[pic->width] = (RGBPixel(*)[pic->height]) pic->img;

    int i;
    unsigned char matrizNask[pic->height][pic->width];
    for (int j=0; j<pic->height;j++) {
    for (i = 0; i < pic->width; i++) {
        pixels[j][i].r = pixels[j][i].r * 0.3;
        pixels[j][i].g = pixels[j][i].g * 0.59;
        pixels[j][i].b = pixels[j][i].b * 0.11;
        matrizNask[j][i] = (unsigned char) round(pixels[j][i].r + pixels[j][i].g + pixels[j][i].b);
    }
    }


    int width = pic->width;
    int height = pic->height;

    QuadNode* raiz = criaQuadTree( newNode(0,0,width,height), minError,width,height,matrizNask, pic);
// COMENTE a linha abaixo quando seu algoritmo ja estiver funcionando
// Caso contrario, ele ira gerar uma arvore de teste com 3 nodos
//#define DEMO
#ifdef DEMO

    /************************************************************/
    /* Teste: criando uma raiz e dois nodos a mais              */
    /************************************************************/

    QuadNode* raiz = newNode(0,0,width,height);
    raiz->status = PARCIAL;
    raiz->color[0] = 0;
    raiz->color[1] = 0;
    raiz->color[2] = 255;

    int meiaLargura = width/2;
    int meiaAltura = height/2;

    QuadNode* nw = newNode(meiaLargura, 0, meiaLargura, meiaAltura);
    nw->status = PARCIAL;
    nw->color[0] = 0;
    nw->color[1] = 0;
    nw->color[2] = 255;

    // Aponta da raiz para o nodo nw
    raiz->NW = nw;

    QuadNode* nw2 = newNode(meiaLargura+meiaLargura/2, 0, meiaLargura/2, meiaAltura/2);
    nw2->status = CHEIO;
    nw2->color[0] = 255;
    nw2->color[1] = 0;
    nw2->color[2] = 0;

    // Aponta do nodo nw para o nodo nw2
    nw->NW = nw2;

#endif
    // Finalmente, retorna a raiz da árvore
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

