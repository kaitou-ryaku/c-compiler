#ifndef __C_COMPILER_MEMORY__
#define __C_COMPILER_MEMORY__

#include "common.h"
#include "../min-bnf-parser/include/min-bnf-parser.h"

int sizeof_symbol_array(const int byte, const int* array, const int array_size);
extern void register_symbol_size(const TYPE* type, SYMBOL* symbol);
extern void register_type_and_symbol_size(
  const BLOCK* block
  , const LEX_TOKEN* token
  , const BNF* bnf
  , const PARSE_TREE* pt
  , TYPE* type
  , SYMBOL* symbol
);

#endif
