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
);

int search_type_table_by_declare_token( const int token_index , const BNF* bnf , const TYPE* type);
void print_type_table(FILE* fp, const LEX_TOKEN* token, const BNF* bnf, const TYPE* type);

#endif
