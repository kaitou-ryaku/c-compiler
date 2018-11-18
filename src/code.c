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
static void generate_code_function_definition(
  const int           function_definition
  , char*             code
  , int*              code_rest_size
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
);
static void generate_code_global_variable(
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
  for (int i=0; i<rest; i++) code[i]='\0';

  // グローバル変数をdata sectionに設置
  generate_code_global_variable(code, &rest, token, symbol);
  code = &(code[code_max_size-rest]);

  // text sectionを設置
  rest -= snprintf(code, rest, "\n.section .text\n");
  code = &(code[code_max_size-rest]);

  assert(is_pt_name("TRANSLATION_UNIT", pt[0], bnf));
  int external_declaration=pt[0].down;
  while (external_declaration >= 0) {
    const int down = pt[external_declaration].down;

    if (is_pt_name("FUNCTION_DEFINITION", pt[down], bnf)) {
      generate_code_function_definition(down, code, &rest, block, token, bnf, pt, symbol);
      code = &(code[code_max_size-rest]);
    }

    external_declaration = pt[external_declaration].right;
  }
}/*}}}*/
static void generate_code_function_definition(/*{{{*/
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

  length = snprintf(code, *code_rest_size, "FUNCTION_DEFINITION %d\n", function_definition);
  *code_rest_size -= length;
  code = &(code[length]);
}/*}}}*/
static void generate_code_global_variable(/*{{{*/
  char*           code
  , int*          code_rest_size
  , const LEX_TOKEN*  token
  , const SYMBOL* symbol
) {
  int length;

  length = snprintf(code, *code_rest_size, ".section .data\n");
  *code_rest_size -= length;
  code = &(code[length]);

  for (int line=0; line<symbol[0].used_size; line++) {
    const int kind = symbol[line].kind;
    const int storage = symbol[line].storage;

    if ((kind == SYMBOL_TABLE_VARIABLE) && (storage == SYMBOL_STORAGE_STATIC)) {
      length = snprintf(code, *code_rest_size, "%s\n", token[symbol[line].token_id].name);
      *code_rest_size -= length;
      code = &(code[length]);
    }
  }
}/*}}}*/
