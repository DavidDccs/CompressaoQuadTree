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
int errominimo = minError;
//    printf("%d, minError do geraNode" , minError);
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
            imagemCinza[i][j] =  (pixels[i][j].r * 0.3) + (pixels[i][j].g * 0.59) + (pixels[i][j].b * 0.11);
        }
    }
//    printf("gera quad tree\n");

    QuadNode* raiz =newNode(0,0,width,height);
    recursao(raiz,pic,0,0,width,height,errominimo,imagemCinza); //cria o primeiro nodo o pai de todos amém

    return raiz;
}

void recursao(QuadNode* raiz, Img* pic, int x, int y, int width, int height, float minError, int imagemCinza[height][width]){

    int errominimo = minError;
    //bota dnv isso
    RGBPixel (*pixels)[pic->width] = (RGBPixel(*)[pic->height]) pic->img;
//    printf("entrou recursao \n");
    int totalPixeis = height * width;

    int meiaWidth = width/2;
    int meiaHeight = height/2;

    //calcula cor media
    int totalR = 0;
    int totalG = 0;
    int totalB = 0;
//    printf("antes bbbbbbbbbbbbbbbb\n ");

       for (int i = y; i < height + y; i++) {
        for (int j = x; j < width + x; j++) {
           //printf("%d j = \n", j);
            totalR = totalR + pixels[i][j].r;
            //printf("\n%d red ", totalR);
            totalG = totalG + pixels[i][j].g;
            //printf("\n%d green", totalG);
            totalB = totalB + pixels[i][j].b;
            //printf("\n%d blue", totalB);
        }
    }
       printf("numero de pix, %d", totalPixeis);
//    printf("antes totalRGB\n ");

    totalR = totalR / totalPixeis;
    totalG = totalG / totalPixeis;
    totalB = totalB / totalPixeis;
    
    //na duvida coloca rgbpixel -> mudaria se fosse array? nos outros testes não...

    raiz->color[0] = totalR;
    raiz->color[1] = totalG;
    raiz->color[2] = totalB;

//    printf("antes calcula erro\n ");

    //calcula o erro
    int histograma[256] = { 0 };
//    printf("uuuuuu\n");
    long erro = 0; 
  
    for (int i = y; i < height + y; i++){
        for (int j = x; j < width + x; j++){
           // int valorDoPixel = imagemCinza[i][j];
            histograma[imagemCinza[i][j]]++;
        }
    }

    int intensidade = 0;
    // Para tanto, deve-se fazer um somatório de cada entrada do histograma multiplicada por sua frequência -> tem que multiplica por i, não somar 
    for(int i =0; i<256; i++) {
        intensidade += i * histograma[i];
    }

    int intensidadeMedia = intensidade / totalPixeis;

    for(int i = y; i < height + y; i++){
        for(int j = x; j < width + x; j++){
            erro += pow(imagemCinza[i][j] - intensidadeMedia,2); // me recuso a importar math pra uma potenciação
        }
    }


    //A seguir, divide-se essa soma pelo total de pixels da região -> entra raiz no meio 

    long erroFinal = erro / totalPixeis; // o mal vence
    erroFinal = sqrt(erroFinal);
    printf("o erro ta como %d",erro);
//    printf("antes do if\n ");
    if( minError < erroFinal && meiaHeight > 0 && meiaWidth > 0){
//        printf("entou if\n ");

        raiz->NW = newNode(raiz->x, raiz->y, meiaWidth , meiaHeight);
        raiz->NE = newNode(raiz->x + meiaWidth, raiz->y, meiaWidth, meiaHeight);
        raiz->SW = newNode(raiz->x, raiz->y + meiaHeight, meiaWidth, meiaHeight);
        raiz->SE = newNode(raiz->x + meiaWidth, raiz->y + meiaHeight, meiaWidth, meiaHeight);

        if (meiaHeight == 1 || meiaWidth == 1){
            raiz->NE->status = CHEIO;
            raiz->NW->status = CHEIO;
            raiz->SW->status = CHEIO;
            raiz->SE->status = CHEIO;
            return;
        }
        raiz->status = PARCIAL;

        recursao(raiz->NW,pic, x, y, meiaWidth, meiaHeight, errominimo, imagemCinza);
        recursao(raiz->NE,pic, x + meiaWidth, y, meiaWidth, meiaHeight, errominimo, imagemCinza);
        recursao(raiz->SW,pic, x, y + meiaHeight, meiaWidth, meiaHeight, errominimo, imagemCinza);
        recursao(raiz->SE,pic, x + meiaWidth, y + meiaHeight, meiaWidth, meiaHeight, errominimo, imagemCinza);
    }
    else {
        raiz->status = CHEIO;
        return;
    }
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

