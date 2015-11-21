  /*

MAC0422 - Sistemas Operacionais
    EP3 - 22/11/2015

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


int fat[8*MAPSIZE], wasted, livres = 8*MAPSIZE, qtyF = 0, qtyD = 0; /* 8*MAPSIZE e a quantidade de blocos em um sistema com 100Mb */
unsigned char bitmap[MAPSIZE]; /* suficiente para armazenar 100Mb */
FILE *unidade;

Arquivo leArquivo(int i){
  Arquivo arq;

  fseek(unidade, i, SEEK_SET);
  fread(&arq, sizeof(Arquivo), 1, unidade);
  return arq;
}

/* funcao recursiva que retorna endereco absoluto do arquivo dado pelo caminho */
int getArquivoRec(char **paradas, int i, Arquivo diretorio, int enderecoPai,  int option){
  Arquivo arq;
  int tamanho, bloco, apartamento;
  int j = 0;

  diretorio.instAcessado = time(NULL);
  diretorio.instModificado = arq.instAcessado;
  fseek(unidade, enderecoPai, SEEK_SET);
  fwrite(&diretorio, sizeof(Arquivo), 1, unidade);

  tamanho = diretorio.diretorio;
  bloco = diretorio.bloco;

  fseek(unidade, bloco * BLOCKSIZE, SEEK_SET);

  if(tamanho > 0){
    do{
      /* se reservamos mais do que o tamanho de um bloco,
      vê na tabela FAT o endereço do próximo */
      if(j > BLOCKSIZE/sizeof(Arquivo)) {
        j = 0;
        bloco = fat[bloco];
        /* proximo bloco "nao existe",
        condicao de parada: nao encontramos o arquivo */
        if(bloco == -1) break;
        fseek(unidade, bloco * BLOCKSIZE, SEEK_SET);
        tamanho -= BLOCKSIZE;
      }
      /* "reserva" o espaço a ser lido */
      j ++;
      /* se reservamos mais do que o tamanho total do diretorio,
      condicao de parada: nao encontramos o arquivo */
      if(j > tamanho) break;

      fread(&arq, sizeof(Arquivo), 1, unidade);
    }while(strcmp(arq.nome, paradas[i]) != 0);
    /* encontramos o arquivo no diretorio */
  }

  switch (option){
    case LS :
      apartamento = enderecoPai;
      break;
    default:
      apartamento = bloco*BLOCKSIZE + j - sizeof(Arquivo);
      break;
  }
  

  i++;
  /* condicao de parada: e a ultima parada no caminho */
  if((paradas[i] == NULL)||(paradas[i-1] == NULL)){
    if(option == ADD){
      diretorio.tamBytes += sizeof(Arquivo);
      diretorio.diretorio++;
      fseek(unidade, enderecoPai, SEEK_SET);
      fwrite(&diretorio, sizeof(Arquivo), 1, unidade);
    }else if (option == REMOVE){
      diretorio.tamBytes -= sizeof(Arquivo);
      diretorio.diretorio--;
      fseek(unidade, enderecoPai, SEEK_SET);
      fwrite(&diretorio, sizeof(Arquivo), 1, unidade);
    }else if (option == TOUCH){
      arq.instAcessado = time(NULL);
      fseek(unidade, -sizeof(Arquivo), SEEK_CUR);
      fwrite(&arq, sizeof(Arquivo), 1, unidade);
    }

    return apartamento;
  }

  /* nao e a ultima parada, chamamos de novo com o indice incrementado e as informacoes do diretorio */
  return getArquivoRec(paradas, i, diretorio, apartamento, option);
}

int getArquivo(char *caminho, int option){
  Arquivo root;
  char **paradas = NULL;

  fseek(unidade, ROOTSEEK, SEEK_SET);
  fread(&root, sizeof(Arquivo), 1, unidade);

  paradas = tokenize(caminho, "/");
  return getArquivoRec(paradas, 0, root, ROOTSEEK, option);
}

void catArquivo(FILE *unidade, char *caminho, int *fat){
  Arquivo arq;
  int i, tamanho;
  char c;

  i = getArquivo(caminho, VISIT);

  if(i == -1) printf("Arquivo nao encontrado.\n");

  arq = leArquivo(i);
  tamanho = arq.tamBytes;

  while(i != -1){
    i = fat[i];
    fseek(unidade, i*BLOCKSIZE, SEEK_SET);
    /* presumindo que o bloco so contem o arquivo */
    for(i = 0; i < tamanho || i < BLOCKSIZE; i++){
      fread(&c, sizeof(char), 1, unidade);
      printf("%c", c);
    }
    tamanho -= BLOCKSIZE;
  }
}

