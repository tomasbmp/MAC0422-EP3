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
int getArquivoRec(char **paradas, int i, Arquivo diretorio, int endereco,  int option){
  Arquivo arq;
  int tamanho, bloco;
  int j = 0;

  diretorio.instAcessado = time(NULL);
  if (diretorio.diretorio >=0)  diretorio.instModificado = diretorio.instAcessado;
  fseek(unidade, endereco, SEEK_SET);
  fwrite(&diretorio, sizeof(Arquivo), 1, unidade);

  switch (option){
    case FINAL:
      if(paradas[i] == NULL) return endereco;
      break;
    case PAI:
      if(paradas[i+1] == NULL) return endereco;
      break;
    default:
      break;
  }


  bloco = diretorio.bloco;

  fseek(unidade, bloco * BLOCKSIZE, SEEK_SET);

  tamanho = diretorio.diretorio;
  while (j < tamanho){
    /* se reservamos mais do que o tamanho de um bloco,
    vê na tabela FAT o endereço do próximo */
    if(j > ARQPERBLOCK) {
      j = 0;
      tamanho -= ARQPERBLOCK;
      bloco = fat[bloco];
      /* proximo bloco "nao existe",
      condicao de parada: nao encontramos o arquivo */
      fseek(unidade, bloco * BLOCKSIZE, SEEK_SET);
    }
    /* "reserva" o espaço a ser lido */

    fread(&arq, sizeof(Arquivo), 1, unidade);
    if (strcmp(paradas[i], arq.nome) == 0) return getArquivoRec(paradas, i+1, arq, bloco*BLOCKSIZE + j*sizeof(Arquivo), option);
    j ++;
    /* se reservamos mais do que o tamanho total do diretorio,
    condicao de parada: nao encontramos o arquivo */
  }
  return -1;
}



int getArquivo(char *caminho, int option){
  Arquivo root;
  char **paradas = NULL;

  fseek(unidade, ROOTSEEK, SEEK_SET);
  fread(&root, sizeof(Arquivo), 1, unidade);

  paradas = tokenize(caminho, "/");
  return getArquivoRec(paradas, 0, root, ROOTSEEK, option);
}

void catArquivo(char *caminho){
  Arquivo arq;
  int i, tamanho, bloco;
  char c;

  i = getArquivo(caminho, FINAL);

  if(i == -1) {
    printf("Arquivo nao encontrado.\n");
    return;
  }

  arq = leArquivo(i);
  tamanho = arq.tamBytes;
  bloco = arq.bloco;

  while(bloco != -1){
    fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
    bloco = fat[bloco];
    /* presumindo que o bloco so contem o arquivo */
    for(i = 0; i < tamanho && i < BLOCKSIZE; i++){
      fread(&c, sizeof(char), 1, unidade);
      printf("%c", c);
    }
    tamanho -= BLOCKSIZE;
  }
  printf("\n");
}

void cpArquivo(char *origem, char *destino){
  Arquivo dir, novo, arq;
  FILE *file_origem = NULL;
  int i, bloco, tamanho, newblock = FALSE, end, arquivos;
  char c, **paradas = NULL;

  file_origem = fopen(origem, "r");
  if(file_origem == NULL){
    printf("Nao foi possivel abrir o arquivo %s", origem);
    return;
  }
  fseek(file_origem, 0, SEEK_END);
  tamanho = ftell(file_origem);

  end = getArquivo(destino, FINAL);
  dir = leArquivo(end);

  if(end == -1){
    printf("Diretorio de destino inexistente.\n");
    return;
  }
  else if(dir.diretorio < 0){
    printf("O destino deve ser um diretorio.\n");
    return;
  }

  bloco = dir.bloco;
  i = 1;
  while(fat[bloco] != -1){
   bloco = fat[bloco];
   i++;
  }

  if(dir.diretorio == ARQPERBLOCK*i) newblock = TRUE;
  if (tamanho/BLOCKSIZE + newblock > livres){
    printf("Espaco insuficiente.\n");
    fclose(file_origem);
    return;
  }


  if(newblock != 0){
    newblock = procuraBloco();
    setBloco(newblock, 1);
    livres--;
    setFAT(newblock, fat[bloco]);
    setFAT(-1, fat[newblock]);
    bloco = newblock;
    wasted += (BLOCKSIZE - sizeof(Arquivo));
  }
  else wasted -= sizeof(Arquivo);

  paradas = tokenize(origem, "/");
  for(i = 0; paradas[i+1] != NULL; i++);
  strcpy(novo.nome, paradas[i]);
  novo.tamBytes = tamanho;
  novo.instCriado = time(NULL);
  novo.instModificado = novo.instCriado;
  novo.instAcessado = novo.instCriado;
  novo.bloco = procuraBloco(bitmap);
  novo.diretorio = -1;

  if(newblock != 0){
    fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
    fwrite(&novo, sizeof(novo), 1, unidade);
  }
  else{
    bloco = dir.bloco;
    arquivos = dir.diretorio;
    fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
    for(i = 0; i < arquivos; i++){
      if (i > ARQPERBLOCK){
        bloco = fat[bloco];
        fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
        i = 0;
        arquivos -= ARQPERBLOCK;
      }
      fread(&arq, sizeof(Arquivo), 1, unidade);
      if(strlen(arq.nome) == 0){ /* este espaco esta vazio */
        break;
      }
    }
    fseek(unidade, bloco*BLOCKSIZE + i*sizeof(Arquivo), SEEK_SET);
    fwrite(&novo, sizeof(novo), 1, unidade);
  }

  dir.tamBytes += sizeof(Arquivo);
  dir.diretorio++;
  fseek(unidade, end, SEEK_SET);
  fwrite(&dir, sizeof(dir), 1, unidade);

  bloco = novo.bloco;
  setBloco(novo.bloco, 1);
  livres--;

  fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
  fseek(file_origem, 0, SEEK_SET);
  for(i = 0; i < tamanho; i++){
    if(i >= BLOCKSIZE){
      tamanho -= BLOCKSIZE;
      i = 0;
      newblock = procuraBloco(bitmap);
      setFAT(newblock, bloco);
      setFAT(-1, newblock);
      setBloco(newblock, 1);
      livres--;
      bloco = newblock;
      fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
    }
    fread(&c, sizeof(char), 1, file_origem);
    fwrite(&c, sizeof(char), 1, unidade);
  }

  addF();
  fseek(unidade, WASTESEEK, SEEK_SET);
  fwrite(&wasted, sizeof(wasted), 1, unidade);
  fwrite(&livres, sizeof(livres), 1, unidade);
  fclose(file_origem);
}

