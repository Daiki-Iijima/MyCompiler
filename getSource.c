#include <stdio.h>
#include <string.h>

#include "getSource.h"

#define MAXLINE 120 /*	1行の最大文字数	*/
#define MAXNUM 30   /*	定数の最大桁数	*/
#define TAB 5;

#define STR(var) \
#var /*	引数にした変数を変数名を示す文字リテラルとして返すマクロ */

static FILE *fpi;          /*	ソースファイル	*/
static FILE *fptex;        /*	LaTex出力ファイル	*/
static char line[MAXLINE]; /*	１行分の入力バッファー	*/

static int lineIndex; /*	次に読む文字の位置	*/
static char ch;       /*	最後に読んだ文字	*/

static Token cToken; /*	最後に読んだトークン	*/
static int spaces;   /*	そのトークンの前のスペース個数	*/
static int CR;       /*	そのトークンの前のCR(改行)の個数	*/
static int printed;  /*	トークンの文字は印字済みか	*/

/*	「予約語or記号』と名前(KeyId)のペア	*/
struct keyWd {
  char *word;
  KeyId keyId;
};

/*	「予約語or記号』と名前(KeyId)のペアの表	*/
/*	実際のコードに使用する複合的な記号を定義している	*/
static struct keyWd KeyWdT[] = {
    {"begin", Begin},
    {"end", End},
    {"if", If},
    {"then", Then},
    {"while", While},
    {"do", Do},
    {"return", Ret},
    {"function", Func},
    {"var", Var},
    {"const", Const},
    {"odd", Odd},
    {"write", Write},
    {"writeln", WriteLn},
    {"$dummy1", end_of_KeyWd},
    {"+", Plus},
    {"-", Minus},
    {"*", Mult},
    {"/", Div},
    {"(", Lparen},
    {")", Rparen},
    {"=", Equal},
    {"<", Lss},
    {">", Gtr},
    {"<>", NotEq},
    {"<=", LssEq},
    {">=", GtrEq},
    {",", Comma},
    {".", Period},
    {";", Semicolon},
    {":=", Assign},
    {"$dummy2", end_of_KeySym},
};

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

static KeyId charClassT[256]; /*	文字の種類を示す表	*/

/*	文字表のデバッグ表示	*/
static void DebugCharClassT() {
  int i;
  for (i = 0; i < 256; i++) {
    if (charClassT[i] == 35)
      printf("%d番目:%s\n", i, "letter");
    else if (charClassT[i] == 36)
      printf("%d番目:%s\n", i, "digit");
    else if (charClassT[i] == 38)
      printf("%d番目:%s\n", i, "other");
    else
      printf("%d番目:%u\n", i, charClassT[i]);
  }
}

/*	文字の種類を表す表を作る	*/
static void initCharClassT() {
  int i;

  for (i = 0; i < 256; i++) {
    charClassT[i] = others;
  }

  /*	ASCllコードの文字コードで登録	*/
  for (i = '0'; i <= '9'; i++) {
    charClassT[i] = digit;
  }
  for (i = 'A'; i <= 'Z'; i++) {
    charClassT[i] = letter;
  }
  for (i = 'a'; i <= 'z'; i++) {
    charClassT[i] = letter;
  }

  /*	演算子の登録	*/
  /*	四則演算子	*/
  charClassT['+'] = Plus;
  charClassT['-'] = Minus;
  charClassT['*'] = Mult;
  charClassT['/'] = Div;
  /*	かっこ */
  charClassT['('] = Lparen;
  charClassT[')'] = Rparen;
  /*	代入、比較演算子	*/
  charClassT['='] = Equal;
  charClassT['<'] = Lss;
  charClassT['>'] = Gtr;
  /*	その他の記号	*/
  charClassT[','] = Comma;
  charClassT['.'] = Period;
  charClassT[';'] = Semicolon;
  charClassT[':'] = colon;

  /*	デバッグ	*/
  DebugCharClassT();
}

/*	テーブルの初期設定、texファイルの初期設定	*/
void initSource() {
  /*	読み込む文字の情報を初期化	*/
  lineIndex = -1;
  ch = '\n';

  printed = 1;
  initCharClassT();

  /*	LaTexコマンド書き込み	*/
}

/*	次の１文字を読み取って返す	*/
char nextChar() {
  char ch;

  /*	1行を読み込む */
  if (lineIndex == -1) {
    if (fgets(line, MAXLINE, fpi) != NULL) {
      printf("読み込み文字列:");
      puts(line);
      lineIndex = 0;
    }
  }

  /*	読み込んである１行から、１文字を抽出	*/
  if ((ch = line[lineIndex++]) == '\n') {
    lineIndex = -1;
    return '\n';
  }

  return ch;
}

/*	次のトークンを読んで返す関数	*/
Token nextToken() {
  int i = 0;
  int num = 0;
  Token temp;
  KeyId cc;
  char ident[MAXNAME];

  /*	空白、改行カウントの初期化	*/
  spaces = 0;
  CR = 0;

  /*	次のトークンまでの空白や改行を、カウントしながら読み飛ばす	*/
  while (1) {
    if (ch == ' ') {
      spaces++;
    } else if (ch == '\t') {
      spaces += TAB;
    } else if (ch == '\n') {
      spaces = -1;
      CR++;
    } else {
      break;
    }

    /*	次の1文字読み取る	*/
    ch = nextChar();
  }

  /*	文字の種類によって処理を分岐	*/
  /*	読み取った文字を作成した文字記号表を元にチェック	*/
  switch (cc = charClassT[ch]) {
    case letter:

      /*	文字か数字だったら繰り返し読み込む	*/
      do {
        /*	1文字目は空白飛ばしの時にすでに読み込んでいるのでそのまま追加*/
        if (i < MAXNAME) ident[i] = ch;
        i++;
        ch = nextChar();
      } while (charClassT[ch] == letter || charClassT[ch] == digit);

      if (i >= MAXNAME) {
        //	errorMessage("なげえよ");
        i = MAXNAME - 1;
      }

      ident[i] = '\0';

      /*	予約語として登録されているかチェック	*/
      for (i = 0; i < end_of_KeyWd; i++) {
        if (strcmp(ident, KeyWdT[i].word) == 0) {
          temp.kind = KeyWdT[i].keyId;
          cToken = temp;
          printed = 0;
          return temp;
        }
      }

      /*	ユーザーが宣言した名前の場合	*/
      temp.kind = Id;
      strcpy(temp.u.id, ident);

      break;
    case digit:
      num = 0;
      do {
        num = 10 * num + (ch - '0');
        i++;
        ch = nextChar();
      } while (charClassT[ch] == digit);

      if (i > MAXNUM) {
        //	errorMessage("なげえよ");
      }
      temp.kind = Num;
      temp.u.value = num;
  }

  printf("トークンまでの改行数:%d\n", CR);
  printf(
      "トークン番号:%u\n"
      "トークン名(文字列の場合):%s\n"
      "トークン値(数字の場合)%d\n",
      temp.kind, temp.u.id, temp.u.value
			);

  return temp;
}
