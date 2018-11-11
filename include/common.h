#ifndef __C_COMPILER_COMMON__
#define __C_COMPILER_COMMON__

#include <stdbool.h>

typedef struct {
  int id;
  int total_size;
  int used_size;
  int state; // unusedは0, intやcharは1, 構造体名は2, 構造体メンバは3
  int kind;  // state=1ならbnf_id, state>1ならidentifierのtokenのid
  int byte;  // バイト数
} TYPE;

typedef struct {
  int   id;
  int   total_size;
  int   used_size;
  int   token_id;
  int   kind; // 関数引数、変数、プロトタイプ等を表すSYMBOL_TABLE_*の値
  int   type; // int, char
  int   storage; // static, extern
  int   qualify; // const, volatile
  int   block; // ローカル変数なら属する{}, 関数定義の引数なら関数の{}, 関数プロトタイプの引数なら0
  int   addr;
  int*  array;      // int a; -> 無効  int *a; -> 0   int **a; -> 00  int a[2][3]; -> 23
  int   array_size; // int a; -> 0     int *a; -> 1   int **a; -> 2   int a[2][3]; -> 2
  int   total_array_size;
  int   size;    // char:1 int:4
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