void touchArquivo(char *caminho){
  Arquivo arq, dir;
  char **paradas = NULL;
  char str[MAXCHAR];
  int end, arquivos, bloco, i, j, endvago = -1, newblock;

  end = getArquivo(caminho, PAI);
  dir = leArquivo(end);
  paradas = tokenize(caminho, "/");
  for(i = 0; paradas[i+1] != NULL; i++);
  strcpy(str, paradas[i]);
  
  bloco = dir.bloco;
  arquivos = dir.diretorio;
  j = 0;
  i = 0;
  fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);

  while (i < arquivos){
    if (j > ARQPERBLOCK){
      j = 0;
      bloco = fat[bloco];
      fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
    }
    fread(&arq, sizeof(Arquivo), 1, unidade);
    if(strlen(arq.nome) != 0){
      if (strcmp(arq.nome, str) == 0){  
        arq.instAcessado = dir.instAcessado;
        fseek(unidade, bloco*BLOCKSIZE + j*sizeof(Arquivo), SEEK_SET);
        fwrite(&arq, sizeof(Arquivo), 1, unidade);
        return;
      }
      i++;
    }
    else if (endvago == -1) endvago = bloco*BLOCKSIZE + j*sizeof(Arquivo);
    j++;
    printf("Cheguei no final do while.\n");
  }

  /* se chegou aqui entao o arquivo nao existe e sera criado */
  strcpy(arq.nome, str);
  arq.tamBytes = 0;
  arq.instCriado = dir.instAcessado;
  arq.instModificado = arq.instCriado;
  arq.instAcessado = arq.instCriado;
  arq.diretorio = -1;

  if((j == ARQPERBLOCK)&&(endvago != -1)){ /* teremos que alocar um bloco novo para o diretorio */
    if (livres < 2){
      printf("Espaco insuficiente.\n");
      return;
    }
    newblock = procuraBloco();
    setFAT(newblock, bloco);
    setFAT(-1, newblock);
    setBloco(newblock, 1);
    livres--;
    bloco = newblock;
    fseek(unidade, newblock*BLOCKSIZE, SEEK_SET);
    wasted += (BLOCKSIZE - sizeof(Arquivo));
  }
  else {
    if (livres < 1){
      printf("Espaco insuficiente.\n");
      return;
    }
    if (endvago == -1)  fseek(unidade, bloco*BLOCKSIZE + j*sizeof(Arquivo), SEEK_SET);
    else fseek(unidade, endvago, SEEK_SET);
    wasted -= sizeof(Arquivo);
  }
  dir.diretorio++;
  dir.tamBytes += sizeof(Arquivo);

  newblock = procuraBloco();
  

  arq.bloco = newblock;
  fwrite(&arq, sizeof(Arquivo), 1, unidade);
  fseek(unidade, end, SEEK_SET);
  fwrite(&dir, sizeof(Arquivo), 1, unidade);

  setBloco(newblock, 1);
  livres--;
  addF();

  
  fseek(unidade, WASTESEEK, SEEK_SET);
  fwrite(&wasted, sizeof(wasted), 1, unidade);
  fwrite(&livres, sizeof(livres), 1, unidade);
}

void lsArquivo(char *caminho){
  int tamanho, endereco, bloco, i;
  Arquivo dir, file;

  endereco = getArquivo (caminho, FINAL);
  dir = leArquivo(endereco);
  bloco = dir.bloco;

  tamanho = dir.diretorio;
  fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
  if(dir.diretorio == 0) (printf("Este diretorio esta vazio.\n"));
  else printf ("\nd?|    nome    | tamanho em bytes | ultima modificacao\n");
  for(i = 0; i < tamanho; i++){
    if(i >= BLOCKSIZE/sizeof(Arquivo) ){
      tamanho -= i;
      i = 0;
      bloco = fat[bloco];
      fseek(unidade, bloco*BLOCKSIZE, SEEK_SET);
    }

    fread(&file, sizeof(Arquivo), 1, unidade);
    /* printf("tamanho de %s = %d\n", file.nome, strlen(file.nome)); */
    if(file.diretorio >=0)  printf("* |");
    else printf("  |");
    /* for(j = 0; j < (12 - strlen(file.nome)); j++) printf(" "); */
    printf("%12s|", file.nome);
    printf("%18d|", file.tamBytes);
    printf(" ");
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
      else catArquivo(argv[1]);

    }
    else if (strcmp(argv[0], "touch") == 0) {
      if(mounted == FALSE) printf("Monte uma unidade antes de realizar este comando.\n");
      else if (argv[1] == NULL) printf("cat: insira o caminho do arquivo.\n");
      else touchArquivo(argv[1]);

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
