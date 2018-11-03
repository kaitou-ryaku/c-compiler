#ifndef __C_COMPILER_COMMON__
#define __C_COMPILER_COMMON__

#include <stdbool.h>

typedef struct {
  int   id;
  int   total_size;
  int   used_size;
  int   token_id;
  int   kind; // -1:未使用 0:変数 1:関数 2:構造体 3:列挙体
  int   type; // 0:char 1:int
  int   block;
  int   addr;
  int   pointer; // 0:int a; 1:int *a; 2:int **a;
  int   size;    // char:1 int:4
  bool  const_var;
  bool  const_addr;
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
