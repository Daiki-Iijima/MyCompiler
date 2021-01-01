#include "compile.h"
#include "getSource.h"

#ifndef TBL
#define TBL
#include "table.h"
#endif

#define FIRSTADDR 2 /*	各ブロックの最初の変数アドレス	*/

static Token token; /*	次のトークンの保存用	*/

int compile() {
  printf("コンパイル開始\n");

  initSource(); /*	getSourceの初期化	*/

  token = nextToken(); /*	最初のトークン	*/

  /*	ブロックの初期化（これ以降の宣言は新しいブロックのもの）*/
  blockBegin(FIRSTADDR);

  block(0); /*	０はダミー（メインブロックの関数名は無い）*/

  return 1;
}

void block(int pIndex) {
  /*	宣言部のコンパイルを繰り返す	*/
  while (1) {
    switch (token.kind) {
      case Const:
        token = nextToken();
        continue;
      case Var:
        token = nextToken();
        continue;
      case Func:
        token = nextToken();
        continue;
      default: /*	それ以外なら宣言部は終わり	*/
        break;
    }
    break;
  }
  printf("==== コンパイル処理完了 ====\n");
}
