#ifndef __C_COMPILER_PT_COMMON__
#define __C_COMPILER_PT_COMMON__

#include "../min-bnf-parser/include/min-bnf-parser.h"
#include <stdbool.h>

bool is_pt_name(const char* name, const PARSE_TREE pt, const BNF* bnf);
int rightside_pt_index(const int pt_index, const PARSE_TREE* pt);
int search_pt_index_right(const char *name, const int pt_index, const PARSE_TREE* pt, const BNF* bnf);
int search_pt_index_left(const char *name, const int pt_index, const PARSE_TREE* pt, const BNF* bnf);
int search_pt_index_up(const char *name, const int pt_index, const PARSE_TREE* pt, const BNF* bnf);
bool delete_pt_recursive(const int index, PARSE_TREE* pt);
bool is_same_token_str(const int a, const int b, const LEX_TOKEN* token);
bool inside_scope(const int token_child_index, const int token_parent_index, const BLOCK* block);

#endif
