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
#include <stdlib.h>

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
static char* generate_code_compound_statement(
  const int           compound_statement
  , char*             code
  , int*              code_rest_size
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
);
static char* generate_code_expression(
  const int           expression
  , char*             code
  , int*              code_rest_size
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
);
static char* generate_code_constant(
  const int           constant
  , char*             code
  , int*              code_rest_size
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
);
static char* generate_code_identifier(
  const int           identifier
  , char*             code
  , int*              code_rest_size
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
);
static char* write_identifier(
  const int           identifier
  , char*             code
  , int*              code_rest_size
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
);
static char* generate_code_binary_operator(
  const int           binary_operator
  , char*             code
  , int*              code_rest_size
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
);
static char* generate_code_assign(
  const int           assign
  , char*             code
  , int*              code_rest_size
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
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

  const int compound_statement = pt[identifier].right;
  assert(compound_statement >= 0);
  assert(is_pt_name("COMPOUND_STATEMENT", pt[compound_statement], bnf));
  assert(pt[compound_statement].right < 0);
  code = generate_code_compound_statement(compound_statement, code, code_rest_size, block, token, bnf, pt, symbol);

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
static char* generate_code_compound_statement(/*{{{*/
  const int           compound_statement
  , char*             code
  , int*              code_rest_size
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
) {
  assert(is_pt_name("COMPOUND_STATEMENT", pt[compound_statement], bnf));

  int down = pt[compound_statement].down;
  while (down >= 0) {
    // TODO とりあえず式文のみ解析
    code = generate_code_expression(down, code, code_rest_size, block, token, bnf, pt, symbol);
    down = pt[down].right;
  }

  return code;
}/*}}}*/
static char* generate_code_expression(/*{{{*/
  const int           expression
  , char*             code
  , int*              code_rest_size
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
) {
  // 色々足りなすぎる
  if      (is_pt_name("integer_constant", pt[expression], bnf)) code = generate_code_constant(expression, code, code_rest_size, token, bnf, pt);
  else if (is_pt_name("identifier"      , pt[expression], bnf)) code = generate_code_identifier(expression, code, code_rest_size, block, token, bnf, pt, symbol);
  else if (is_pt_name("plus"            , pt[expression], bnf)) code = generate_code_binary_operator(expression, code, code_rest_size, block, token, bnf, pt, symbol);
  else if (is_pt_name("equal"           , pt[expression], bnf)) code = generate_code_assign(expression, code, code_rest_size, block, token, bnf, pt, symbol);

  return code;
}/*}}}*/
static char* generate_code_constant(/*{{{*/
  const int           constant
  , char*             code
  , int*              code_rest_size
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
) {
  // TODO 整数、浮動小数点、文字定数、文字列定数
  assert(is_pt_name("integer_constant"  , pt[constant], bnf));
  int length;

  const int number = atoi(token[pt[constant].token_begin_index].name);
  length = snprintf(code, *code_rest_size, "mov eax, %d\npush eax\n", number);
  *code_rest_size -= length;
  code = &(code[length]);

  return code;
}/*}}}*/
static char* generate_code_identifier(/*{{{*/
  const int           identifier
  , char*             code
  , int*              code_rest_size
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
) {
  assert(is_pt_name("identifier"  , pt[identifier], bnf));
  int length;

  length = snprintf(code, *code_rest_size, "mov eax, ");
  *code_rest_size -= length;
  code = &(code[length]);

  code = write_identifier(identifier, code, code_rest_size, block, token, bnf, pt, symbol);

  length = snprintf(code, *code_rest_size, "\npush eax\n");
  *code_rest_size -= length;
  code = &(code[length]);

  return code;
}/*}}}*/
static char* write_identifier(
  const int           identifier
  , char*             code
  , int*              code_rest_size
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
) {
  // TODO ポインタ処理、配列処理、、構造体処理、関数呼び出し等
  assert(is_pt_name("identifier"  , pt[identifier], bnf));
  int length;

  const int token_index  = pt[identifier].token_begin_index;
  const int symbol_index = search_symbol_table(token_index, block, token, bnf, pt, symbol);
  const int kind         = symbol[symbol_index].kind;
  const int storage      = symbol[symbol_index].storage;

  // 変数
  if (kind == SYMBOL_TABLE_VARIABLE) {

    // ローカル変数
    if (storage == SYMBOL_STORAGE_AUTO) {
      length = snprintf(code, *code_rest_size, "[ebp-%d]", symbol[symbol_index].address);
      *code_rest_size -= length;
      code = &(code[length]);

    // グローバル変数
    } else if ((storage == SYMBOL_STORAGE_STATIC) || (storage == SYMBOL_STORAGE_EXTERN)) {
      length = snprintf(code, *code_rest_size, "_%s", token[symbol[symbol_index].token_id].name);
      *code_rest_size -= length;
      code = &(code[length]);
    }

  // 関数の引数
  } else if (kind == SYMBOL_TABLE_F_ARGUMENT) {
    length = snprintf(code, *code_rest_size, "[ebp+%d]", symbol[symbol_index].address);
    *code_rest_size -= length;
    code = &(code[length]);
  }

  return code;
}
static char* generate_code_binary_operator(/*{{{*/
  const int           binary_operator
  , char*             code
  , int*              code_rest_size
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
) {
  // TODO 色々足りなすぎる
  assert(is_pt_name("plus"  , pt[binary_operator], bnf));
  int length;

  const int left = pt[binary_operator].down;
  assert(left >= 0);
  const int right = pt[left].right;
  assert(right >= 0);

  code = generate_code_expression(left, code, code_rest_size, block, token, bnf, pt, symbol);
  code = generate_code_expression(right, code, code_rest_size, block, token, bnf, pt, symbol);

  length = snprintf(code, *code_rest_size, "pop edx\npop eax\n");
  *code_rest_size -= length;
  code = &(code[length]);

  if (is_pt_name("plus", pt[binary_operator], bnf)) {
    length = snprintf(code, *code_rest_size, "add  eax, edx\n");
    *code_rest_size -= length;
    code = &(code[length]);
  }

  //if      (0 == strcmp(str, "+" )) fprintf(fp, "add  eax, edx\n");

  //char *str = s.token[ope -> init].str;
  //if      (0 == strcmp(str, "+" )) fprintf(fp, "add  eax, edx\n");
  //else if (0 == strcmp(str, "-" )) fprintf(fp, "sub  eax, edx\n");
  //else if (0 == strcmp(str, "*" )) fprintf(fp, "imul eax, edx\n");
  //else if (0 == strcmp(str, "/" )) fprintf(fp, "idiv eax, edx\n");
  //else if (0 == strcmp(str, "%" )) fprintf(fp, "imod eax, edx\n");
  //else asm_compare(fp, str, label, ope -> init, "eax", "edx");
  //fprintf(fp, "push eax\n");

  return code;
}/*}}}*/
static char* generate_code_assign(/*{{{*/
  const int           assign
  , char*             code
  , int*              code_rest_size
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
) {
  // TODO 色々足りなすぎる
  assert(is_pt_name("equal"  , pt[assign], bnf));
  int length;

  const int left = pt[assign].down;
  assert(left >= 0);
  assert(is_pt_name("identifier"  , pt[left], bnf));

  const int right = pt[left].right;
  assert(right >= 0);

  // 先に右辺を計算
  code = generate_code_expression(right, code, code_rest_size, block, token, bnf, pt, symbol);

  // 左辺に代入
  length = snprintf(code, *code_rest_size, "mov ");
  *code_rest_size -= length;
  code = &(code[length]);

  code = write_identifier(left, code, code_rest_size, block, token, bnf, pt, symbol);

  length = snprintf(code, *code_rest_size, ", eax\n");
  *code_rest_size -= length;
  code = &(code[length]);

  return code;
}/*}}}*/

