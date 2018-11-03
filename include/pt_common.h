#ifndef __C_COMPILER_PT_COMMON__
#define __C_COMPILER_PT_COMMON__

#include "../min-bnf-parser/include/min-bnf-parser.h"

bool is_pt_name(const char* name, const PARSE_TREE pt, const BNF* bnf);
extern int rightside_pt_index(const int pt_index, const PARSE_TREE* pt);

#endif
