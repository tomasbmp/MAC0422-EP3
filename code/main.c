/*

MAC0422 - Sistemas Operacionais
    EP2 - 05/10/2015

Guilherme Souto Schützer     - 8658544
Tomas Marcondes Bezerra Paim - 7157602

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "io.h"

int main(){
  char*  input, shell_prompt[MAXCHAR];
  char** argv = NULL;
  int i, FAT[8*MAPSIZE], wasted; /* 25000 e a quantidade de blocos em um sistema com 100Mb */
  time_t rawtime;
  int mounted = FALSE;
  unsigned char bitmap[MAPSIZE], c; /* suficiente para armazenar 100Mb */
  FILE *unidade;

  unidade = NULL;

  rawtime = time(NULL);
  printf(ctime(&rawtime));
  
  while(1) {

    snprintf(shell_prompt, sizeof(shell_prompt), "[ep3]: ");
    input = readline(shell_prompt);

    add_history(input);
    argv = tokenize(input);
    /* free(input);
    input = NULL; */



  	if (strcmp(argv[0], "mount") == 0) {
      if(mounted == FALSE){
          unidade = fopen(argv[1], "r+");
          if(unidade == NULL){ /* arquivo não existe e deve ser criado */
            unidade = fopen(argv[1], "w+");
            if (unidade == NULL) {
              printf ("ERRO: Unidade nao pode ser criada.\n");
              return -1;
            }
            printf("Unidade nao encontrada. Criando nova unidade...");
            bitmap[0] = 0;
            bitmap[0] = setBit(0, bitmap[0], 1);
            fwrite (&bitmap[0], sizeof(bitmap[0]), 1, unidade);

            for (i = 1; i < MAPSIZE; i++) { /* seta o bitmap */
              bitmap[i] = 0;
              fwrite (&bitmap[i], sizeof(bitmap[i]), 1, unidade);
            }
            wasted = 871; /* quantidade de bytes nao utilizados neste primeiro bloco, */
                          /* menos 4 bytes necessarios para armazenar a variavel wastes */

            for(i = i; i < 100000000; i++){ /* cria o arquivo inteiro com 100Mb */
              fwrite (&bitmap[1], sizeof(bitmap[1]), 1, unidade); 
            }

            fseek(unidade, MAPSIZE, SEEK_SET);
            fwrite(&wasted, sizeof(wasted), 1, unidade);
            printf(" Unidade criada com sucesso!\n");
          }
          else { /* realiza os procedimentos para retomar uma unidade */
            printf("Estou retomando uma unidade criada anteriormente.\n");
            fseek(unidade, 0, SEEK_SET);
            for(i = 0; i < MAPSIZE; i++)
              fread(&bitmap[i], sizeof(char), 1, unidade);

            printf("%d\n", bitmap[0]);
          } 
          mounted = TRUE;
      }
      else printf("Desmonte o sistema de arquivos atual para poder montar outro.\n");

  	}
    else if (strcmp(argv[0], "cp") == 0){
      if(mounted == FALSE) printf("Monte um arquivo antes de realizar este comando.\n");
      else {
      }

    }
  	else if (strcmp(argv[0], "mkdir") == 0) {
      if(mounted == FALSE) printf("Monte um arquivo antes de realizar este comando.\n");
      else {}

  	}
  	else if (strcmp(argv[0], "rmdir") == 0) {
      if(mounted == FALSE) printf("Monte um arquivo antes de realizar este comando.\n");
      else {}

  	}
    else if (strcmp(argv[0], "cat") == 0) {
      if(mounted == FALSE) printf("Monte um arquivo antes de realizar este comando.\n");
      else {}

    }
    else if (strcmp(argv[0], "touch") == 0) {
      if(mounted == FALSE) printf("Monte um arquivo antes de realizar este comando.\n");
      else {}

  	}
  	else if (strcmp(argv[0], "rm") == 0) {
      if(mounted == FALSE) printf("Monte um arquivo antes de realizar este comando.\n");
      else {}

  	}
  	else if (strcmp(argv[0], "ls") == 0) {
      if(mounted == FALSE) printf("Monte um arquivo antes de realizar este comando.\n");
      else {}
  		
  	}
  	else if (strcmp(argv[0], "find") == 0) {
      if(mounted == FALSE) printf("Monte um arquivo antes de realizar este comando.\n");
      else {}
  		
  	}
  	else if (strcmp(argv[0], "df") == 0) {
      if(mounted == FALSE) printf("Monte um arquivo antes de realizar este comando.\n");
      else {}
  		
  	}
  	else if (strcmp(argv[0], "umount") == 0) {
      if(mounted == FALSE) printf("Monte um arquivo antes de realizar este comando.\n");
      else {
        mounted = FALSE;
        fclose(unidade);
        printf("Unidade desmontada com sucesso.\n");
      }
  		
  	}
    else if (strcmp(argv[0], "sai") == 0) {
  		printf("Adeus.\n");
  		break;
  	}

		else {
			printf("Lista de comandos:\n");
			printf("mount <arquivo>        			-- monta o sistema de arquivos contido em <arquivo>\n");
			printf("cp <origem> <destino>      	-- cria uma copia do arquivo <origem> em <destino>\n");
			printf("mkdir <diretorio>   	    	-- cria o diretorio <diretorio>\n");
			printf("rmdir <diretorio>           -- remove o diretorio <diretorio>\n");
			printf("cat <arquivo> 				      -- mostra o conteudo de <arquivo>\n");
			printf("touch <arquivo> 	      		-- atualiza o ultimo acesso de <arquivo> para o instante atual\n");
			printf("rm <arquivo> 				        -- remove o arquivo <arquivo>\n");
			printf("ls <diretorio> 			       	-- lista tudo abaixo do diretorio <diretorio>\n");
			printf("find <diretorio> <arquivo>  -- busca a partir de <diretorio> por <arquivo>\n");
			printf("df 							            -- imprime informacoes do sistema de arquivos\n");
			printf("umount 						          -- desmonta o sistema de arquivos\n");
			printf("sai               		     	-- encerra o programa\n");
		}

    if(argv != NULL){
      free(argv);
      argv = NULL;
    }
  }

  if(input != NULL)
    free(input);

  if(argv != NULL)
  	free(argv);

  return 0;
}
