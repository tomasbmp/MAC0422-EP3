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
#include "utils.h"

int main(){
  char*  input, shell_prompt[MAXCHAR];
  char** argv = NULL;
  
  while(1) {

    snprintf(shell_prompt, sizeof(shell_prompt), "[ep3]: ");
    input = readline(shell_prompt);

    add_history(input);
    argv = tokenize(input);
    /* free(input);
    input = NULL; */

  	if (strcmp(argv[0], "mount") == 0) {

  	}
    else if (strcmp(argv[0], "cp") == 0){

    }
  	else if (strcmp(argv[0], "mkdir") == 0) {

  	}
  	else if (strcmp(argv[0], "rmdir") == 0) {

  	}
    else if (strcmp(argv[0], "cat") == 0) {

    }
    else if (strcmp(argv[0], "touch") == 0) {

  	}
  	else if (strcmp(argv[0], "rm") == 0) {

  	}
  	else if (strcmp(argv[0], "ls") == 0) {
  		
  	}
  	else if (strcmp(argv[0], "find") == 0) {
  		
  	}
  	else if (strcmp(argv[0], "df") == 0) {
  		
  	}
  	else if (strcmp(argv[0], "umount") == 0) {
  		
  	}
    else if (strcmp(argv[0], "sai") == 0) {
  		printf("Adeus.\n");
  		break;
  	}

		else {
			printf("Lista de comandos:\n");
			printf("mount <arquivo>    			-- monta o sistema de arquivos contido em <arquivo>\n");
			printf("cp <origem> <destino>      	-- cria uma copia do arquivo <origem> em <destino>\n");
			printf("mkdir <diretorio>   		-- cria o diretorio <diretorio>\n");
			printf("rmdir <diretorio>           -- remove o diretorio <diretorio>\n");
			printf("cat <arquivo> 				-- mostra o conteudo de <arquivo>\n");
			printf("touch <arquivo> 			-- atualiza o ultimo acesso de <arquivo> para o instante atual");
			printf("rm <arquivo> 				-- remove o arquivo <arquivo>");
			printf("ls <diretorio> 				-- lista tudo abaixo do diretorio <diretorio>");
			printf("find <diretorio> <arquivo> 	-- busca a partir de <diretorio> por <arquivo>");
			printf("df 							-- imprime informacoes do sistema de arquivos");
			printf("umount 						-- desmonta o sistema de arquivos");
			printf("sai               			-- encerra o programa\n");
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
