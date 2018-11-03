#ifndef __C_COMPILER_COMMON__
#define __C_COMPILER_COMMON__

#include <stdbool.h>

typedef struct {
  int   id;
  int   total_size;
  int   used_size;
  int   token_id;
  int   kind; // SYMBOL_TABLE_*の値
  int   type; // int, char
  int   storage; // static, extern
  int   qualify; // const, volatile
  int   pointer_qualify; // int *const a;もしくは const int *const a;でのみ値が入る
  int   block; // ローカル変数なら属する{}, 関数定義の引数なら関数の{}, 関数プロトタイプの引数なら0
  int   addr;
  int   pointer; // 0:int a; 1:int *a; 2:int **a;
  int   size;    // char:1 int:4
  int   arg_func_id; // 引数の場合、親関数のidを入れる
  int   func_arg_num; // 関数の場合、引数の個数を入れる
} SYMBOL;

typedef struct {
  int   id;
  int   total_size;
  int   used_size;
  int   here;
  int   up;
} BLOCK;

#endif
