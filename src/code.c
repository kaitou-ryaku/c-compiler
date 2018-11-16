#include "../min-bnf-parser/include/min-bnf-parser.h"
#include "../include/ast.h"
#include "../include/common.h"
#include "../include/pt_common.h"
#include "../include/code.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

// 関数プロトタイプ/*{{{*/
static int generate_code_function_definition(
  const int           function_definition
  , char*             code
  , const int         code_max_size
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
  for (int i=0; i<rest; i++) code[i]='\0';

  assert(is_pt_name("TRANSLATION_UNIT", pt[0], bnf));
  int external_declaration=pt[0].down;
  while (external_declaration >= 0) {
    const int down = pt[external_declaration].down;

    if (is_pt_name("FUNCTION_DEFINITION", pt[down], bnf)) {
      const int code_size = generate_code_function_definition(down, code, rest, block, token, bnf, pt, symbol);
      rest=rest-code_size;
      code=code+code_size;
    }

    external_declaration = pt[external_declaration].right;
  }
}/*}}}*/
static int generate_code_function_definition(/*{{{*/
  const int           function_definition
  , char*             code
  , const int         code_max_size
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
) {
  assert(is_pt_name("FUNCTION_DEFINITION", pt[function_definition], bnf));
  return snprintf(code, code_max_size, "FUNCTION_DEFINITION %d\n", function_definition);
}/*}}}*/
