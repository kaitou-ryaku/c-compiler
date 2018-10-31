#ifndef __C_COMPILER_AST__
#define __C_COMPILER_AST__

#include "../min-bnf-parser/include/min-bnf-parser.h"
extern void translate_pt_to_ast(PARSE_TREE* pt, const BNF* bnf);

#endif
