
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
