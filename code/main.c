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

int FAT[8*MAPSIZE], wasted, livres = 8*MAPSIZE, qtyF = 0, qtyD = 0; /* 8*MAPSIZE e a quantidade de blocos em um sistema com 100Mb */
unsigned char bitmap[MAPSIZE]; /* suficiente para armazenar 100Mb */
FILE *unidade;

int main(){
  char*  input, shell_prompt[MAXCHAR];
  char** argv = NULL;
  int i;
  time_t rawtime;
  int mounted = FALSE;
  Arquivo novo;

  unidade = NULL;

  printf("sizeof(Arquivo) = %d\n", sizeof(Arquivo));

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
            livres--;
            fwrite (&bitmap[0], sizeof(bitmap[0]), 1, unidade);

            for (i = 1; i < MAPSIZE; i++) { /* seta o bitmap */
              bitmap[i] = 0;
              fwrite (&bitmap[i], sizeof(bitmap[i]), 1, unidade);
            }
            wasted = BLOCKSIZE - MAPSIZE - 4 - 4 - 4 - 4; /* quantidade de bytes nao utilizados neste primeiro bloco, */
                          /* menos 4 bytes necessarios para armazenar a variavel wastes, */
                          /* menos 4 bytes necessarios para armazenar a variavel livres, */
                          /* menos 4 bytes necessarios para armazenar a variavel qtyD, */
                          /* menos 4 bytes necessarios para armazenar a variavel qtyF */

            for(; i < 100000000; i++){ /* cria o arquivo inteiro com 100Mb */
              fwrite (&bitmap[1], sizeof(bitmap[1]), 1, unidade); 
            }

            for(i = 0; i < MAPSIZE*8; i++){ /* inicializa todas as posicoes de FAT com -1 */
              setFAT(-1, i);
            }

            for(i = 1; i < 26; i++){ /* seta os blocos usados pela tabela FAT */
              setBloco(i, 1);
              livres--;
            }


            memcpy(novo.nome, "root\0", 5);
            novo.tamBytes = 0;
            novo.instCriado = time(NULL);
            novo.instModificado = novo.instCriado;
            novo.instAcessado = novo.instCriado;
            novo.diretorio = 0;
            novo.bloco = 26;

            qtyD++;

            wasted -= sizeof(Arquivo);

            setBloco(26, 1);
            livres--;

            fseek(unidade, MAPSIZE, SEEK_SET);
            fwrite(&wasted, sizeof(wasted), 1, unidade);  /* MAPSIZE */
            fwrite(&livres, sizeof(livres), 1, unidade);  /* MAPSIZE + 4 */
            fwrite(&qtyD, sizeof(qtyD), 1, unidade);      /* MAPSIZE + 8 */
            fwrite(&qtyF, sizeof(qtyF), 1, unidade);      /* MAPSIZE + 12 */
            fwrite(&novo, sizeof(Arquivo), 1, unidade);   /* MAPSIZE + 16 */

            printf(" Unidade criada com sucesso!\n");
            printf("livres = %d (deveria ser 24972\n", livres);
          }
          else { /* realiza os procedimentos para retomar uma unidade */
            printf("Estou retomando uma unidade criada anteriormente.\n");
            fseek(unidade, 0, SEEK_SET);
            for(i = 0; i < MAPSIZE; i++) fread(&bitmap[i], sizeof(char), 1, unidade);
            fread(&wasted, sizeof(wasted), 1, unidade);  /* MAPSIZE */
            fread(&livres, sizeof(livres), 1, unidade);  /* MAPSIZE + 4 */
            fread(&qtyD, sizeof(qtyD), 1, unidade);      /* MAPSIZE + 8 */
            fread(&qtyF, sizeof(qtyF), 1, unidade);      /* MAPSIZE + 12 */

            fseek(unidade, BLOCKSIZE, SEEK_SET);
            for (i = 0; i < MAPSIZE*8; i++) fread(&FAT[i], sizeof(int), 1, unidade);

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

  if(unidade != NULL)
    fclose(unidade);

  return 0;
}
