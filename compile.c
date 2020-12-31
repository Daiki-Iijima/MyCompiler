#include "compile.h"
#include "getSource.h"

static Token token; /*	次のトークンの保存用	*/

int compile() {

  printf("コンパイル開始\n");

  initSource(); /*	getSourceの初期化	*/

  token = nextToken(); /*	最初のトークン	*/

  return 1;
}
