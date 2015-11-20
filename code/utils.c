
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

char** tokenize(char *str, char *separator) {
  char** tokens = NULL;
  int i;

  tokens = malloc(sizeof (char*) * MAXCHAR);
  tokens[0] = strtok(str, separator);

  for(i = 1; tokens[i - 1] != NULL; i++) {
    tokens[i] = strtok(NULL, separator);
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

int devolveBit(int posicao, unsigned char byte) {
	byte >>= posicao;
	return byte%2;
}
