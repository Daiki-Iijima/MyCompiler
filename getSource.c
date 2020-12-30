#include <stdio.h>
#include <string.h>

#include "getSource.h"

static FILE *fpi;   /*	ソースファイル	*/
static FILE *fptex; /*	LaTex出力ファイル	*/

/*	ソースファイルのOpen、.texファイルの作成	*/
int openSource(char fileName[]) {
  char fileNameO[30];

  /*	ファイルが開けるか確認しながら展開	*/
  if ((fpi = fopen(fileName, "r")) == NULL) {
    printf("%sの展開に失敗しました。\n", fileName);
    return 0;
  }

  strcpy(fileNameO, fileName);
  strcat(fileNameO, ".tex");

  /*	入力ソースファイルと同じ名前の.texファイルを作る	*/
  if ((fptex = fopen(fileNameO, "w")) == NULL) {
    printf("LaTex出力ファイル%sの展開に失敗しました。\n", fileNameO);
    return 0;
  }

  return 1;
}

/*	ソースファル、.texファイルをClose	*/
void closeSource() {
  fclose(fpi);
  fclose(fptex);

  printf("ファイルを正常に閉じました。\n");
}
