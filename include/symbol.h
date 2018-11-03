#ifndef __C_COMPILER_SYMBOL__
#define __C_COMPILER_SYMBOL__

#include "../min-bnf-parser/include/min-bnf-parser.h"

int create_symbol_table(const BLOCK* block, const LEX_TOKEN* token, const BNF* bnf, PARSE_TREE* pt, SYMBOL* symbol, const int symbol_max_size);
int create_block(BLOCK* block, const int block_max_size, const LEX_TOKEN* token);
void print_block(FILE* fp, const BLOCK* block, const LEX_TOKEN* token);

#endif
