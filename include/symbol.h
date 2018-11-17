#ifndef __C_COMPILER_SYMBOL__
#define __C_COMPILER_SYMBOL__

#include "../min-bnf-parser/include/min-bnf-parser.h"

void create_symbol_table(const BLOCK* block, const LEX_TOKEN* token, const BNF* bnf, PARSE_TREE* pt, SYMBOL* symbol);
int create_block(BLOCK* block, const int block_max_size, const LEX_TOKEN* token);
void print_block(FILE* fp, const BLOCK* block, const LEX_TOKEN* token);
void register_declarator(const int symbol_empty_id, const int declarator, const LEX_TOKEN* token, const BNF* bnf, PARSE_TREE* pt, SYMBOL* symbol);
void initialize_symbol_table(SYMBOL* symbol, const int symbol_max_size, int* array, const int array_size);
int search_unused_symbol_index(const SYMBOL* symbol);
void print_symbol_table_line(FILE* fp, const int line, const LEX_TOKEN* token, const BNF* bnf, const PARSE_TREE* pt, const SYMBOL* symbol);
int get_new_array_index(const int* array, const int array_max_size);
void delete_empty_external_declaration(const BNF* bnf, PARSE_TREE* pt);
void print_symbol_table_all(FILE* fp, const LEX_TOKEN* token, const BNF* bnf, const PARSE_TREE* pt, const SYMBOL* symbol);
int search_symbol_table_by_declare_token(const int token_index, const SYMBOL* symbol);
int search_symbol_table(const int token_index, const BLOCK* block, const LEX_TOKEN* token, const BNF* bnf, const PARSE_TREE* pt, const SYMBOL* symbol);

static const int SYMBOL_TABLE_UNUSED     = -1;
static const int SYMBOL_TABLE_VARIABLE   = 0;
static const int SYMBOL_TABLE_FUNCTION   = 1;
static const int SYMBOL_TABLE_F_ARGUMENT = 2;
static const int SYMBOL_TABLE_PROTOTYPE  = 3;
static const int SYMBOL_TABLE_P_ARGUMENT = 4;
static const int SYMBOL_TABLE_STRUCT_MEMBER = 5;

static const int ARRAY_TYPE_POINTER       = 0;
static const int ARRAY_TYPE_UNDEFINED     = -2;

static const int SYMBOL_TYPE_SIGNED       = 0;
static const int SYMBOL_TYPE_UNSIGNED     = 1;

static const int SYMBOL_TYPE_MEDIUM       = 0;
static const int SYMBOL_TYPE_SHORT        = 1;
static const int SYMBOL_TYPE_LONG         = 2;

static const int SYMBOL_TYPE_INT          = 0;
static const int SYMBOL_TYPE_VOID         = 1;
static const int SYMBOL_TYPE_CHAR         = 2;
static const int SYMBOL_TYPE_FLOAT        = 3;
static const int SYMBOL_TYPE_DOUBLE       = 4;
static const int SYMBOL_TYPE_STRUCT       = 5;
static const int SYMBOL_TYPE_TYPEDEF_NAME = 6;

static const int SYMBOL_STORAGE_AUTO      = 0;
static const int SYMBOL_STORAGE_STATIC    = 1;
static const int SYMBOL_STORAGE_EXTERN    = 2;
static const int SYMBOL_STORAGE_REGISTER  = 3;

static const int SYMBOL_QUALIFIER_VOLATILE = 0;
static const int SYMBOL_QUALIFIER_CONST    = 1;

#endif
