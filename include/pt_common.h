#ifndef __C_COMPILER_PT_COMMON__
#define __C_COMPILER_PT_COMMON__

#include "../min-bnf-parser/include/min-bnf-parser.h"

bool is_pt_name(const char* name, const PARSE_TREE pt, const BNF* bnf);
int rightside_pt_index(const int pt_index, const PARSE_TREE* pt);
int search_pt_index_right(const char *name, const int pt_index, const PARSE_TREE* pt, const BNF* bnf);
int search_pt_index_left(const char *name, const int pt_index, const PARSE_TREE* pt, const BNF* bnf);
int search_pt_index_up(const char *name, const int pt_index, const PARSE_TREE* pt, const BNF* bnf);
bool delete_pt_recursive(const int index, PARSE_TREE* pt);

#endif
