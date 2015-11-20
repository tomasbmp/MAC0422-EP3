/*

MAC0422 - Sistemas Operacionais
    EP2 - 05/10/2015

Guilherme Souto Schützer     - 8658544
Tomás Marcondes Bezerra Paim - 7157602

*/

#include <stdio.h>
#include "io.h"

void imprimeBitmap (){
	int i, j;
	for(i = 0; i < MAPSIZE; i++){
		for(j = 0; j < 8; j++)
			printf("%d", devolveBit(j, bitmap[i]));
	}
	printf("\n");

}