#include "../include/common.h"
#include "../include/symbol.h"
#include "../include/pt_common.h"
#include "../min-bnf-parser/include/min-bnf-parser.h"
#include "../min-bnf-parser/include/text.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 関数プロトタイプ/*{{{*/
static void initialize_symbol_table_unit(SYMBOL* symbol, const int index, int* array);
static int cretae_block_recursive(
 const   int        token_begin_index
 , const int        token_end_index
 , const int        current_block_id
 , const int        up_block_id
 , BLOCK*           block
 , const LEX_TOKEN* token
 );
static int search_corresponding_rblock(const int begin, const int far_end, const LEX_TOKEN* token);
static int create_symbol_variable_recursive(
  const int symbol_empty_id
  , const int pt_top_index
  , const BLOCK* block
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
);
static int create_symbol_function_recursive(
  const int symbol_empty_id
  , const BLOCK* block
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
);
static int register_declaration(
  const int  kind
  , const int symbol_empty_id
  , const int declaration
  , const int block_here
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
);
static int register_function(
  const int  kind
  , const int symbol_empty_id
  , const int declaration
  , const int block_here
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
);
static void delete_declaration(const int declaration, const BNF* bnf, PARSE_TREE* pt);
static void delete_function(const int function_definition, const BNF* bnf, PARSE_TREE* pt);
static int register_parameter_declaration(
  const   int argument_id
  , const int function_id
  , const int kind
  , const int symbol_empty_id
  , const int parameter_declaration
  , const int block_here
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
);
static void register_type(
  const   int symbol_empty_id
  , const int declaration_specifiers
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
);
static int register_parameter_type_list(
  const   int  kind
  , const int function_id
  , const int symbol_empty_id
  , const int parameter_type_list
  , const BLOCK* block
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
);
static void register_direct_declarator(
  const   int symbol_empty_id
  , const int direct_declarator
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
);
static void register_pointer(
  const   int symbol_empty_id
  , const int pointer
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
);
static int search_identifier_recursive(
  const int declarator
  , const BNF* bnf
  , PARSE_TREE* pt
);
/*}}}*/
extern void create_symbol_table(const BLOCK* block, const LEX_TOKEN* token, const BNF* bnf, PARSE_TREE* pt, SYMBOL* symbol) {/*{{{*/
  // memberテーブルは初期化済みとする
  int empty_symbol_id = search_unused_symbol_index(symbol);
  empty_symbol_id = create_symbol_function_recursive(empty_symbol_id, block, token, bnf, pt, symbol);
  empty_symbol_id = create_symbol_variable_recursive(empty_symbol_id, 0, block, token, bnf, pt, symbol);
  delete_empty_external_declaration(bnf, pt);
  for (int i=0; i<empty_symbol_id; i++) symbol[i].used_size = empty_symbol_id;
}/*}}}*/
extern void initialize_symbol_table(SYMBOL* symbol, const int symbol_max_size, int* array, const int array_max_size) {/*{{{*/
  for (int i=0; i<array_max_size; i++) {
    array[i] = -1;
  }

  for (int i=0; i<symbol_max_size; i++) {
    symbol[i].id         = i;
    symbol[i].total_size = symbol_max_size;
    symbol[i].total_array_size = array_max_size;
    initialize_symbol_table_unit(symbol, i, array);
  }
}/*}}}*/
static void initialize_symbol_table_unit(SYMBOL* symbol, const int index, int* array) {/*{{{*/
  symbol[index].used_size       = 0;
  symbol[index].token_id        = -1;
  symbol[index].kind            = SYMBOL_TABLE_UNUSED;
  symbol[index].type_length     = SYMBOL_TYPE_MEDIUM;
  symbol[index].type_sign       = SYMBOL_TYPE_SIGNED;
  symbol[index].type_body       = SYMBOL_TYPE_INT;
  symbol[index].typedef_id      = -1;
  symbol[index].body_token_id   = -1;
  symbol[index].storage         = SYMBOL_STORAGE_AUTO;
  symbol[index].qualifier       = SYMBOL_QUALIFIER_VOLATILE;
  symbol[index].block           = -1;
  symbol[index].address         = -1;
  symbol[index].array           = array;
  symbol[index].array_size      = -1;
  symbol[index].byte            = -1;
  symbol[index].original_byte   = -1;
  symbol[index].function_id     = -1;
  symbol[index].argument_id     = -1;
  symbol[index].total_argument  = -1;
}/*}}}*/
extern int create_block(BLOCK* block, const int block_max_size, const LEX_TOKEN* token) {/*{{{*/
  assert(token[0].used_size < block_max_size);

  for (int i=0; i<block_max_size; i++) {
    block[i].id = i;
    block[i].total_size = block_max_size;
    block[i].used_size = token[0].used_size;
    block[i].here = -1;
    block[i].up = -1;
  }

  const int ret = cretae_block_recursive(0, token[0].used_size, 0, -1, block, token);
  return ret;
}/*}}}*/
static int cretae_block_recursive(/*{{{*/
 const   int        token_begin_index
 , const int        token_end_index
 , const int        current_block_id
 , const int        up_block_id
 , BLOCK*           block
 , const LEX_TOKEN* token
 ) {

  int new_block_id = current_block_id+1;
  for (int i=token_begin_index; i<token_end_index; i++) {
    const char c = token[i].src[token[i].begin];
    if (c == '{') {
      const int begin_inside = i;
      const int end_inside   = search_corresponding_rblock(begin_inside, token_end_index, token);
      block[begin_inside].here = new_block_id;
      block[begin_inside].up   = current_block_id;
      block[end_inside].here   = new_block_id;
      block[end_inside].up     = current_block_id;

      new_block_id = cretae_block_recursive(begin_inside+1, end_inside, new_block_id, current_block_id, block, token);

      i = end_inside;
    } else {
      block[i].here = current_block_id;
      block[i].up   = up_block_id;
    }
  }

  return new_block_id;
}

