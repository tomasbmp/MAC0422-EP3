/*

MAC0422 - Sistemas Operacionais
    EP2 - 05/10/2015

Guilherme Souto Schützer     - 8658544
Tomás Marcondes Bezerra Paim - 7157602

*/

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define MAXCHAR 1024
#define TRUE 1
#define FALSE 0

typedef struct arq Arq;
struct acesso
{
  char     *nome;
  int     tamBytes;
  time_t instCriado;
  time_t instModificado;
  time_t instAcessado;
};

char** tokenize(char* str);
