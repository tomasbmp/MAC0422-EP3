
/*

MAC0422 - Sistemas Operacionais
    EP2 - 05/10/2015

Guilherme Souto Schützer     - 8658544
Tomas Marcondes Bezerra Paim - 7157602

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

char** tokenize(char* str) {
  char** tokens = NULL;
  int i;

  tokens = malloc(sizeof (char*) * MAXCHAR);
  tokens[0] = strtok(str, " ");

  for(i = 1; tokens[i - 1] != NULL; i++) {
    tokens[i] = strtok(NULL, " ");
  }

  return tokens;
}

unsigned char setBit(int posicao, unsigned char byte, int bit) {
	unsigned char setter;
	
	setter = 1;
	setter <<= posicao;

	if (bit == 0) setter = ~setter;

	if (bit == 1) setter = setter|byte;
	else setter = setter&byte;

	return setter;
}

void setBloco(int bloco, int bit){
	int i, j;
	i = bloco/8;
	j = bloco%8;

	bitmap[i] = setBit(j, bitmap[i], bit);
	fseek(unidade, i, SEEK_SET);
	fwrite(&bitmap[i], sizeof(bitmap[i]), 1, unidade);
}

int devolveBit(int posicao, unsigned char byte) {
	byte >>= posicao;
	return byte%2;
}

int procuraBloco(){
  int i, j, livre = -1;
  for (i = 0; (i < MAPSIZE) && (livre < 0); i++){
    for (j = 0; (j < 8) && (livre < 0); j++){
      if (devolveBit(j, bitmap[i]) == 0)
        livre = 8*i+j;
    }
  }
  return livre;
}

void setFAT(int conteudo, int posicao){
	FAT[posicao] = conteudo;
	fseek(unidade, BLOCKSIZE+4*posicao, SEEK_SET);
	fwrite(&FAT[posicao], sizeof(int), 1, unidade);
}