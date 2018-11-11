#ifndef __C_COMPILER_SYMBOL__
#define __C_COMPILER_SYMBOL__

#include "../min-bnf-parser/include/min-bnf-parser.h"

int create_symbol_table(const BLOCK* block, const LEX_TOKEN* token, const BNF* bnf, PARSE_TREE* pt, SYMBOL* symbol, const int symbol_max_size, int* array, const int array_max_size);
int create_block(BLOCK* block, const int block_max_size, const LEX_TOKEN* token);
void print_block(FILE* fp, const BLOCK* block, const LEX_TOKEN* token);
void register_declarator(const int symbol_empty_id, const int declarator, const LEX_TOKEN* token, const BNF* bnf, PARSE_TREE* pt, SYMBOL* symbol);
void initialize_symbol_table(SYMBOL* symbol, const int symbol_max_size, int* array, const int array_size);
int search_unused_symbol_index(const SYMBOL* symbol);
void print_symbol_table_line(FILE* fp, const int line, const LEX_TOKEN* token, const BNF* bnf, const SYMBOL* symbol);
int get_new_array_index(const int* array, const int array_max_size);
extern void delete_empty_external_declaration(const BNF* bnf, PARSE_TREE* pt);

static const int SYMBOL_TABLE_UNUSED     = -1;
static const int SYMBOL_TABLE_VARIABLE   = 0;
static const int SYMBOL_TABLE_FUNCTION   = 1;
static const int SYMBOL_TABLE_F_ARGUMENT = 2;
static const int SYMBOL_TABLE_PROTOTYPE  = 3;
static const int SYMBOL_TABLE_P_ARGUMENT = 4;
static const int SYMBOL_TABLE_STRUCT_MEMBER = 5;

static const int ARRAY_TYPE_POINTER = 0;
static const int ARRAY_TYPE_UNDEFINED_SIZE = -2;

#endif
