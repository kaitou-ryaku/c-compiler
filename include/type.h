#ifndef __C_COMPILER_TYPE__
#define __C_COMPILER_TYPE__

#include "../min-bnf-parser/include/min-bnf-parser.h"
void create_type_table(
  const BLOCK* block
  , const LEX_TOKEN* token
  , PARSE_TREE* pt
  , const BNF* bnf
  , TYPE* type
  , const int type_max_size
  , SYMBOL* member
  , const int member_max_size
  , int* array
  , const int array_max_size
);

#endif