void cpArquivo(char *origem, char *destino){
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

  i = getArquivo(destino, ADD);
  dir = leArquivo(i);

  if(i == -1){
    printf("Diretorio de destino inexistente.\n");
    return;
  }
  else if(dir.diretorio < 0){
    printf("O destino deve ser um diretorio.\n");
    return;
  }

  arquivos_restantes = dir.diretorio;
  for(bloco = dir.bloco; fat[bloco] != -1; bloco = fat[bloco])
    arquivos_restantes -= BLOCKSIZE/sizeof(Arquivo);

  if(arquivos_restantes*sizeof(Arquivo) + sizeof(Arquivo) > BLOCKSIZE){
    if(livres == 0){
      i = getArquivo(destino, REMOVE);
      dir = leArquivo(i);
      printf("Espaco insuficiente.\n");
      fclose(file_origem);
      return;
    }
    livres--;

    arquivos_restantes = 0;
    wasted += BLOCKSIZE - sizeof(Arquivo);
    setFAT(procuraBloco(bitmap), bloco);
    setFAT(-1, fat[bloco]);
  }
  else wasted -= sizeof(Arquivo);

  livres -= tamanho/BLOCKSIZE;
  fseek(unidade, WASTESEEK, SEEK_SET);
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
      tamanho -= BLOCKSIZE;
      i = 0;
      setFAT(procuraBloco(bitmap), bloco);
      setFAT(-1, fat[bloco]);
      bloco = fat[bloco];
      fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
    }
    fread(&c, sizeof(char), 1, file_origem);
    fwrite(&c, sizeof(char), 1, unidade);
  }

  addF();
  fclose(file_origem);
}

void touchArquivo(char *caminho){
  Arquivo arq, dir;
  char **paradas = NULL;
  char str[MAXCHAR];
  int i, arquivos_restantes, bloco;

  i = getArquivo(caminho, TOUCH);
  arq = leArquivo(i);
  paradas = tokenize(caminho, "/");
  strcpy(str, "/");
  for(i = 0; paradas[i+2] != NULL; i++){
    strncat(str, paradas[i], 13);
    strncat(str, "/", 13);
  }

  if(arq.bloco != -1){
    printf("Arquivo %s acessado.\n", caminho);
    return;
  }

  strcpy(arq.nome, paradas[i+1]);
  arq.tamBytes = 0;
  arq.instCriado = time(NULL);
  arq.instModificado = arq.instCriado;
  arq.instAcessado = arq.instCriado;
  arq.diretorio = -1;

  i = getArquivo(str, ADD);

  arquivos_restantes = dir.diretorio;
  for(bloco = dir.bloco; fat[bloco] != -1; bloco = fat[bloco])
    arquivos_restantes -= BLOCKSIZE/sizeof(Arquivo);

  if(arquivos_restantes*sizeof(Arquivo) + sizeof(Arquivo) > BLOCKSIZE){
    if(livres == 0){
      i = getArquivo(str, REMOVE);
      dir = leArquivo(i);
      printf("Espaco insuficiente.\n");
      return;
    }
    livres--;

    arquivos_restantes = 0;
    wasted += BLOCKSIZE - sizeof(Arquivo);
  }
  else wasted -= sizeof(Arquivo);

  addF();
  fseek(unidade, WASTESEEK, SEEK_SET);
  fwrite(&wasted, sizeof(wasted), 1, unidade);
  fwrite(&livres, sizeof(livres), 1, unidade);
}

void lsArquivo(char *caminho){
  int tamanho, endereco, bloco, i;
  Arquivo dir, file;

  endereco = getArquivo (caminho, LS);
  dir = leArquivo(endereco);
  bloco = dir.bloco;

  tamanho = dir.diretorio;
  fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
  if(dir.diretorio == 0) (printf("Este diretorio esta vazio.\n"));
  else printf ("d?|    nome    | tamanho em bytes | ultima modificacao");
  for(i = 0; i < tamanho; i++){
    if(i >= BLOCKSIZE/sizeof(Arquivo) ){
      tamanho -= i;
      i = 0;
      bloco = fat[bloco];
      fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
    }

    fread(&file, sizeof(Arquivo), 1, unidade);
    if(file.diretorio >=0)  printf("* |");
    printf("%12s|", file.nome);
    printf("%18d|", file.tamBytes);
    printf(ctime(&file.instModificado));
  }
}

int main(){
  char*  input, shell_prompt[MAXCHAR];
  char** argv = NULL;
  int i;
  time_t rawtime;
  int mounted = FALSE;
  Arquivo novo;

  unidade = NULL;

  printf("sizeof(Arquivo) = %lu\n", sizeof(Arquivo));

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
            for (i = 0; i < MAPSIZE*8; i++) fread(&fat[i], sizeof(int), 1, unidade);


          }
          mounted = TRUE;
      }
      else printf("Desmonte o sistema de arquivos atual para poder montar outro.\n");

  	}
    else if (strcmp(argv[0], "cp") == 0){
      if(mounted == FALSE) printf("Monte uma unidade antes de realizar este comando.\n");
      else if (argv[1] == NULL) printf("cp: insira o caminho da origem.\n");
      else if (argv[2] == NULL) printf("cp: insira o caminho do destino.\n");
      else cpArquivo(argv[1], argv[2]);
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
      else if (argv[1] == NULL) printf("cat: insira o caminho do diretorio.\n");
      else lsArquivo(argv[1]);

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

  if(unidade != NULL)
    fclose(unidade);

  return 0;
}
