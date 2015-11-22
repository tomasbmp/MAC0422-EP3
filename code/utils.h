/*

MAC0422 - Sistemas Operacionais
    EP3 - 22/11/2015

Guilherme Souto Schützer     - 8658544
Tomás Marcondes Bezerra Paim - 7157602

*/

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define BLOCKSIZE 4000
#define MAPSIZE 3125
#define ARQPERBLOCK (BLOCKSIZE/sizeof(Arquivo))
#define WASTESEEK MAPSIZE
#define LIVRESSEEK (MAPSIZE+4)
#define QTYDSEEK (MAPSIZE+8)
#define QTYFSEEK (MAPSIZE+12)
#define ROOTSEEK (MAPSIZE+16)
#define MAXCHAR 256
#define TRUE 1
#define FALSE 0
#define PAI 0
#define FINAL 1

extern int fat[8*MAPSIZE], wasted, livres, qtyF, qtyD; /* 8*MAPSIZE e a quantidade de blocos em um sistema com 100Mb */
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
  int     bloco;
  int 		diretorio;
};


void addF();
void addD();
void rmF();
void rmD();

char** tokenize(char *str, char *separator);

unsigned char setBit(int posicao, unsigned char byte, int bit);

void setBloco(int bloco, int bit);

int devolveBit(int posicao, unsigned char byte);

int procuraBloco();

int qtyBlocks(int arquivos);

void setFAT(int conteudo, int posicao);
