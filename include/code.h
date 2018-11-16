#ifndef __C_COMPILER_CODE__
#define __C_COMPILER_CODE__

#include "../min-bnf-parser/include/min-bnf-parser.h"
#include "../include/common.h"
#include <stdbool.h>

void generate_code(
  char*               code
  , const int         code_max_size
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
);

#endif