static int search_corresponding_rblock(const int begin, const int far_end, const LEX_TOKEN* token) {
  assert('{' == token[begin].src[token[begin].begin]);
  int block_count = 0;

  int ret;
  for (ret=begin; ret<far_end; ret++) {
    const char c = token[ret].src[token[ret].begin];
    if (c == '{') block_count++;
    if (c == '}') block_count--;
    if (block_count == 0) break;
  }

  assert(block_count == 0);
  return ret;
}/*}}}*/
extern void print_block(FILE* fp, const BLOCK* block, const LEX_TOKEN* token) {/*{{{*/
  for (int i=0; i<token[0].used_size; i++) {
    const LEX_TOKEN t = token[i];
    for (int j=t.begin; j<t.end; j++) {
      fprintf(fp, "%c", t.src[j]);
    }
    fprintf(fp, " ");
  }
  fprintf(fp, "\n");

  for (int i=0; i<block[0].used_size; i++) {
    const BLOCK b = block[i];
    const LEX_TOKEN t = token[i];
    for (int j=t.begin; j<t.end; j++) {
      if (b.here == -1) fprintf(fp, "-");
      else              fprintf(fp, "%d", b.here % 10);
    }
    fprintf(fp, " ");
  }
  fprintf(fp, "\n");

  for (int i=0; i<block[0].used_size; i++) {
    const BLOCK b = block[i];
    const LEX_TOKEN t = token[i];
    for (int j=t.begin; j<t.end; j++) {
      if (b.up == -1) fprintf(fp, "-");
      else            fprintf(fp, "%d", b.up % 10);
    }
    fprintf(fp, " ");
  }
  fprintf(fp, "\n");
}/*}}}*/
static int create_symbol_variable_recursive(/*{{{*/
  const int symbol_empty_id
  , const int pt_top_index
  , const BLOCK* block
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
) {

  assert(symbol_empty_id < symbol[0].total_size);
  int new_symbol_empty_id = symbol_empty_id;

  // 変数をシンボルテーブルに登録
  if (pt_top_index < 0) {
    ;
  }
  else if (is_pt_name("DECLARATION", pt[pt_top_index], bnf)) {
    const int next_index = pt[pt_top_index].right;

    const int declaration = pt_top_index;
    const int up = pt[declaration].up;
    assert(pt[declaration].down >= 0);

    const int init_declarator_list = search_pt_index_right("INIT_DECLARATOR_LIST", pt[declaration].down, pt, bnf);
    assert(init_declarator_list >= 0);

    const int init_declarator = search_pt_index_right("INIT_DECLARATOR", pt[init_declarator_list].down, pt, bnf);
    assert(init_declarator >= 0);

    const int declarator = search_pt_index_right("DECLARATOR", pt[init_declarator].down, pt, bnf);
    assert(declarator >= 0);

    const int direct_declarator = search_pt_index_right("DIRECT_DECLARATOR", pt[declarator].down, pt, bnf);
    assert(direct_declarator >= 0);

    // 関数定義ブロック内の変数宣言
    if (is_pt_name("COMPOUND_STATEMENT", pt[up], bnf)) {
      new_symbol_empty_id = register_declaration(SYMBOL_TABLE_VARIABLE, symbol_empty_id, declaration, block[pt[up].token_begin_index].here, token, bnf, pt, symbol);
      delete_declaration(declaration, bnf, pt);
    }

    // 関数の引数の超古いスタイル
    else if (is_pt_name("FUNCTION_DEFINITION", pt[up], bnf)) {
      fprintf(stderr, "ERROR: Very old style function definition. Not supported.\n");
      assert(0);
    }

    // for文の最初の部分で宣言できるように独自拡張した
    else if (is_pt_name("ITERATION_STATEMENT", pt[up], bnf)) {
      const int for_compound_statement = search_pt_index_right("STATEMENT", declaration, pt, bnf);
      assert(for_compound_statement >= 0);
      new_symbol_empty_id = register_declaration(SYMBOL_TABLE_VARIABLE, symbol_empty_id, declaration, block[pt[for_compound_statement].token_begin_index].here, token, bnf, pt, symbol);
      delete_declaration(declaration, bnf, pt);
    }

    // 関数プロトタイプ宣言、グローバル変数のいずれか
    else if (is_pt_name("EXTERNAL_DECLARATION", pt[up], bnf)) {
      const int down = pt[direct_declarator].down;
      assert(down >= 0);
      const int parameter_type_list = search_pt_index_right("PARAMETER_TYPE_LIST", down, pt, bnf);

      // 関数プロトタイプ宣言
      if (parameter_type_list >= 0) {
        new_symbol_empty_id = register_declaration(SYMBOL_TABLE_PROTOTYPE, symbol_empty_id, declaration, 0, token, bnf, pt, symbol);

        // 関数名と型を登録
        const int prototype_id = symbol_empty_id;
        new_symbol_empty_id = register_parameter_type_list(SYMBOL_TABLE_P_ARGUMENT, prototype_id, new_symbol_empty_id, parameter_type_list, block, token, bnf, pt, symbol);

        delete_declaration(declaration, bnf, pt);
      }

      // グローバル変数
      else {
        new_symbol_empty_id = register_declaration(SYMBOL_TABLE_VARIABLE, symbol_empty_id, declaration, 0, token, bnf, pt, symbol);
        delete_declaration(declaration, bnf, pt);
      }
    }

    if (next_index >= 0) new_symbol_empty_id = create_symbol_variable_recursive(new_symbol_empty_id, next_index, block, token, bnf, pt, symbol);
  }

  else {
    const int right = pt[pt_top_index].right;
    if (right >= 0) new_symbol_empty_id = create_symbol_variable_recursive(new_symbol_empty_id, right, block, token, bnf, pt, symbol);
    const int down  = pt[pt_top_index].down;
    if (down >= 0)  new_symbol_empty_id = create_symbol_variable_recursive(new_symbol_empty_id, down, block, token, bnf, pt, symbol);
  }

  return new_symbol_empty_id;
}/*}}}*/
static int create_symbol_function_recursive(/*{{{*/
  const int symbol_empty_id
  , const BLOCK* block
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
) {

  assert(symbol_empty_id < symbol[0].total_size);
  int new_symbol_empty_id = symbol_empty_id;

  int external_declaration = pt[0].down;
  while (external_declaration >= 0) {
    const int function_definition = search_pt_index_right("FUNCTION_DEFINITION", pt[external_declaration].down, pt, bnf);
    if (function_definition >= 0) {
      const int declarator = search_pt_index_right("DECLARATOR", pt[function_definition].down, pt, bnf);
      assert(declarator >= 0);

      const int direct_declarator = search_pt_index_right("DIRECT_DECLARATOR", pt[declarator].down, pt, bnf);
      assert(direct_declarator >= 0);

      // 関数名と型を登録
      const int function_id = new_symbol_empty_id;
      new_symbol_empty_id = register_function(SYMBOL_TABLE_FUNCTION, function_id, function_definition, 0, token, bnf, pt, symbol);
      const int parameter_type_list = search_pt_index_right("PARAMETER_TYPE_LIST", pt[direct_declarator].down, pt, bnf);
      new_symbol_empty_id = register_parameter_type_list(SYMBOL_TABLE_F_ARGUMENT, function_id, new_symbol_empty_id, parameter_type_list, block, token, bnf, pt, symbol);

      delete_function(function_definition, bnf, pt);
    }
    external_declaration = pt[external_declaration].right;
  }

  return new_symbol_empty_id;
}/*}}}*/
static int register_declaration(/*{{{*/
  const int  kind
  , const int symbol_empty_id
  , const int declaration
  , const int block_here
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
) {
  int declaration_specifiers = search_pt_index_right("DECLARATION_SPECIFIERS", pt[declaration].down, pt, bnf);
  assert(declaration_specifiers >= 0);

  const int init_declarator_list = search_pt_index_right("INIT_DECLARATOR_LIST", pt[declaration].down, pt, bnf);
  assert(init_declarator_list >= 0);

  const int init_declarator = search_pt_index_right("INIT_DECLARATOR", pt[init_declarator_list].down, pt, bnf);
  assert(init_declarator >= 0);

  const int declarator = search_pt_index_right("DECLARATOR", pt[init_declarator].down, pt, bnf);
  assert(declarator >= 0);

  symbol[symbol_empty_id].kind = kind;
  symbol[symbol_empty_id].block = block_here;
  register_type(symbol_empty_id, declaration_specifiers, bnf, pt, symbol);

  const int array_index = get_new_array_index(symbol[0].array, symbol[0].total_array_size);
  symbol[symbol_empty_id].array = &(symbol[0].array[array_index]);
  register_declarator(symbol_empty_id, declarator, token, bnf, pt, symbol);
  const int registered_array_index = get_new_array_index(symbol[0].array, symbol[0].total_array_size);
  symbol[symbol_empty_id].array_size = registered_array_index - array_index;

  return symbol_empty_id+1;
}/*}}}*/
static int register_function(/*{{{*/
  const int  kind
  , const int symbol_empty_id
  , const int function_definition
  , const int block_here
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
) {
  int declaration_specifiers = search_pt_index_right("DECLARATION_SPECIFIERS", pt[function_definition].down, pt, bnf);
  assert(declaration_specifiers >= 0);

  const int declarator = search_pt_index_right("DECLARATOR", pt[function_definition].down, pt, bnf);
  assert(declarator >= 0);

  symbol[symbol_empty_id].kind = kind;
  symbol[symbol_empty_id].block = block_here;
  register_type(symbol_empty_id, declaration_specifiers, bnf, pt, symbol);

  const int array_index = get_new_array_index(symbol[0].array, symbol[0].total_array_size);
  symbol[symbol_empty_id].array = &(symbol[0].array[array_index]);
  register_declarator(symbol_empty_id, declarator, token, bnf, pt, symbol);
  const int registered_array_index = get_new_array_index(symbol[0].array, symbol[0].total_array_size);
  symbol[symbol_empty_id].array_size = registered_array_index - array_index;

  return symbol_empty_id+1;
}/*}}}*/
static void delete_declaration(const int declaration, const BNF* bnf, PARSE_TREE* pt) {/*{{{*/
  // 必ず存在するノード
  const int init_declarator_list = search_pt_index_right("INIT_DECLARATOR_LIST", pt[declaration].down, pt, bnf);
  assert(init_declarator_list >= 0);

  const int init_declarator = search_pt_index_right("INIT_DECLARATOR", pt[init_declarator_list].down, pt, bnf);
  assert(init_declarator >= 0);

  const int declarator = search_pt_index_right("DECLARATOR", pt[init_declarator].down, pt, bnf);
  assert(declarator >= 0);

  const int identifier = search_identifier_recursive(declarator, bnf, pt);
  assert(identifier >= 0);

  // 存在しない可能性もあるノード
  const int equal       = search_pt_index_right("equal", pt[init_declarator].down, pt, bnf);
  const int initializer = search_pt_index_right("INITIALIZER", pt[init_declarator].down, pt, bnf);

  // 初期化がある場合、declarationを初期化に置換
  if (equal >= 0) {
    assert(initializer >= 0);

    if (pt[declaration].left >= 0) {
      pt[pt[declaration].left].right = equal;
      pt[equal].left = pt[declaration].left;
    }
    else {
      pt[pt[declaration].up].down = equal;
      pt[equal].left = -1;
    }
    pt[equal].up = pt[declaration].up;

    if (pt[declaration].right >= 0) {
      pt[pt[declaration].right].left = equal;
      pt[equal].right = pt[declaration].right;
    } else {
      pt[equal].right = -1;
    }
    pt[equal].down = identifier;

    pt[identifier].up = equal;
    pt[identifier].left = -1;
    pt[identifier].right = initializer;

    pt[initializer].up = equal;
    pt[initializer].left = identifier;
    pt[initializer].right = -1;
  }

  // 初期化がない場合、単にdeclarationを削除
  else {
    delete_pt_recursive(declaration, pt);
  }
}/*}}}*/
extern void print_symbol_table_line(FILE* fp, const int line, const LEX_TOKEN* token, const BNF* bnf, const PARSE_TREE* pt, const SYMBOL* symbol) {/*{{{*/
  fprintf(fp, "%03d | ", symbol[line].id);

  const int token_id = symbol[line].token_id;
  int rest = 15;
  if (token_id >= 0) {
    for (int i=token[token_id].begin; i<token[token_id].end; i++) {
      fprintf(fp, "%c", token[0].src[i]);
      rest--;
    }
    for (int i=0; i<rest; i++) fprintf(fp, " ");
  } else {
    fprintf(fp, "               ");
  }
  fprintf(fp, "| ");

  const int kind = symbol[line].kind;
  if (kind == SYMBOL_TABLE_UNUSED       ) fprintf(fp, "UNUSED        ");
  if (kind == SYMBOL_TABLE_VARIABLE     ) fprintf(fp, "VAR %2d:{}   " , symbol[line].block);
  if (kind == SYMBOL_TABLE_FUNCTION     ) fprintf(fp, "FNC %03d(%d){}", symbol[line].id, symbol[line].total_argument);
  if (kind == SYMBOL_TABLE_F_ARGUMENT   ) fprintf(fp, "ARG %03d(%d){}", symbol[line].function_id, symbol[line].argument_id);
  if (kind == SYMBOL_TABLE_PROTOTYPE    ) fprintf(fp, "PRT %03d(%d); ", symbol[line].id, symbol[line].total_argument);
  if (kind == SYMBOL_TABLE_P_ARGUMENT   ) fprintf(fp, "ARG %03d(%d); ", symbol[line].function_id, symbol[line].argument_id);
  if (kind == SYMBOL_TABLE_STRUCT_MEMBER) fprintf(fp, "STR %03d(%d); ", symbol[line].function_id, symbol[line].argument_id);

  fprintf(fp, " | ");

  if (symbol[line].qualifier   == SYMBOL_QUALIFIER_VOLATILE) fprintf(fp, "volatile");
  if (symbol[line].qualifier   == SYMBOL_QUALIFIER_CONST   ) fprintf(fp, "const   ");

  fprintf(fp, " | ");

  if (symbol[line].storage     == SYMBOL_STORAGE_AUTO     ) fprintf(fp, "auto    ");
  if (symbol[line].storage     == SYMBOL_STORAGE_STATIC   ) fprintf(fp, "static  ");
  if (symbol[line].storage     == SYMBOL_STORAGE_EXTERN   ) fprintf(fp, "extern  ");
  if (symbol[line].storage     == SYMBOL_STORAGE_REGISTER ) fprintf(fp, "register");

  fprintf(fp, " | ");

  if (symbol[line].type_body   == SYMBOL_TYPE_TYPEDEF_NAME) {
    const int alias_id = pt[symbol[line].typedef_id].token_begin_index;
    rest = 21;
    for (int i=token[alias_id].begin; i<token[alias_id].end; i++) {
      fprintf(fp, "%c", token[0].src[i]);
      rest--;
    }
    for (int i=0; i<rest; i++) fprintf(fp, " ");

  } else {
    if (symbol[line].type_sign   == SYMBOL_TYPE_SIGNED      ) fprintf(fp, "signed  ");
    if (symbol[line].type_sign   == SYMBOL_TYPE_UNSIGNED    ) fprintf(fp, "unsigned");
    fprintf(fp, " ");
    if (symbol[line].type_length == SYMBOL_TYPE_MEDIUM      ) fprintf(fp, "     ");
    if (symbol[line].type_length == SYMBOL_TYPE_SHORT       ) fprintf(fp, "short");
    if (symbol[line].type_length == SYMBOL_TYPE_LONG        ) fprintf(fp, "long ");
    fprintf(fp, " ");
    if (symbol[line].type_body   == SYMBOL_TYPE_INT         ) fprintf(fp, "int   ");
    if (symbol[line].type_body   == SYMBOL_TYPE_VOID        ) fprintf(fp, "void  ");
    if (symbol[line].type_body   == SYMBOL_TYPE_CHAR        ) fprintf(fp, "char  ");
    if (symbol[line].type_body   == SYMBOL_TYPE_FLOAT       ) fprintf(fp, "float ");
    if (symbol[line].type_body   == SYMBOL_TYPE_DOUBLE      ) fprintf(fp, "double");
    if (symbol[line].type_body   == SYMBOL_TYPE_STRUCT      ) fprintf(fp, "struct");
  }

  fprintf(fp, "| ");
  rest = 15;
  for (int i=0; i<symbol[line].array_size; i++) {
    const int byte = symbol[line].array[i];
    if (byte == ARRAY_TYPE_UNDEFINED) {
      rest -= fprintf(fp, "[]");
    } else if (byte == ARRAY_TYPE_POINTER) {
      rest -= fprintf(fp, "*");
    } else {
      rest -= fprintf(fp, "[%d]", byte);
    }
  }
  for (int i=0; i<rest; i++) fprintf(fp, " ");

  fprintf(fp, "block:%3d ", symbol[line].block);
  fprintf(fp, "address:%3d ", symbol[line].address);
  fprintf(fp, "byte:%3d ", symbol[line].byte);
  fprintf(fp, "original_byte:%3d ", symbol[line].original_byte);
}/*}}}*/
static int register_parameter_declaration(/*{{{*/
  const   int argument_id
  , const int function_id
  , const int kind
  , const int symbol_empty_id
  , const int parameter_declaration
  , const int block_here
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
) {

  int declaration_specifiers = search_pt_index_right("DECLARATION_SPECIFIERS", pt[parameter_declaration].down, pt, bnf);
  assert(declaration_specifiers >= 0);

  const int declarator = search_pt_index_right("DECLARATOR", pt[parameter_declaration].down, pt, bnf);

  symbol[symbol_empty_id].kind = kind;
  symbol[symbol_empty_id].block = block_here;
  symbol[symbol_empty_id].function_id = function_id;
  symbol[symbol_empty_id].argument_id = argument_id;
  register_type(symbol_empty_id, declaration_specifiers, bnf, pt, symbol);

  if (declarator >= 0) {
    const int array_index = get_new_array_index(symbol[0].array, symbol[0].total_array_size);
    symbol[symbol_empty_id].array = &(symbol[0].array[array_index]);
    register_declarator(symbol_empty_id, declarator, token, bnf, pt, symbol);
    const int registered_array_index = get_new_array_index(symbol[0].array, symbol[0].total_array_size);
    symbol[symbol_empty_id].array_size = registered_array_index - array_index;
  } else {
    symbol[symbol_empty_id].array_size = 0;
  }

  return symbol_empty_id+1;
}/*}}}*/
static void register_type(/*{{{*/
  const   int symbol_empty_id
  , const int declaration_specifiers
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
) {
  int specifier = pt[declaration_specifiers].down;

  while (specifier >= 0) {
    if (is_pt_name("TYPE_SPECIFIER", pt[specifier], bnf)) {
      specifier = register_type_specifier(symbol_empty_id, specifier, bnf, pt, symbol);
    } else if (is_pt_name("STORAGE_CLASS_SPECIFIER", pt[specifier], bnf)) {
      specifier = register_storage_class_specifier(symbol_empty_id, specifier, bnf, pt, symbol);
    } else if (is_pt_name("TYPE_QUALIFIER", pt[specifier], bnf)) {
      specifier = register_type_qualifier(symbol_empty_id, specifier, bnf, pt, symbol);
    } else {
      assert(0);
    }
  }
}/*}}}*/
extern int register_type_specifier(/*{{{*/
  const   int symbol_empty_id
  , const int type_specifier
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
) {
  assert(type_specifier >= 0);
  assert(is_pt_name("TYPE_SPECIFIER", pt[type_specifier], bnf));

  int specifier = type_specifier;
  while (specifier >= 0 && is_pt_name("TYPE_SPECIFIER", pt[specifier], bnf)) {
    const int keyword = pt[specifier].down;
    assert(keyword >= 0);
    if (is_pt_name("signed"       , pt[keyword], bnf)) symbol[symbol_empty_id].type_sign   = SYMBOL_TYPE_SIGNED;
    if (is_pt_name("unsigned"     , pt[keyword], bnf)) symbol[symbol_empty_id].type_sign   = SYMBOL_TYPE_UNSIGNED;

    if (is_pt_name("short"        , pt[keyword], bnf)) symbol[symbol_empty_id].type_length = SYMBOL_TYPE_SHORT;
    if (is_pt_name("long"         , pt[keyword], bnf)) symbol[symbol_empty_id].type_length = SYMBOL_TYPE_LONG;

    if (is_pt_name("int"          , pt[keyword], bnf)) symbol[symbol_empty_id].type_body   = SYMBOL_TYPE_INT;
    if (is_pt_name("void"         , pt[keyword], bnf)) symbol[symbol_empty_id].type_body   = SYMBOL_TYPE_VOID;
    if (is_pt_name("char"         , pt[keyword], bnf)) symbol[symbol_empty_id].type_body   = SYMBOL_TYPE_CHAR;
    if (is_pt_name("float"        , pt[keyword], bnf)) symbol[symbol_empty_id].type_body   = SYMBOL_TYPE_FLOAT;
    if (is_pt_name("double"       , pt[keyword], bnf)) symbol[symbol_empty_id].type_body   = SYMBOL_TYPE_DOUBLE;
    if (is_pt_name("struct"       , pt[keyword], bnf)) symbol[symbol_empty_id].type_body   = SYMBOL_TYPE_STRUCT;
    if (is_pt_name("typedef_name" , pt[keyword], bnf)) {
      symbol[symbol_empty_id].type_body   = SYMBOL_TYPE_TYPEDEF_NAME;
      symbol[symbol_empty_id].typedef_id  = keyword;
    }

    if ( is_pt_name("int"          , pt[keyword], bnf)
      || is_pt_name("void"         , pt[keyword], bnf)
      || is_pt_name("char"         , pt[keyword], bnf)
      || is_pt_name("float"        , pt[keyword], bnf)
      || is_pt_name("double"       , pt[keyword], bnf)
      || is_pt_name("struct"       , pt[keyword], bnf)
      || is_pt_name("typedef_name" , pt[keyword], bnf)
    ) {
      symbol[symbol_empty_id].body_token_id = pt[keyword].token_begin_index;
    }

    specifier = pt[specifier].right;
  }

  return specifier;
}/*}}}*/
extern int register_storage_class_specifier(/*{{{*/
  const   int symbol_empty_id
  , const int storage_class_specifier
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
) {
  assert(storage_class_specifier >= 0);
  assert(is_pt_name("STORAGE_CLASS_SPECIFIER", pt[storage_class_specifier], bnf));

  const int keyword = pt[storage_class_specifier].down;
  assert(keyword >= 0);
  if (is_pt_name("auto"    , pt[keyword], bnf)) symbol[symbol_empty_id].storage = SYMBOL_STORAGE_AUTO;
  if (is_pt_name("static"  , pt[keyword], bnf)) symbol[symbol_empty_id].storage = SYMBOL_STORAGE_STATIC;
  if (is_pt_name("extern"  , pt[keyword], bnf)) symbol[symbol_empty_id].storage = SYMBOL_STORAGE_EXTERN;
  if (is_pt_name("register", pt[keyword], bnf)) symbol[symbol_empty_id].storage = SYMBOL_STORAGE_REGISTER;
  return pt[storage_class_specifier].right;
}/*}}}*/
extern int register_type_qualifier(/*{{{*/
  const   int symbol_empty_id
  , const int type_qualifier
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
) {
  assert(type_qualifier >= 0);
  assert(is_pt_name("TYPE_QUALIFIER", pt[type_qualifier], bnf));

  const int keyword = pt[type_qualifier].down;
  assert(keyword >= 0);
  if (is_pt_name("volatile", pt[keyword], bnf)) symbol[symbol_empty_id].qualifier = SYMBOL_QUALIFIER_VOLATILE;
  if (is_pt_name("const"   , pt[keyword], bnf)) symbol[symbol_empty_id].qualifier = SYMBOL_QUALIFIER_CONST;
  return pt[type_qualifier].right;
}/*}}}*/
static int register_parameter_type_list(/*{{{*/
  const int  kind
  , const int function_id
  , const int symbol_empty_id
  , const int parameter_type_list
  , const BLOCK* block
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
) {

  const bool is_f_argument = (kind == SYMBOL_TABLE_F_ARGUMENT);
  const bool is_p_argument = (kind == SYMBOL_TABLE_P_ARGUMENT);

  // 関数定義と関数プロトタイプによって、引数のスコープを分岐
  int block_compound_statement;
  if (is_f_argument) {
    const int compound_statement = search_pt_index_right("COMPOUND_STATEMENT", pt[pt[parameter_type_list].up].up, pt, bnf);
    block_compound_statement = block[pt[compound_statement].token_begin_index].here;
  } else if (is_p_argument) {
    block_compound_statement = -1;
  } else {
    assert(0);
  }

  int argument_id = 0;
  int new_symbol_empty_id = symbol_empty_id;

  // 引数が存在しない場合
  if (parameter_type_list < 0) {
    symbol[function_id].total_argument = argument_id;
  }

  // 引数が存在する場合は登録する
  else {
    const int parameter_list = search_pt_index_right("PARAMETER_LIST", pt[parameter_type_list].down, pt, bnf);
    int parameter_declaration = pt[parameter_list].down;

    while (parameter_declaration >= 0) {
      if (!is_pt_name("PARAMETER_DECLARATION", pt[parameter_declaration], bnf)) {
        parameter_declaration = pt[parameter_declaration].right;
        continue;
      }

      const int tmp_id = register_parameter_declaration(
        argument_id
        , function_id
        , kind
        , new_symbol_empty_id
        , parameter_declaration
        , block_compound_statement
        , token
        , bnf
        , pt
        , symbol
      );

      new_symbol_empty_id = tmp_id;
      argument_id++;
      parameter_declaration = pt[parameter_declaration].right;
    }
    symbol[function_id].total_argument = argument_id;
  }

  return new_symbol_empty_id;
}/*}}}*/
static void delete_function(const int function_definition, const BNF* bnf, PARSE_TREE* pt) {/*{{{*/
  const int declarator = search_pt_index_right("DECLARATOR", pt[function_definition].down, pt, bnf);
  assert(declarator >= 0);

  const int direct_declarator = search_pt_index_right("DIRECT_DECLARATOR", pt[declarator].down, pt, bnf);
  assert(direct_declarator >= 0);

  const int identifier = search_pt_index_right("identifier", pt[direct_declarator].down, pt, bnf);
  assert(identifier >= 0);

  const int compound_statement = search_pt_index_right("COMPOUND_STATEMENT", pt[function_definition].down, pt, bnf);
  assert(compound_statement >= 0);

  // FUNCTION_DEFINITION直下に関数名のidentifierを追加
  pt[function_definition].down = identifier;
  pt[identifier].up    = function_definition;
  pt[identifier].down  = -1;
  pt[identifier].left  = -1;
  pt[identifier].right = compound_statement;

  pt[compound_statement].left = identifier;
}/*}}}*/
extern void delete_empty_external_declaration(const BNF* bnf, PARSE_TREE* pt) {/*{{{*/
  int index = pt[0].down;
  while (index >= 0) {
    assert(is_pt_name("EXTERNAL_DECLARATION", pt[index], bnf));
    if (pt[index].down < 0) delete_pt_recursive(index, pt);
    index = pt[index].right;
  }
}/*}}}*/
extern void register_declarator(/*{{{*/
  const   int symbol_empty_id
  , const int declarator
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
) {

  const int down = pt[declarator].down;
  assert(down >= 0);

  int direct_declarator;
  if (is_pt_name("POINTER", pt[down], bnf)) {
    register_pointer(symbol_empty_id, down, bnf, pt, symbol);
    direct_declarator = pt[down].right;
  } else {
    direct_declarator = down;
  }
  assert(direct_declarator >= 0);

  register_direct_declarator(symbol_empty_id, direct_declarator, token, bnf, pt, symbol);
}/*}}}*/
extern int get_new_array_index(const int* array, const int array_max_size) {/*{{{*/
  int index=0;
  for (index=0; index<array_max_size; index++) {
    if (array[index] == -1) break;
  }
  return index;
}/*}}}*/
static void register_pointer(/*{{{*/
  const   int symbol_empty_id
  , const int pointer
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
) {
  int array_index = get_new_array_index(symbol[0].array, symbol[0].total_array_size);
  int tmp_pointer = pointer;
  while (tmp_pointer >= 0) {
    if ( is_pt_name("star"    , pt[pt[tmp_pointer].down], bnf)) {
      symbol[0].array[array_index] = ARRAY_TYPE_POINTER;
      array_index++;
    }
    if ( is_pt_name("const"   , pt[pt[tmp_pointer].down], bnf)
      || is_pt_name("volatile", pt[pt[tmp_pointer].down], bnf)
    ) {
      fprintf(stderr, "ERROR: *const and *volatile is not supported\n");
      assert(0);
    }
    tmp_pointer = pt[pt[tmp_pointer].down].right;
  }
}/*}}}*/
static void register_direct_declarator(/*{{{*/
  const   int symbol_empty_id
  , const int direct_declarator
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
) {

  int array_index = get_new_array_index(symbol[0].array, symbol[0].total_array_size);

  assert(pt[direct_declarator].down >= 0);
  int right = rightside_pt_index(pt[direct_declarator].down, pt);

  while (right >= 0) {
    if ( is_pt_name("rbrace", pt[right], bnf)) {
      const int left = pt[right].left;
      assert(left >= 0);
      if ( is_pt_name("lbrace", pt[left], bnf)) {
        symbol[0].array[array_index] = ARRAY_TYPE_UNDEFINED;
      } else if ( is_pt_name("integer_constant", pt[left], bnf)) {
        const LEX_TOKEN t = token[pt[left].token_begin_index];
        char str[100];
        int str_length;
        for (str_length=0; str_length<t.end-t.begin; str_length++) {
          str[str_length] = t.src[t.begin+str_length];
        }
        str[str_length] = '\0';
        const int str_int = atoi(str);
        assert(str_int > 0);
        symbol[0].array[array_index] = str_int;
      } else {
        assert(0);
      }
      array_index++;

    } else if ( is_pt_name("identifier", pt[right], bnf)) {
      symbol[symbol_empty_id].token_id = pt[right].token_begin_index;

    } else if ( is_pt_name("DECLARATOR", pt[right], bnf)) {
      register_declarator(symbol_empty_id, right, token, bnf, pt, symbol);
    }

    right = pt[right].left;
  }
}/*}}}*/
static int search_identifier_recursive(/*{{{*/
  const int declarator
  , const BNF* bnf
  , PARSE_TREE* pt
) {
  assert(is_pt_name("DECLARATOR", pt[declarator], bnf));

  int index =declarator;
  while (index >= 0) {
    if (is_pt_name("DIRECT_DECLARATOR", pt[index], bnf)) {
      const int direct_declarator = index;
      index = search_pt_index_right("identifier", pt[direct_declarator].down, pt, bnf);
      if (index >= 0) break;
      index = search_pt_index_right("DECLARATOR", pt[direct_declarator].down, pt, bnf);
    } else if (is_pt_name("DECLARATOR", pt[index], bnf)) {
      index = search_pt_index_right("DIRECT_DECLARATOR", pt[index].down, pt, bnf);
    } else {
      assert(0);
    }
  }

  return index;
}/*}}}*/
extern int search_unused_symbol_index(const SYMBOL* symbol) {/*{{{*/
  int ret;
  for (ret=0; ret<symbol[0].total_size; ret++) {
    if (symbol[ret].kind == SYMBOL_TABLE_UNUSED) break;
  }
  return ret;
}/*}}}*/
extern void print_symbol_table_all(FILE* fp, const LEX_TOKEN* token, const BNF* bnf, const PARSE_TREE* pt, const SYMBOL* symbol) {/*{{{*/
  for (int i=0; symbol[i].kind != SYMBOL_TABLE_UNUSED; i++) {
    print_symbol_table_line(fp, i, token, bnf, pt, symbol);
    fprintf(fp, "\n");
  }
}/*}}}*/
extern int search_symbol_table_by_declare_token(const int token_index, const SYMBOL* symbol) {/*{{{*/
  int symbol_index;
  const int used_size = symbol[0].used_size;
  for (symbol_index=0; symbol_index<used_size; symbol_index++) {
    if (token_index == symbol[symbol_index].token_id) break;
  }
  if (symbol_index == used_size) symbol_index = -1;

  return symbol_index;
}/*}}}*/
extern int search_symbol_table(/*{{{*/
  const int           token_index
  , const BLOCK*      block
  , const LEX_TOKEN*  token
  , const BNF*        bnf
  , const PARSE_TREE* pt
  , const SYMBOL*     symbol
) {
  int symbol_index=0;
  const int used_size = symbol[0].used_size;
  while (symbol_index<used_size) {
    const int symbol_token_index = symbol[symbol_index].token_id;
    if (token_index < symbol_token_index) {
      symbol_index++;
      continue;
    }
    if (!is_same_token_str(token_index, symbol_token_index, token)) {
      symbol_index++;
      continue;
    }

    const char* storage_name = bnf[pt[symbol[symbol_index].storage].bnf_id].name;
    if ( (0 != strcmp("static", storage_name))
      && (0 != strcmp("extern", storage_name))
      && !(inside_scope(token_index, symbol_token_index, block))
    ) {
      symbol_index++;
      continue;
    }

    break;
  }

  assert(symbol_index < used_size);

  return symbol_index;
}/*}}}*/
