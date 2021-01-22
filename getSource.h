#include <stdio.h>
#ifndef TBL
#define TBL
#include "table.h"
#endif

#define MAXNAME 31 /*	名前の最大長さ	*/

/*	キーや文字の種類（名前）	*/
typedef enum keys {
  Begin, /*	予約語の名前	*/
  End,
  If,
  Then,
  While,
  Do,
  Ret,
  Func,
  Var,
  Const,
  Odd,
  Write,
  WriteLn,
  end_of_KeyWd, /*	予約語の名前の最後	*/
  Plus,         /*	演算子と区切り記号の名前	*/
  Minus,
  Mult,
  Div,
  Lparen,
  Rparen,
  Equal,
  Lss,
  Gtr,
  NotEq,
  LssEq,
  GtrEq,
  Comma,
  Period,
  Semicolon,
  Assign,
  end_of_KeySym, /*	演算子と区切り記号の名前の最後	*/
  Id,            /*	トークンの種類	*/
  Num,
  nul,
  end_of_Token, /*	トークンの種類の最後	*/
  letter,       /*	上記以外の文字の種類	*/
  digit,
  colon,
  others
} KeyId;

/*	トークンの型	*/
typedef struct token {
  KeyId kind; /*	トークンの種類かキーの名前	*/
  union {
    char id[MAXNAME]; /*	kindがIdの時のその名前(変数名)	*/
    int value;        /*	Numの時のその値	*/
  } u;
} Token;

Token nextToken();                /*	次のトークンを返す	*/
Token checkGet(Token t, KeyId k); /* t.kind == k のチェック*/
/*	t.kind == k : 次のトークンを読んで返す */
/*	t.kind != k : エラーメッセージ出す*/
/*	tとkが共に記号or予約語なら、tを捨て次のトークンを読んで返す*/
/*	それ以外なら、kを挿入したことにしてtを返す(kの記述忘れと判断)*/

int openSource(char fileName[]);
void closeSource();
void initSource(); /*	テーブルの初期設定、texファイルの初期設定	*/

/*	エラー出力関係	*/
/*	.texファイルに出力	*/
void errorType(char *m);   /*	型エラーを出力	*/
void errorInsert(KeyId k); /*	記述ミスらしい部分を出力	*/
void errorMissingId();     /*	変数名が見つからないことを出力	*/
void errorMissingOp();     /*	演算子が見つからないことを出力	*/
void errorDelete(); /*今読んだトークンを読み捨てたことを出力	*/
void errorMessage(char *m); /*	エラーメッセージを出力	*/
/*	エラーメッセージを出力して、コンパイル終了	*/
void errorF(char *m);

/*	エラーの個数を返す	*/
int errorN();

/*	現トークン(Id)の種類をセット(.texファイルの出力のため) */
void setIdKind(KindT k);
