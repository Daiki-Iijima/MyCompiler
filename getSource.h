#include <stdio.h>

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
  Plus,        /*	演算子と区切り記号の名前	*/
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
    char id[MAXNAME]; /*	Identfierの時、その名前	*/
    int value;        /*	Numの時、その値	*/
  } u;
} Token;

Token nextToken(); /*	次のトークンを返す	*/

int openSource(char fileName[]);
void closeSource();
void initSource(); /*	テーブルの初期設定、texファイルの初期設定	*/
