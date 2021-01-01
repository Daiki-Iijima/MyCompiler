#ifndef TBL
#define TBL
#include "table.h"
#endif

#include "getSource.h"

#define MAXLEVEL 5 /*	ブロックの最大の深さ	*/

static int tIndex = 0; /*	名前表のインデックス	*/
static int level = -1; /*	現在のブロックレベル	*/
/*	index[i]にはブロックレベルiの最後のインデックスが入る*/
static int index[MAXLEVEL];
/*	index[i]にはブロックレベルiの最後の変数の番地が入る*/
static int addr[MAXLEVEL];
static int localAddr; /*	現在のブロックの最後の変数番地	*/

void DebugLog() {
  printf("名前表インデックス番号:%d\n", tIndex);
  printf("ブロックレベル:%d\n", level);
  printf("localAddr:%d\n", localAddr);
}

/*	ブロックの始まり(最初の変数の番地)で呼ばれる	*/
void blockBegin(int firstAddr) {
  /*	一番最初にこのメソッドを読んだ時に絶対呼ばれる	*/
  if (level == -1) {
    localAddr = firstAddr;
    tIndex = 0;
    level++;
		DebugLog();
    return;
  }

  /*	ネストの深さが限界値の場合エラー	*/
  if (level == MAXLEVEL - 1) {
  }

  index[level] = tIndex;   /*	今までのブロックの情報を格納	*/
  addr[level] = localAddr; /*	今までの変数の情報を格納	*/
  localAddr = firstAddr; /*	新しいブロックの最初の変数の番地	*/
  level++;               /*	新しいブロックのレベル	*/

  DebugLog();

  return;
}
