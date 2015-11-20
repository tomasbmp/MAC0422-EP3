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

static Arquivo root;
static Arquivo erro;
static int wasted, livres;

/* funcao recursiva que retorna struct com dados do arquivo dado pelo caminho */
Arquivo getArquivoRec(FILE *unidade, int *fat, char **paradas, Arquivo diretorio, int i, int tamanho_adicional, int novo){
  Arquivo arq, erro, encontrado;
  int tamanho, bloco;
  int j;
  j = 0;

  tamanho = diretorio.tamBytes;
  bloco = diretorio.bloco;

  fseek(unidade, bloco * BLOCKSIZE, SEEK_SET);
  do{
    /* se reservamos mais do que o tamanho de um bloco,
    vê na tabela FAT o endereço do próximo */
    if(j > BLOCKSIZE) {
      j = 0;
      bloco = fat[bloco];
      /* proximo bloco "nao existe",
      condicao de parada: nao encontramos o arquivo */
      if(bloco == -1) return erro;
      fseek(unidade, bloco * BLOCKSIZE, SEEK_SET);
      tamanho -= BLOCKSIZE;
    }
    /* "reserva" o espaço a ser lido */
    j += sizeof(arq);
    /* se reservamos mais do que o tamanho total do diretorio,
    condicao de parada: nao encontramos o arquivo */
    if(j > tamanho) return erro;
    fread(&arq, sizeof(arq), 1, unidade);
  }while(strcmp(arq.nome, paradas[i]));
  /* encontramos o arquivo no diretorio */

  if(arq.diretorio >= 0){
    arq.tamBytes += tamanho_adicional;
    arq.instAcessado = time(NULL);
    arq.instModificado = arq.instAcessado;
    fseek(unidade, -sizeof(Arquivo), SEEK_CUR);
    fwrite(&arq, sizeof(Arquivo), 1, unidade);
  }

  i++;
  /* condicao de parada: e a ultima parada no caminho */
  if(paradas[i] == NULL) return arq;
  /* nao e a ultima parada, chamamos de novo com o indice incrementado e as informacoes do diretorio */
  encontrado = getArquivoRec(unidade, fat, paradas, diretorio, i, tamanho_adicional, novo);
  if(encontrado.bloco != -1){
    /* ultimo diretorio: atualizar o numero de arquivos */
    if(paradas[i+1] == NULL){
      if(tamanho < 0)
        /* tamanho negativo significa remocao de arquivo */
        arq.diretorio--;
      else if (novo == TRUE)
        arq.diretorio++;
    }
    fseek(unidade, bloco*BLOCKSIZE + j, SEEK_SET);
    fwrite(&arq, sizeof(Arquivo), 1, unidade);
  }
  return encontrado;

}

Arquivo getArquivo(FILE *unidade, int *fat, char *caminho, int tamanho_adicional, int novo){
  char **paradas = NULL;

  root.tamBytes += tamanho_adicional;
  root.instAcessado = time(NULL);
  root.instModificado = root.instAcessado;

  paradas = tokenize(caminho, "/");
  return getArquivoRec(unidade, fat, paradas, root, 0, tamanho_adicional, novo);
}

void catBloco(FILE *unidade, int bloco, int tamanho){
  int i;
  char c;
  fseek(unidade, bloco * BLOCKSIZE, SEEK_SET);
  /* presumindo que o bloco so contem o arquivo */
  for(i = 0; i < tamanho || i < BLOCKSIZE; i++){
    fread(&c, sizeof(char), 1, unidade);
    printf("%c", c);
  }
}

void catArquivo(FILE *unidade, char *caminho, int *fat){
  Arquivo arq;
  int i, tamanho;

  arq = getArquivo(unidade, fat, caminho, 0, FALSE);

  i = arq.bloco;
  tamanho = arq.tamBytes;
  if(i == -1) printf("Arquivo nao encontrado.\n");

  while(i != -1){
    i = fat[i];
    catBloco(unidade, i, tamanho);
    tamanho -= BLOCKSIZE;
  }
}

