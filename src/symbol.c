#include "../include/common.h"
#include "../include/symbol.h"
#include "../include/pt_common.h"
#include "../min-bnf-parser/include/min-bnf-parser.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

const int SYMBOL_TABLE_UNUSED     = -1;
const int SYMBOL_TABLE_VARIABLE   = 0;
const int SYMBOL_TABLE_FUNCTION   = 1;
const int SYMBOL_TABLE_F_ARGUMENT = 2;
const int SYMBOL_TABLE_PROTOTYPE  = 3;
const int SYMBOL_TABLE_P_ARGUMENT = 4;

// 関数プロトタイプ/*{{{*/
static void initialize_symbol_table(SYMBOL* symbol, const int symbol_max_size);
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
static void print_symbol_table_line(FILE* fp, const int line, const LEX_TOKEN* token, const BNF* bnf, const SYMBOL* symbol);
/*}}}*/
extern int create_symbol_table(const BLOCK* block, const LEX_TOKEN* token, const BNF* bnf, PARSE_TREE* pt, SYMBOL* symbol, const int symbol_max_size) {/*{{{*/
  initialize_symbol_table(symbol, symbol_max_size);
  //create_symbol_function_recursive(0, block, token, bnf, pt, symbol);
  create_symbol_variable_recursive(0, 0, block, token, bnf, pt, symbol);
  for (int i=0; i<10; i++) {
    print_symbol_table_line(stderr, i, token, bnf, symbol);
    fprintf(stderr, "\n");
  }
  return 0;
}/*}}}*/
static void initialize_symbol_table(SYMBOL* symbol, const int symbol_max_size) {/*{{{*/
  for (int i=0; i<symbol_max_size; i++) {
    symbol[i].id              = i;
    symbol[i].total_size      = symbol_max_size;
    symbol[i].used_size       = 0;
    symbol[i].token_id        = -1;
    symbol[i].kind            = SYMBOL_TABLE_UNUSED;
    symbol[i].type            = -1;
    symbol[i].storage         = -1;
    symbol[i].qualify         = -1;
    symbol[i].pointer_qualify = -1;
    symbol[i].block           = -1;
    symbol[i].addr            = -1;
    symbol[i].pointer         = -1;
    symbol[i].size            = -1;
    symbol[i].arg_func_id     = -1;
    symbol[i].func_arg_num    = -1;
  }
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
    const int declaration = pt_top_index;
    const int up = pt[declaration].up;
    assert(pt[declaration].down >= 0);

    const int init_declarator = search_pt_index_right("INIT_DECLARATOR", pt[declaration].down, pt, bnf);
    assert(init_declarator >= 0);

    const int declarator = search_pt_index_right("DECLARATOR", pt[init_declarator].down, pt, bnf);
    assert(declarator >= 0);

    const int direct_declarator = search_pt_index_right("DIRECT_DECLARATOR", pt[declarator].down, pt, bnf);
    assert(direct_declarator >= 0);

    const int identifier = search_pt_index_right("identifier", pt[direct_declarator].down, pt, bnf);
    assert(identifier >= 0);

    // 関数定義ブロック内の変数宣言
    if (is_pt_name("COMPOUND_STATEMENT", pt[up], bnf)) {
      print_parse_tree_unit(stderr, identifier, pt, bnf, token);
      fprintf(stderr, " COMPOUND_STATEMENT");
      fprintf(stderr, "\n");
      new_symbol_empty_id = register_declaration(SYMBOL_TABLE_VARIABLE, symbol_empty_id, declaration, block[pt[up].token_begin_index].here, token, bnf, pt, symbol);
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
        print_parse_tree_unit(stderr, identifier, pt, bnf, token);
        fprintf(stderr, " PROTOTYPE");
        fprintf(stderr, "\n");
        new_symbol_empty_id = register_declaration(SYMBOL_TABLE_PROTOTYPE, symbol_empty_id, declaration, 0, token, bnf, pt, symbol);
      }

      // グローバル変数
      else {
        print_parse_tree_unit(stderr, identifier, pt, bnf, token);
        fprintf(stderr, " GLOBAL_VARIABLE");
        fprintf(stderr, "\n");
        new_symbol_empty_id = register_declaration(SYMBOL_TABLE_VARIABLE, symbol_empty_id, declaration, 0, token, bnf, pt, symbol);
      }
    }

    const int right = pt[pt_top_index].right;
    if (right >= 0) create_symbol_variable_recursive(new_symbol_empty_id, right, block, token, bnf, pt, symbol);
  }

  else {
    const int right = pt[pt_top_index].right;
    if (right >= 0) new_symbol_empty_id = create_symbol_variable_recursive(symbol_empty_id, right, block, token, bnf, pt, symbol);
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

      const int identifier = search_pt_index_right("identifier", pt[direct_declarator].down, pt, bnf);
      assert(identifier >= 0);

      print_parse_tree_unit(stderr, identifier, pt, bnf, token);
      fprintf(stderr, " FUNCTION_DEFINITION ");
      fprintf(stderr, "\n");
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

  // 必ず存在するノード
  int declaration_specifier = search_pt_index_right("DECLARATION_SPECIFIER", pt[declaration].down, pt, bnf);
  assert(declaration_specifier >= 0);

  const int init_declarator = search_pt_index_right("INIT_DECLARATOR", pt[declaration].down, pt, bnf);
  assert(init_declarator >= 0);

  const int declarator = search_pt_index_right("DECLARATOR", pt[init_declarator].down, pt, bnf);
  assert(declarator >= 0);

  const int direct_declarator = search_pt_index_right("DIRECT_DECLARATOR", pt[declarator].down, pt, bnf);
  assert(direct_declarator >= 0);

  const int identifier = search_pt_index_right("identifier", pt[direct_declarator].down, pt, bnf);
  assert(identifier >= 0);

  // 存在する可能性のあるノード
  int       pointer     = search_pt_index_right("POINTER", pt[declarator].down, pt, bnf);
  const int equal       = search_pt_index_right("equal", pt[init_declarator].down, pt, bnf);
  const int initializer = search_pt_index_right("INITIALIZER", pt[init_declarator].down, pt, bnf);

  // 登録
  symbol[symbol_empty_id].token_id = pt[identifier].token_begin_index;
  symbol[symbol_empty_id].kind = kind;
  symbol[symbol_empty_id].block = block_here;

  // 型
  while (is_pt_name("DECLARATION_SPECIFIER", pt[declaration_specifier], bnf)) {
    const int specifier = pt[declaration_specifier].down;
    assert(specifier >= 0);
    const int keyword = pt[pt[declaration_specifier].down].down;
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

    declaration_specifier = pt[declaration_specifier].right;
  }

  // ポインタ
  int pointer_depth = 0;
  while (pointer >= 0) {
    assert(pointer >= 0);

    if ( is_pt_name("star"    , pt[pt[pointer].down], bnf)) pointer_depth++;
    if ( is_pt_name("const"   , pt[pt[pointer].down], bnf)
      || is_pt_name("volatile", pt[pt[pointer].down], bnf)
    ) {
      symbol[symbol_empty_id].pointer_qualify = pt[pt[pointer].down].bnf_id;
    }

    pointer = pt[pt[pointer].down].right;
  }

  symbol[symbol_empty_id].pointer = pointer_depth;

  // TODO equalの処理

  return symbol_empty_id+1;
}/*}}}*/
static void print_symbol_table_line(FILE* fp, const int line, const LEX_TOKEN* token, const BNF* bnf, const SYMBOL* symbol) {/*{{{*/
  fprintf(fp, "id:%d ", symbol[line].id);

  if (symbol[line].kind == -1) fprintf(fp, "UNUSED    ");
  if (symbol[line].kind == 0 ) fprintf(fp, "VARIABLE  ");
  if (symbol[line].kind == 1 ) fprintf(fp, "ARGUMENT  ");
  if (symbol[line].kind == 2 ) fprintf(fp, "FUNCTION  ");
  if (symbol[line].kind == 3 ) fprintf(fp, "PROTOTYPE ");

  const int token_id = symbol[line].token_id;
  int rest = 15;
  if (token_id >= 0) {
    for (int i=token[token_id].begin; i<token[token_id].end; i++) {
      fprintf(fp, "%c", token[0].src[i]);
      rest--;
    }
    for (int i=0; i<rest; i++) {
      fprintf(fp, " ");
    }
  } else {
    fprintf(fp, "-------------- ");
  }

  if (NULL != bnf[symbol[line].type].name) fprintf(fp, "%10s ", bnf[symbol[line].type].name);
  else fprintf(fp, "---------- ");

  if (NULL != bnf[symbol[line].storage].name) fprintf(fp, "%10s ", bnf[symbol[line].storage].name);
  else fprintf(fp, "---------- ");

  if (NULL != bnf[symbol[line].qualify].name) fprintf(fp, "%10s ", bnf[symbol[line].qualify].name);
  else fprintf(fp, "---------- ");

  fprintf(fp, "pointer:%3d ", symbol[line].pointer);

  if (NULL != bnf[symbol[line].pointer_qualify].name) fprintf(fp, "%10s ", bnf[symbol[line].pointer_qualify].name);
  else fprintf(fp, "---------- ");

  fprintf(fp, "block:%3d ", symbol[line].block);
  fprintf(fp, "addr:%3d ", symbol[line].addr);
  fprintf(fp, "size:%3d ", symbol[line].size);
  fprintf(fp, "arg_func_id:%3d ", symbol[line].arg_func_id);
  fprintf(fp, "func_arg_num:%3d", symbol[line].func_arg_num);
}/*}}}*/
