#ifndef __C_COMPILER_TYPE__
#define __C_COMPILER_TYPE__

#include "../min-bnf-parser/include/min-bnf-parser.h"
int create_default_type(const LEX_TOKEN* token, PARSE_TREE* pt, const BNF* bnf, TYPE* type, const int type_max_size);

#endif
