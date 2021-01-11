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
static KindT idKind; /*	現トークン(Id)の種類	*/
static int spaces;   /*	そのトークンの前のスペース個数	*/
static int CR;       /*	そのトークンの前のCR(改行)の個数	*/
static int printed;  /*	トークンの文字は印字済みか	*/

static void printcToken(); /*	トークンの印字	*/

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
  // DebugCharClassT();
}

/*	テーブルの初期設定、texファイルの初期設定	*/
void initSource() {
  /*	読み込む文字の情報を初期化	*/
  lineIndex = -1;
  ch = '\n';

  printed = 1;
  initCharClassT();

  /*	LaTexコマンド書き込み	*/
  fprintf(fptex, "\\documentstyle[12pt]{article}\n");
  fprintf(fptex, "\\begin{document}\n");
  fprintf(fptex, "\\fbxsep=0pt\n");
  fprintf(fptex, "\\def\\insert#1{$\\fbox{#1}$}\n");
  fprintf(fptex, "\\def\\delete#1{$\\fboxrule=.5mm\\fbox{#1}$}\n");
  fprintf(fptex, "\\rm\n");
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
    } else {
      // errorF("ファイル読み込み終了");
    }
  }

  /*	読み込んである１行から、１文字を抽出	*/
  if ((ch = line[lineIndex++]) == '\n') {
    lineIndex = -1;
    return '\n';
  }

  return ch;
}

/*	トークン読み込みのデバッグ用	*/
void DebugToken(Token t) {
  printf("===読み取り===\n");
  switch (t.kind) {
    case 18:
      printf("トークン分類:記号\n");
      printf("内容:\(\n");
      break;
    case 19:
      printf("トークン分類:記号\n");
      printf("内容:\)\n");
      break;
    case 26:
      printf("トークン分類:記号\n");
      printf("内容:,\n");
      break;
    case 28:
      printf("トークン分類:記号\n");
      printf("内容:;\n");
      break;
    case 31:
      printf("トークン分類:文字列\n");
      printf("内容:%s\n", t.u.id);
      break;
  }
}

/*	次のトークンを読んで返す関数	*/
Token nextToken() {
  int i = 0;
  int num = 0;
  Token temp;
  KeyId cc;
  char ident[MAXNAME];

  printcToken(); /*	前のトークンを印字	*/

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
      /*	===	文字列===  */
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

          /*	デバッグ	*/
          printf("===予約語===:%s\n", ident);

          return temp;
        }
      }

      /*	ユーザーが宣言した名前の場合	*/
      temp.kind = Id;
      strcpy(temp.u.id, ident);

      break;
      /*	===数値===	*/
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
      break;

      /*	===各種記号の組み合わせパターンと記号のチェック===	*/
    case colon:
      if ((ch = nextChar() == '=')) { /*	[:=]チェック	*/
        ch = nextChar();
        temp.kind = Assign;
        break;
      } else { /*	[:]になる	*/
        temp.kind = nul;
        break;
      }
    case Lss:
      if ((ch = nextChar() == '=')) { /*	[<=]チェック	*/
        ch = nextChar();
        temp.kind = LssEq;
      } else if (ch == '>') { /*	[<>]チェック	*/
        ch = nextChar();
        temp.kind = NotEq;
      } else { /*	[<]になる	*/
        temp.kind = Lss;
        break;
      }
    case Gtr:
      if ((ch = nextChar() == '=')) { /*	[>=]チェック	*/
        ch = nextChar();
        temp.kind = GtrEq;
        break;
      } else { /*	[>]になる	*/
        temp.kind = Gtr;
        break;
      }
    default:
      temp.kind = cc;
      ch = nextChar();
      break;
  }

  cToken = temp; /*	今読み取ったトークンを保存	*/
  printed = 0;

  /*	デバッグ表示	*/
  DebugToken(temp);

  return temp;
}

/* kは予約語か?	*/
int isKeyWd(KeyId k) { return (k < end_of_KeyWd); }
/*	kは記号か？	*/
int isKeySym(KeyId k) {
  if (k < end_of_KeyWd) return 0;
  return (k < end_of_KeySym);
}

/*	t == kのチェックと、不一致の場合の対応	*/
Token checkGet(Token t, KeyId k) {
  if (t.kind == k) return nextToken();

  /*	tとkがどちらも記号 or 予約語の場合、
   *	読み取ったコードの記述ミスと判断して、想定しているワード(k)を
   *	挿入する
   */
  if ((isKeyWd(k) && isKeyWd(t.kind)) || (isKeySym(k) && isKeySym(t.kind))) {
    // errorDelete();
    // errorInsert(k);
    return nextToken();
  }

  // errorInsert(k);
  return t;
}

/*	空白や改行の印字	*/
static void printSpaces() {
  while (CR-- > 0) {
    fprintf(fptex, "\\ \\par\n");
  }

  while (spaces-- > 0) {
    fprintf(fptex, "\\ ");
  }

  CR = 0;
  spaces = 0;
}

/*	現在のトークンの印字	*/
void printcToken() {
  int i = (int)cToken.kind;

  /*	印字したか printed = 1の場合ここに入る	*/
  if (printed) {
    printed = 0;
    return;
  }

  printed = 1;

  printSpaces(); /*	トークンの前の空白や改行印字	*/

  if (i < end_of_KeyWd) /*	予約語	*/
    fprintf(fptex, "{\\bf %s}", KeyWdT[i].word);
  else if (i < end_of_KeySym) /*	演算子か区切り記号	*/
    fprintf(fptex, "$%s$", KeyWdT[i].word);
  else if (i == (int)Id) { /*	Identfier	*/
    switch (idKind) {
      case varId:
        fprintf(fptex, "%s", cToken.u.id);
        return;
      case parId:
        fprintf(fptex, "{\\sl %s}", cToken.u.id);
        return;
      case funcId:
        fprintf(fptex, "{\\it %s}", cToken.u.id);
        return;
      case constId:
        fprintf(fptex, "{\\sf %s}", cToken.u.id);
        return;
    }
  } else if (i == (int)Num) { /* Num	*/
    fprintf(fptex, "%d", cToken.u.value);
  }
}

/*	現トークン(Id)の種類をセット	*/
void setIdKind(KindT k) {
  printf("ID:%u\n", k);
  idKind = k;
}
