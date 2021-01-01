/* Identifierの種類 */
typedef enum kindT { varId, funcId, parId, constId } KindT;

/* 変数、パラメタ、関数のアドレスの型 */
typedef struct relAddr {
  int level;
  int addr;
} RelAddr;

/*	ブロックの始まり（最初の変数の番地）で呼ばれる */
void blockBegin(int firstAddr);
void blockEnd(); /*	ブロックの終わりで呼ばれる	*/
int bLevel();    /*	現ブロックのレベルを返す	*/
