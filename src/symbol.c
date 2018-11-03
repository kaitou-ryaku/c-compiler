#include "../include/common.h"
#include "../include/symbol.h"
#include "../include/pt_common.h"
#include "../min-bnf-parser/include/min-bnf-parser.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// 関数プロトタイプ/*{{{*/
static void initialize_symbol_table(SYMBOL* symbol, const int symbol_max_size);
static int cretae_block_recursive(
 const   int        begin_token_index
 , const int        end_token_index
 , const int        current_block_id
 , const int        up_block_id
 , BLOCK*           block
 , const LEX_TOKEN* token
 );
static int search_corresponding_rblock(const int begin, const int far_end, const LEX_TOKEN* token);
static void create_symbol_recursive(const int pt_top_index, const BLOCK* block, const LEX_TOKEN* token, const BNF* bnf, PARSE_TREE* pt, SYMBOL* symbol);
/*}}}*/
extern int create_symbol_table(const BLOCK* block, const LEX_TOKEN* token, const BNF* bnf, PARSE_TREE* pt, SYMBOL* symbol, const int symbol_max_size) {/*{{{*/
  initialize_symbol_table(symbol, symbol_max_size);
  create_symbol_recursive(0, block, token, bnf, pt, symbol);

  return 0;
}/*}}}*/
static void initialize_symbol_table(SYMBOL* symbol, const int symbol_max_size) {/*{{{*/
  for (int i=0; i<symbol_max_size; i++) {
    symbol[i].id         = i;
    symbol[i].total_size = symbol_max_size;
    symbol[i].used_size  = 0;
    symbol[i].lex_id     = -1;
    symbol[i].name       = NULL;
    symbol[i].kind       = -1;
    symbol[i].type       = -1;
    symbol[i].scope      = -1;
    symbol[i].addr       = -1;
    symbol[i].pointer    = -1;
    symbol[i].size       = -1;
    symbol[i].const_var  = false;
    symbol[i].const_addr = false;
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
 const   int        begin_token_index
 , const int        end_token_index
 , const int        current_block_id
 , const int        up_block_id
 , BLOCK*           block
 , const LEX_TOKEN* token
 ) {

  int new_block_id = current_block_id+1;
  for (int i=begin_token_index; i<end_token_index; i++) {
    const char c = token[i].src[token[i].begin];
    if (c == '{') {
      const int begin_inside = i;
      const int end_inside   = search_corresponding_rblock(begin_inside, end_token_index, token);
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
static void create_symbol_recursive(const int pt_top_index, const BLOCK* block, const LEX_TOKEN* token, const BNF* bnf, PARSE_TREE* pt, SYMBOL* symbol) {
  // 変数をシンボルテーブルに登録
  if (pt_top_index < 0) {
    ;
  }
  else if (is_pt_name("DECLARATION", pt[pt_top_index], bnf)) {
    print_parse_tree_unit(stderr, pt_top_index, pt, bnf, token);
    fprintf(stderr, "\n ");
    const int declaration = pt_top_index;
    const int up = pt[declaration].up;

    assert(pt[declaration].down >= 0);
    const int semicolon = rightside_pt_index(pt[declaration].down, pt);
    assert(is_pt_name("semicolon", pt[semicolon], bnf));

    const int init_declarator = pt[semicolon].left;
    assert(init_declarator >= 0);
    assert(is_pt_name("INIT_DECLARATOR", pt[init_declarator], bnf));

    const int declarator = pt[init_declarator].down;
    assert(is_pt_name("DECLARATOR", pt[declarator], bnf));
    const int direct_declarator = rightside_pt_index(pt[declarator].down, pt);
    assert(is_pt_name("DIRECT_DECLARATOR", pt[direct_declarator], bnf));
    //const int identifier = rightside_pt_index(pt[direct_declarator].down, pt);
    const int identifier = pt[direct_declarator].down;
    assert(is_pt_name("identifier", pt[identifier], bnf));

    // 関数定義ブロック内の変数宣言
    if (is_pt_name("COMPOUND_STATEMENT", pt[up], bnf)) {
      fprintf(stderr, "COMPOUND_STATEMENT ");
      print_parse_tree_unit(stderr, identifier, pt, bnf, token);
      fprintf(stderr, "\n ");
    }

    // 関数の引数
    else if (is_pt_name("FUNCTION_DEFINITION", pt[up], bnf)) {
      fprintf(stderr, "FUNCTION_DEFINITION ");
      print_parse_tree_unit(stderr, identifier, pt, bnf, token);
      fprintf(stderr, "\n ");
    }

    // 関数プロトタイプ宣言、グローバル変数のいずれか
    else if (is_pt_name("EXTERNAL_DECLARATION", pt[up], bnf)) {
      const int check = rightside_pt_index(identifier, pt);

      // 関数プロトタイプ宣言
      if (is_pt_name("rparen", pt[check], bnf)) {
        fprintf(stderr, "PROTOTYPE ");
        print_parse_tree_unit(stderr, identifier, pt, bnf, token);
        fprintf(stderr, "\n ");
      }

      // グローバル変数
      else {
        fprintf(stderr, "GLOBAL_VARIABLE ");
        print_parse_tree_unit(stderr, identifier, pt, bnf, token);
        fprintf(stderr, "\n ");
      }
    }

    const int right = pt[pt_top_index].right;
    if (right >= 0) create_symbol_recursive(right, block, token, bnf, pt, symbol);
  }

  else {
    const int right = pt[pt_top_index].right;
    if (right >= 0) create_symbol_recursive(right, block, token, bnf, pt, symbol);
    const int down  = pt[pt_top_index].down;
    if (down >= 0) create_symbol_recursive(down, block, token, bnf, pt, symbol);
  }
}
