#include "../min-bnf-parser/include/min-bnf-parser.h"
#include "../include/ast.h"
#include "../include/common.h"
#include "../include/pt_common.h"
#include "../include/code.h"
#include "../include/symbol.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

// 関数プロトタイプ/*{{{*/
static char* generate_code_function_definition(
  const int           function_definition
  , char*             code
  , int*              code_rest_size
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
);
static char* generate_code_global_variable(
  char*           code
  , int*          code_rest_size
  , const LEX_TOKEN*  token
  , const SYMBOL* symbol
);
/*}}}*/
extern void generate_code(/*{{{*/
  char*               code
  , const int         code_max_size
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
) {

  int rest = code_max_size;
  int length = 0;
  for (int i=0; i<rest; i++) code[i]='\0';

  // グローバル変数をdata sectionに設置
  length = snprintf(code, rest, ".section .data\n\n");
  rest -= length;
  code = &(code[length]);

  code = generate_code_global_variable(code, &rest, token, symbol);

  // text sectionを設置
  length = snprintf(code, rest, "\n.section .text\n\n");
  rest -= length;
  code = &(code[length]);

  assert(is_pt_name("TRANSLATION_UNIT", pt[0], bnf));
  int external_declaration=pt[0].down;
  while (external_declaration >= 0) {
    const int down = pt[external_declaration].down;

    if (is_pt_name("FUNCTION_DEFINITION", pt[down], bnf)) {
      code = generate_code_function_definition(down, code, &rest, block, token, bnf, pt, symbol);
    }

    external_declaration = pt[external_declaration].right;
  }
}/*}}}*/
static char* generate_code_function_definition(/*{{{*/
  const int           function_definition
  , char*             code
  , int*              code_rest_size
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
) {
  assert(is_pt_name("FUNCTION_DEFINITION", pt[function_definition], bnf));
  int length;

  // 関数名を取得
  const int identifier = pt[function_definition].down;
  assert(is_pt_name("identifier", pt[identifier], bnf));

  length = snprintf(code, *code_rest_size, "_%s:\n", token[pt[identifier].token_begin_index].name);
  *code_rest_size -= length;
  code = &(code[length]);

  // スタックフレームの処理
  length = snprintf(code, *code_rest_size, "push ebp\nmov  ebp, esp\n");
  *code_rest_size -= length;
  code = &(code[length]);

  // ローカル変数領域の確保
  const int function_id = search_symbol_table_by_declare_token(pt[identifier].token_begin_index, symbol);
  assert(function_id >= 0);
  int all_var_byte = 0;
  for (int line=0; line<symbol[0].used_size; line++) {
    const int kind = symbol[line].kind;
    const int storage = symbol[line].storage;

    if ((kind == SYMBOL_TABLE_VARIABLE) && (storage == SYMBOL_STORAGE_AUTO) && (symbol[line].function_id == function_id)) {
      if (all_var_byte < symbol[line].address + symbol[line].byte) {
        all_var_byte = symbol[line].address + symbol[line].byte;
      }
    }
  }
  length = snprintf(code, *code_rest_size, "sub  esp %d\n", all_var_byte);
  *code_rest_size -= length;
  code = &(code[length]);

  //TODO

  length = snprintf(code, *code_rest_size, "leave\nret\n\n");
  *code_rest_size -= length;
  code = &(code[length]);

  return code;
}/*}}}*/
static char* generate_code_global_variable(/*{{{*/
  char*           code
  , int*          code_rest_size
  , const LEX_TOKEN*  token
  , const SYMBOL* symbol
) {
  int length;

  for (int line=0; line<symbol[0].used_size; line++) {
    const int kind = symbol[line].kind;
    const int storage = symbol[line].storage;

    if ((kind == SYMBOL_TABLE_VARIABLE) && (storage == SYMBOL_STORAGE_STATIC)) {
      length = snprintf(code, *code_rest_size, "_%s times %d db 0\n", token[symbol[line].token_id].name, symbol[line].byte);
      *code_rest_size -= length;
      code = &(code[length]);
    }
  }

  return code;
}/*}}}*/
