#include "../include/common.h"
#include "../include/symbol.h"
#include "../include/pt_common.h"
#include "../min-bnf-parser/include/min-bnf-parser.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const int SYMBOL_TABLE_UNUSED     = -1;
const int SYMBOL_TABLE_VARIABLE   = 0;
const int SYMBOL_TABLE_FUNCTION   = 1;
const int SYMBOL_TABLE_F_ARGUMENT = 2;
const int SYMBOL_TABLE_PROTOTYPE  = 3;
const int SYMBOL_TABLE_P_ARGUMENT = 4;

// 関数プロトタイプ/*{{{*/
static void initialize_symbol_table(SYMBOL* symbol, const int symbol_max_size, int* array, const int array_size);
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
static void print_symbol_table_line(FILE* fp, const int line, const LEX_TOKEN* token, const BNF* bnf, const SYMBOL* symbol);
static bool delete_pt_recursive(const int index, PARSE_TREE* pt);
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
static int register_pointer(
  const   int symbol_empty_id
  , const int pointer
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
);
static int register_array(
  const   int symbol_empty_id
  , const int lbrace
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
void delete_empty_external_declaration(const BNF* bnf, PARSE_TREE* pt);
/*}}}*/
extern int create_symbol_table(const BLOCK* block, const LEX_TOKEN* token, const BNF* bnf, PARSE_TREE* pt, SYMBOL* symbol, const int symbol_max_size, int* array, const int array_max_size) {/*{{{*/
  initialize_symbol_table(symbol, symbol_max_size, array, array_max_size);

  int empty_symbol_id = 0;
  empty_symbol_id = create_symbol_function_recursive(empty_symbol_id, block, token, bnf, pt, symbol);
  empty_symbol_id = create_symbol_variable_recursive(empty_symbol_id, 0, block, token, bnf, pt, symbol);
  delete_empty_external_declaration(bnf, pt);

  for (int i=0; i<empty_symbol_id; i++) {
    print_symbol_table_line(stderr, i, token, bnf, symbol);
    fprintf(stderr, "\n");
  }
  return 0;
}/*}}}*/
static void initialize_symbol_table(SYMBOL* symbol, const int symbol_max_size, int* array, const int array_max_size) {/*{{{*/
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
  symbol[index].type            = -1;
  symbol[index].storage         = -1;
  symbol[index].qualify         = -1;
  symbol[index].pointer_qualify = -1;
  symbol[index].block           = -1;
  symbol[index].addr            = -1;
  symbol[index].pointer         = -1;
  symbol[index].array           = array;
  symbol[index].array_begin     = -1;
  symbol[index].array_end       = -1;
  symbol[index].size            = -1;
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

    const int identifier = search_pt_index_right("identifier", pt[direct_declarator].down, pt, bnf);
    assert(identifier >= 0);

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

    // 関数プロトタイプ宣言、グローバル変数のいずれか
    else if (is_pt_name("EXTERNAL_DECLARATION", pt[up], bnf)) {

      const int rparen = search_pt_index_right("rparen", identifier, pt, bnf);
      // 関数プロトタイプ宣言
      if (rparen >= 0) {
        new_symbol_empty_id = register_declaration(SYMBOL_TABLE_PROTOTYPE, symbol_empty_id, declaration, 0, token, bnf, pt, symbol);

        // 関数名と型を登録
        const int prototype_id = symbol_empty_id;
        const int parameter_type_list = search_pt_index_right("PARAMETER_TYPE_LIST", pt[direct_declarator].down, pt, bnf);
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

  const int direct_declarator = search_pt_index_right("DIRECT_DECLARATOR", pt[declarator].down, pt, bnf);
  assert(direct_declarator >= 0);

  const int identifier = search_pt_index_right("identifier", pt[direct_declarator].down, pt, bnf);
  assert(identifier >= 0);

  const int pointer = search_pt_index_right("POINTER", pt[declarator].down, pt, bnf);

  const int lbrace = search_pt_index_right("lbrace", identifier, pt, bnf);

  symbol[symbol_empty_id].token_id = pt[identifier].token_begin_index;
  symbol[symbol_empty_id].kind = kind;
  symbol[symbol_empty_id].block = block_here;
  register_type(symbol_empty_id, declaration_specifiers, bnf, pt, symbol);
  register_pointer(symbol_empty_id, pointer, bnf, pt, symbol);
  register_array(symbol_empty_id, lbrace, token, bnf, pt, symbol);

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

  const int direct_declarator = search_pt_index_right("DIRECT_DECLARATOR", pt[declarator].down, pt, bnf);
  assert(direct_declarator >= 0);

  const int identifier = search_pt_index_right("identifier", pt[direct_declarator].down, pt, bnf);
  assert(identifier >= 0);

  const int pointer = search_pt_index_right("POINTER", pt[declarator].down, pt, bnf);

  symbol[symbol_empty_id].token_id = pt[identifier].token_begin_index;
  symbol[symbol_empty_id].kind = kind;
  symbol[symbol_empty_id].block = block_here;
  register_type(symbol_empty_id, declaration_specifiers, bnf, pt, symbol);
  register_pointer(symbol_empty_id, pointer, bnf, pt, symbol);

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

  const int direct_declarator = search_pt_index_right("DIRECT_DECLARATOR", pt[declarator].down, pt, bnf);
  assert(direct_declarator >= 0);

  const int identifier = search_pt_index_right("identifier", pt[direct_declarator].down, pt, bnf);
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
static void print_symbol_table_line(FILE* fp, const int line, const LEX_TOKEN* token, const BNF* bnf, const SYMBOL* symbol) {/*{{{*/
  fprintf(fp, "%03d | ", symbol[line].id);

  if (symbol[line].kind == -1) fprintf(fp, "UNUSED     ");
  if (symbol[line].kind == 0 ) fprintf(fp, "VARIABLE   ");
  if (symbol[line].kind == 1 ) fprintf(fp, "FUNCTION   ");
  if (symbol[line].kind == 2 ) fprintf(fp, "F_ARGUMENT ");
  if (symbol[line].kind == 3 ) fprintf(fp, "PROTOTYPE  ");
  if (symbol[line].kind == 4 ) fprintf(fp, "P_ARGUMENT ");

  fprintf(fp, "| ");
  if (symbol[line].storage >= 0) fprintf(fp, "%-8s", bnf[symbol[line].storage].name);
  else fprintf(fp, "        ");

  fprintf(fp, "| ");
  if (symbol[line].qualify >= 0) fprintf(fp, "%-8s", bnf[symbol[line].qualify].name);
  else fprintf(fp, "        ");

  fprintf(fp, "| ");
  if (symbol[line].type >= 0) fprintf(fp, "%-8s", bnf[symbol[line].type].name);
  else fprintf(fp, "        ");

  fprintf(fp, "| ");
  int rest = 5;
  for (int i=0; i<symbol[line].pointer; i++) {
    rest -= fprintf(fp, "*");
  }
  for (int i=0; i<rest; i++) fprintf(fp, " ");

  fprintf(fp, "| ");
  if (symbol[line].pointer_qualify >= 0) fprintf(fp, "%-8s", bnf[symbol[line].pointer_qualify].name);
  else fprintf(fp, "        ");

  fprintf(fp, "| ");
  const int token_id = symbol[line].token_id;
  rest = 15;
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
  rest = 15;
  for (int i=symbol[line].array_begin; i<symbol[line].array_end; i++) {
    const int size = symbol[line].array[i];
    if (size == -2) {
      rest -= fprintf(fp, "[]");
    } else {
      rest -= fprintf(fp, "[%d]", size);
    }
  }
  for (int i=0; i<rest; i++) fprintf(fp, " ");

  fprintf(fp, "block:%3d ", symbol[line].block);
  fprintf(fp, "addr:%3d ", symbol[line].addr);
  fprintf(fp, "size:%3d ", symbol[line].size);
  fprintf(fp, "func:%3d ", symbol[line].function_id);
  fprintf(fp, "arg:%3d ", symbol[line].argument_id);
  fprintf(fp, "arg_tot:%3d ", symbol[line].total_argument);
}/*}}}*/
static bool delete_pt_recursive(const int index, PARSE_TREE* pt) {/*{{{*/
  bool ret;
  const PARSE_TREE del = pt[index];

  if (index < 0) {
    ret = false;
  }

  else {
    const int up    = pt[index].up;
    const int left  = pt[index].left;
    const int right = pt[index].right;

    if (up >= 0) {
      if      ((left < 0) && (right >= 0)) {
        assert(pt[up].down == index);
        pt[up].down = right;
      }
      else if ((left < 0) && (right <  0)) {
        assert(pt[up].down == index);
        pt[up].down = -1;
      }
    }

    if (left  >= 0) pt[left].right = del.right;
    if (right >= 0) pt[right].left = del.left;

    ret = true;
  }

  return ret;
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

  // 関数プロトタイプや関数定義のvoidなど、identifierが存在しないパターンを考慮して分岐
  if (declarator >= 0) {
    const int pointer = search_pt_index_right("POINTER", pt[declarator].down, pt, bnf);
    register_pointer(symbol_empty_id, pointer, bnf, pt, symbol);

    const int direct_declarator = search_pt_index_right("DIRECT_DECLARATOR", pt[declarator].down, pt, bnf);

    symbol[symbol_empty_id].token_id = -1;
    if (direct_declarator >= 0) {
      const int identifier = search_pt_index_right("identifier", pt[direct_declarator].down, pt, bnf);
      if (identifier >= 0) symbol[symbol_empty_id].token_id = pt[identifier].token_begin_index;
    }
  }

  symbol[symbol_empty_id].kind = kind;
  symbol[symbol_empty_id].block = block_here;
  symbol[symbol_empty_id].function_id = function_id;
  symbol[symbol_empty_id].argument_id = argument_id;
  register_type(symbol_empty_id, declaration_specifiers, bnf, pt, symbol);

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
    const int keyword = pt[specifier].down;
    assert(keyword >= 0);

    if (is_pt_name("TYPE_SPECIFIER", pt[specifier], bnf)) {
      assert(symbol[symbol_empty_id].type < 0);
      symbol[symbol_empty_id].type = pt[keyword].bnf_id;
    }
    if (is_pt_name("STORAGE_CLASS_SPECIFIER", pt[specifier], bnf)) {
      assert(symbol[symbol_empty_id].storage < 0);
      symbol[symbol_empty_id].storage = pt[keyword].bnf_id;
    }
    if (is_pt_name("TYPE_QUALIFIER", pt[specifier], bnf)) {
      assert(symbol[symbol_empty_id].qualify < 0);
      symbol[symbol_empty_id].qualify = pt[keyword].bnf_id;
    }

    specifier = pt[specifier].right;
  }
}/*}}}*/
static int register_pointer(/*{{{*/
  const   int symbol_empty_id
  , const int pointer
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
) {

  int tmp_pointer = pointer;
  int pointer_depth = 0;
  while (tmp_pointer >= 0) {
    if ( is_pt_name("star"    , pt[pt[tmp_pointer].down], bnf)) pointer_depth++;
    if ( is_pt_name("const"   , pt[pt[tmp_pointer].down], bnf)
      || is_pt_name("volatile", pt[pt[tmp_pointer].down], bnf)
    ) {
      symbol[symbol_empty_id].pointer_qualify = pt[pt[tmp_pointer].down].bnf_id;
    }

    tmp_pointer = pt[pt[tmp_pointer].down].right;
  }
  symbol[symbol_empty_id].pointer = pointer_depth;

  return pointer_depth;
}/*}}}*/
static int register_array(/*{{{*/
  const   int symbol_empty_id
  , const int lbrace
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* symbol
) {

  // int a[5][6][7];
  // array_begin_id = 100
  // array_end_id   = 103
  // symbol[0].array = symbol[1].array = ... = symbol[symbol_empty_id].array
  // symbol[0].array[100] = 5
  // symbol[0].array[101] = 6
  // symbol[0].array[102] = 7
  // symbol[0].array[103] = -1
  // symbol[0].array[104] = -1
  //
  // int a[][];
  // array_begin_id = 100
  // array_end_id   = 102
  // symbol[0].array[100] = -2
  // symbol[0].array[101] = -2
  // symbol[0].array[102] = -1
  // symbol[0].array[103] = -1
  //
  // int a;
  // array_begin_id = 100
  // array_end_id   = 100
  // symbol[0].array[100] = -1

  int array_empty_id;
  for (array_empty_id=0; array_empty_id<symbol[symbol_empty_id].total_array_size; array_empty_id++) {
    if (symbol[symbol_empty_id].array[array_empty_id] == -1) {
      // 配列が存在しない場合の設定
      symbol[symbol_empty_id].array_begin = array_empty_id;
      symbol[symbol_empty_id].array_end   = array_empty_id;
      break;
    }
  }
  assert(array_empty_id < symbol[0].total_array_size);

  int tmp_lbrace = lbrace;

  while (tmp_lbrace >= 0) {
    const int right = pt[tmp_lbrace].right;

    if (       is_pt_name("rbrace"  , pt[right], bnf)) {
      symbol[symbol_empty_id].array[array_empty_id] = -2;

    } else if (is_pt_name("integer_constant"  , pt[right], bnf)) {
      const LEX_TOKEN t = token[pt[right].token_begin_index];
      char str[100];
      int str_length;
      for (str_length=0; str_length<t.end-t.begin; str_length++) {
        str[str_length] = t.src[t.begin+str_length];
      }
      str[str_length] = '\0';
      const int str_int = atoi(str);
      assert(str_int > 0);
      symbol[symbol_empty_id].array[array_empty_id] = str_int;

    } else {
      assert(0);
    }

    array_empty_id++;
    symbol[symbol_empty_id].array_end = array_empty_id;
    tmp_lbrace = search_pt_index_right("lbrace", pt[tmp_lbrace].right, pt, bnf);
  }

  return array_empty_id;
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

      // voidの場合
      const int type_id = symbol[new_symbol_empty_id].type;
      if ((type_id < 0) || (0 == strcmp("void", bnf[type_id].name))) {
        initialize_symbol_table_unit(symbol, new_symbol_empty_id, symbol[0].array);

      // int等の場合
      } else {
        // 関数定義で変数名が無かったらエラー
        if (symbol[new_symbol_empty_id].token_id < 0) {
          fprintf(stderr, "ERROR: Function definition argument lacks identifier.\n");
          assert(0);
        }

        new_symbol_empty_id = tmp_id;
        argument_id++;
      }

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
void delete_empty_external_declaration(const BNF* bnf, PARSE_TREE* pt) {/*{{{*/
  int index = pt[0].down;
  while (index >= 0) {
    assert(is_pt_name("EXTERNAL_DECLARATION", pt[index], bnf));
    if (pt[index].down < 0) delete_pt_recursive(index, pt);
    index = pt[index].right;
  }
}/*}}}*/
