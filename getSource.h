#include <stdio.h>

#define MAXNAME 31 /*	名前の最大長さ	*/

/*	キーや文字の種類（名前）	*/
typedef enum keys { Test } KeyId;

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
