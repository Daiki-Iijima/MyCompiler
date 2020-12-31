#include <stdio.h>

#include "compile.h"
#include "getSource.h"

int main() {
  char fileName[30];

  printf("ファイル名を入力してください\n");
  scanf("%s", fileName);

  if (!openSource(fileName)) return 1;

  if (compile()) {
    printf("コンパイル成功\n");
  }

  closeSource();

  return 0;
}
