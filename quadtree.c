#include "quadtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>     /* OpenGL functions */
#endif

unsigned int first = 1;
char desenhaBorda = 1;

Img* cinza = NULL;

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
    // Converte o vetor RGBPixel para uma MATRIZ que pode acessada por pixels[linha][coluna]
    RGBPixel (*pixels)[pic->width] = (RGBPixel(*)[pic->height]) pic->img;

    int width = pic->width;
    int height = pic->height;

    // HISTOGRAMA

    int histograma [256];

    //Como a conversão para cinza é somente para o histograma, não tem pq salvar ela ; coloca na posição do tom de cinza uma frequência a mais.
    for(int i = 0; i<height; i++){
        for(int j = 0; j<width; j++){
            int vermelho = round(pixels[i][j].r * 0.3);
            int verde = round(pixels[i][j].g * 0.59);
            int azul = round(pixels[i][j].b * 0.11);

            histograma[vermelho+azul+verde] = histograma[vermelho+azul+verde] + 1;
        }
    }

    //VARIAVEIS REFERENTE AO HISTOGRAMA DA REGIÃO
    int histograma [256];
    int intensidadeMediaCinza = 0;
    
    //VARIAVEIS REFERENTE A COR DA REGIÃO
    int nR =0;
    int nG =0;
    int nB =0;


    //ARRAY DE PIXELS
    RGBPixel (*pixels)[pic->width] = (RGBPixel(*)[pic->height]) pic->img;
    
    //**CALCULO DO HISTOGRAMA**

    //Como a conversão para cinza é somente para o histograma, não tem pq salvar ela ; coloca na posição do tom de cinza uma frequência a mais. 
    // aproveitando o for para salvar quantidade de cada cor
    for(int i = 0; i<height; i++){
        for(int j = 0; j<width; j++){

            nR += pixels[i][j].r;
            nG += pixels[i][j].g;
            nB += pixels[i][j].b;

            int vermelho = round(pixels[i][j].r * 0.3);
            int verde = round(pixels[i][j].g * 0.59);
            int azul = round(pixels[i][j].b * 0.11);

            histograma[vermelho+azul+verde] = histograma[vermelho+azul+verde] + 1;
        }
    }

    //**Calculo de nivel de erro da região **

    for(int i = 0 ; i<256; i++){
        intensidadeMediaCinza += histograma[i]; 
    }

    intensidadeMediaCinza = round(intensidadeMediaCinza/256);
    
    int somatorio1= 0;
    // relembrando que o somatorio1 consiste na soma do somatorio2, ou seja a soma de todas as somas de somatorio2
    int somatorio2 = 0;

    for(int sum1 = 0; sum1<width-1; sum1++){
        for(int sum2 = 0; sum2<height-1; sum2++){
            //ficou muito feito, talvez melhorar? -> é p ser ao quadrado, mas eu n confio em C
            somatorio2 += (round((pixels[sum1][sum2].r * 0.3 + pixels[sum1][sum2].g * 0.59 + pixels[sum1][sum2].b * 0.11) - intensidadeMediaCinza)) * 
            (round((pixels[sum1][sum2].r * 0.3 + pixels[sum1][sum2].g * 0.59 + pixels[sum1][sum2].b * 0.11) - intensidadeMediaCinza));
        }
        somatorio1 += somatorio2;
    }

    //calculo do ngc -> como somatorio1 é a soma de todos os somatorios2, usa só ele
    int nivelErro = round(sqrt(
        1/width*height * somatorio1
    ));
    

    //**MEDIA DA REGIÃO**

    int mediaCor = round((nR + nG + nB)/3);


    //////////////////////////////////////////////////////////////////////////
    // Implemente aqui o algoritmo que gera a quadtree, retornando o nodo raiz
    //////////////////////////////////////////////////////////////////////////
    QuadNode* raiz = newNode(0,0,width,height);
    int meiaLargura = width;
    int meiaAltura = height;

    raiz->status = PARCIAL;
    raiz->color[0] = 0;
    raiz->color[1] = 0;
    raiz->color[2] = 255;

    meiaLargura = meiaLargura/2;
    meiaAltura = meiaAltura/2;

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

