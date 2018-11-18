#ifndef __C_COMPILER_COMMON__
#define __C_COMPILER_COMMON__

#include <stdbool.h>

typedef struct {
  int id;
  int total_size;
  int used_size;
  int sign;
  int length;
  int body;
  int token_id; // bnf_idがstructかtypedefの場合のみ使用。LEX_TOKEN配列のindex
  int alias_id; // bnf_idがtypedefの場合のみ使用。TYPE配列のindex
  int block;
  int byte;  // バイト数
} TYPE;

typedef struct {
  int   id;
  int   total_size;
  int   used_size;
  int   token_id;
  int   kind; // 関数引数、変数、プロトタイプ等を表すSYMBOL_TABLE_*の値
  int   type_length;// long, short, 何もなしの整数値。詳細はsymbol.h内のconst変数で定義
  int   type_sign;  // signed, unsignedの整数値。詳細はsymbol.h内のconst変数で定義
  int   type_body;  // int, char, typedef_keywordの整数値。詳細はsymbol.h内のconst変数で定義
  int   typedef_id; // typedef_keywordの整数値。詳細はsymbol.h内のconst変数で定義
  int   body_token_id; // type_bodyのtoken_id
  int   storage; // static, externの整数値。詳細はsymbol.h内のconst変数で定義
  int   qualifier; // const, volatileの整数値。詳細はsymbol.h内のconst変数で定義
  int   block; // ローカル変数なら属する{}, 関数定義の引数なら関数の{}, 関数プロトタイプの引数なら0
  int   address; // 静的変数の絶対アドレスbyte、自動変数のスタックオフセットbyte、構造体のオフセットのbyte数
  int*  array;      // int a; -> 無効  int *a; -> 0   int **a; -> 00  int a[2][3]; -> 23
  int   array_size; // int a; -> 0     int *a; -> 1   int **a; -> 2   int a[2][3]; -> 2
  int   total_array_size;
  int   byte;         // char:1 char[3]:3 int:4
  int   original_byte;// char:1 char[3]:1 int:4
  int   function_id;  // 引数の場合、親関数のidを入れる
  int   argument_id;  // 引数の場合、何番目の引数か入れる
  int   total_argument; // 関数の場合、引数の個数を入れる
} SYMBOL;

typedef struct {
  int   id;
  int   total_size;
  int   used_size;
  int   here;
  int   up;
} BLOCK;

#endif