void cpArquivo(FILE *unidade, unsigned char *bitmap, char *origem, char *destino, int *fat){
  Arquivo dir, novo;
  FILE *file_origem = NULL;
  int i, bloco, tamanho, arquivos_restantes;
  char c, **paradas = NULL;

  file_origem = fopen(origem, "r");
  if(file_origem == NULL){
    printf("Nao foi possivel abrir o arquivo %s", origem);
    return;
  }
  fseek(file_origem, 0, SEEK_END);
  tamanho = ftell(file_origem);
  if(livres - tamanho/BLOCKSIZE < 0){
    printf("Espaço insuficiente.\n");
    fclose(file_origem);
    return;
  }

  dir = getArquivo(unidade, fat, destino, tamanho, TRUE);
  if(dir.bloco == -1){
    printf("Diretorio de destino inexistente.\n");
    return;
  }
  else if(dir.diretorio >= 0){
    printf("O destino deve ser um diretorio.\n");
    return;
  }

  arquivos_restantes = dir.diretorio;
  for(bloco = dir.bloco; bloco != -1; bloco = fat[bloco])
    arquivos_restantes -= BLOCKSIZE/sizeof(Arquivo);

  if(arquivos_restantes + sizeof(Arquivo) > BLOCKSIZE){
    if(livres == 0){
      dir = getArquivo(unidade, fat, destino, -tamanho, FALSE);
      printf("Espaco insuficiente.\n");
      fclose(file_origem);
      return;
    }
    livres--;

    setFAT(procuraBloco(bitmap), bloco);
    setFAT(-1, fat[bloco]);
  }

  arquivos_restantes++;
  livres -= tamanho/BLOCKSIZE;
  wasted += BLOCKSIZE - arquivos_restantes*sizeof(Arquivo);
  fseek(unidade, MAPSIZE, SEEK_SET);
  fwrite(&wasted, sizeof(wasted), 1, unidade);
  fwrite(&livres, sizeof(livres), 1, unidade);

  paradas = tokenize(origem, "/");
  for(i = 0; paradas[i+1] != NULL; i++);
  strcpy(novo.nome, paradas[i]);
  novo.tamBytes = tamanho;
  novo.instCriado = time(NULL);
  novo.instModificado = novo.instCriado;
  novo.instAcessado = novo.instCriado;
  novo.bloco = procuraBloco(bitmap);
  novo.diretorio = -1;

  fseek(unidade, bloco*BLOCKSIZE + arquivos_restantes*sizeof(Arquivo), SEEK_SET);
  fwrite(&novo, sizeof(novo), 1, unidade);

  bloco = novo.bloco;
  fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
  fseek(file_origem, 0, SEEK_SET);
  for(i = 0; i < tamanho; i++){
    if(i > BLOCKSIZE){
      i = 0;
      setFAT(procuraBloco(bitmap), bloco);
      setFAT(-1, fat[bloco]);
      bloco = fat[bloco];
      fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
    }
    fread(&c, sizeof(char), 1, file_origem);
    fwrite(&c, sizeof(char), 1, unidade);
  }

  fclose(file_origem);
}

int main(){
  char*  input, shell_prompt[MAXCHAR];
  char** argv = NULL;
  int i, fat[8*MAPSIZE], wasted; /* 25000 e a quantidade de blocos em um sistema com 100Mb */
  time_t rawtime;
  int mounted = FALSE;
  unsigned char bitmap[MAPSIZE]; /* suficiente para armazenar 100Mb */
  FILE *unidade;

  unidade = NULL;
  erro.bloco = -1;

  rawtime = time(NULL);
  printf(ctime(&rawtime));

  while(1) {

    snprintf(shell_prompt, sizeof(shell_prompt), "[ep3]: ");
    input = readline(shell_prompt);

    add_history(input);
    argv = tokenize(input, " ");
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
      if(mounted == FALSE) printf("Monte uma unidade antes de realizar este comando.\n");
      else if (argv[1] == NULL) printf("cp: insira o caminho da origem.\n");
      else if (argv[2] == NULL) printf("cp: insira o caminho do destino.\n");
      else cpArquivo(unidade, bitmap, argv[1], argv[2], fat);
    }
  	else if (strcmp(argv[0], "mkdir") == 0) {
      if(mounted == FALSE) printf("Monte uma unidade antes de realizar este comando.\n");
      else {}

  	}
  	else if (strcmp(argv[0], "rmdir") == 0) {
      if(mounted == FALSE) printf("Monte uma unidade antes de realizar este comando.\n");
      else {}

  	}
    else if (strcmp(argv[0], "cat") == 0) {
      if(mounted == FALSE) printf("Monte uma unidade antes de realizar este comando.\n");
      else if (argv[1] == NULL) printf("cat: insira o caminho do arquivo.\n");
      else catArquivo(unidade, argv[1], fat);

    }
    else if (strcmp(argv[0], "touch") == 0) {
      if(mounted == FALSE) printf("Monte uma unidade antes de realizar este comando.\n");
      else {}

  	}
  	else if (strcmp(argv[0], "rm") == 0) {
      if(mounted == FALSE) printf("Monte uma unidade antes de realizar este comando.\n");
      else {}

  	}
  	else if (strcmp(argv[0], "ls") == 0) {
      if(mounted == FALSE) printf("Monte uma unidade antes de realizar este comando.\n");
      else {}

  	}
  	else if (strcmp(argv[0], "find") == 0) {
      if(mounted == FALSE) printf("Monte uma unidade antes de realizar este comando.\n");
      else {}

  	}
  	else if (strcmp(argv[0], "df") == 0) {
      if(mounted == FALSE) printf("Monte uma unidade antes de realizar este comando.\n");
      else {}

  	}
  	else if (strcmp(argv[0], "umount") == 0) {
      if(mounted == FALSE) printf("Monte uma unidade antes de realizar este comando.\n");

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
