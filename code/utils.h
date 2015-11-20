/*

MAC0422 - Sistemas Operacionais
    EP2 - 05/10/2015

Guilherme Souto Schützer     - 8658544
Tomás Marcondes Bezerra Paim - 7157602

*/

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define BLOCKSIZE 4000
#define MAPSIZE 3125
#define MAXCHAR 1024
#define TRUE 1
#define FALSE 0

extern int FAT[8*MAPSIZE], wasted, livres, qtyF, qtyD; /* 8*MAPSIZE e a quantidade de blocos em um sistema com 100Mb */
extern unsigned char bitmap[MAPSIZE]; /* suficiente para armazenar 100Mb */
extern FILE *unidade;

typedef struct arquivo Arquivo;
struct arquivo
{
  char     	nome[13];
  int     	tamBytes;
  time_t 	instCriado;
  time_t 	instModificado;
  time_t 	instAcessado;
  int 		bloco;
  int 		diretorio;
};

char** tokenize(char* str);

unsigned char setBit(int posicao, unsigned char byte, int bit);

void setBloco(int bloco, int bit);

int devolveBit(int posicao, unsigned char byte);

int procuraBloco();

void setFAT(int conteudo, int posicao);
