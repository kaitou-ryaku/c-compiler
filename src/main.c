#include "../min-bnf-parser/include/min-bnf-parser.h"
#include "../include/ast.h"
#include "../include/common.h"
#include "../include/symbol.h"
#include "../include/typedef.h"
#include "../include/type.h"
#include "../include/memory.h"
#include "../include/code.h"
#include <stdio.h>
#include <assert.h>

static void read_file(const char* filename, char* str, const int str_max_size);
int main(const int argc, const char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Error: No Source File\n");
    assert(argc == 2);
  }
  fprintf(stderr, "L:%03d DONE: argument check\n", __LINE__);

  FILE *fp;

  static char lex_str[100000];
  static char syntax_str[100000];
  static char pair_str[1000];
  static char source_str[100000];
  read_file("bnf/mod_c_lex.bnf"   , lex_str   , sizeof(lex_str)    / sizeof(char));
  // read_file("bnf/kandr.bnf"       , syntax_str, sizeof(syntax_str) / sizeof(char));
  read_file("bnf/mod_c_syntax.bnf", syntax_str, sizeof(syntax_str) / sizeof(char));
  read_file("bnf/mod_c_pair.txt"  , pair_str  , sizeof(pair_str)   / sizeof(char));
  //read_file("bnf/mod_c_source.txt", source_str, sizeof(source_str) / sizeof(char));
  read_file(argv[1]               , source_str, sizeof(source_str) / sizeof(char));
  fprintf(stderr, "L:%03d DONE: read files\n", __LINE__);

  BNF bnf[255]; // char:255文字のアルファベットに対応
  initialize_bnf(bnf, sizeof(bnf)/sizeof(BNF));

  static char           lex_name[50000];
  static char           lex_def[50000];
  static char           lex_simple[50000];
  static MIN_REGEX_NODE lex_node[50000];

  const int lex_size = create_lex(
    lex_str, bnf
    , lex_name   , sizeof(lex_name)   / sizeof(char)
    , lex_def    , sizeof(lex_def)    / sizeof(char)
    , lex_simple , sizeof(lex_simple) / sizeof(char)
    , lex_node   , sizeof(lex_node)   / sizeof(MIN_REGEX_NODE)
  );
  fprintf(stderr, "L:%03d DONE: read lex[%d] from bnf string\n", __LINE__, lex_size);

  static char           work[20000];
  static char           syntax_name[40000];
  static char           syntax_def[200000];
  static char           syntax_simple[200000];
  static MIN_REGEX_NODE syntax_node[20000];

  const int syntax_size = create_syntax(
    syntax_str, bnf
    , work          , sizeof(work)          / sizeof(char)
    , syntax_name   , sizeof(syntax_name)   / sizeof(char)
    , syntax_def    , sizeof(syntax_def)    / sizeof(char)
    , syntax_simple , sizeof(syntax_simple) / sizeof(char)
    , syntax_node   , sizeof(syntax_node)   / sizeof(MIN_REGEX_NODE)
  );
  fprintf(stderr, "L:%03d DONE: read syntax[%d] from bnf string\n", __LINE__, syntax_size);

  if ((fp = fopen("syntax.dot", "w")) == NULL) assert(0);
  syntax_to_dot(fp, bnf, "12.0", "0.2", "#FF0000", "#FF0000", "#000000");
  fclose(fp);
  fprintf(stderr, "L:%03d DONE: write syntax.dot\n", __LINE__);

  static PAIR_BNF pair_bnf[1000];
  const int pair_size = create_pair_bnf(
    pair_str, bnf
    , pair_bnf, sizeof(pair_bnf) / sizeof(PAIR_BNF)
  );
  fprintf(stderr, "L:%03d DONE: read token pair[](){} total:%d\n", __LINE__, pair_size);
  print_pair_bnf(stderr, pair_bnf, bnf);

  static LEX_TOKEN token[1000];
  const int token_size = match_lexer(token, sizeof(token)/sizeof(LEX_TOKEN), bnf, source_str);
  fprintf(stderr, "L:%03d DONE: lexical analysis. Token list size:%d\n", __LINE__, token_size);

  static BLOCK block[10000];
  create_block(block, sizeof(block)/sizeof(BLOCK), token);
  fprintf(stderr, "L:%03d DONE: create block list\n", __LINE__);

  replace_typedef(stderr, block, token, bnf);
  fprintf(stderr, "L:%03d DONE: replace typedef\n", __LINE__);

  if ((fp = fopen("token_list.txt", "w")) == NULL) assert(0);
  print_block(fp, block, token);
  print_token(fp, bnf, token, token_size);
  fclose(fp);
  fprintf(stderr, "L:%03d DONE: write token_list.txt\n", __LINE__);

  static PARSE_TREE pt[500000];
  static PARSE_MEMO memo[2000000]; // これがメモリを一番食う
  const int pt_size = parse_token_list(token, bnf, pair_bnf, pt, sizeof(pt)/sizeof(PARSE_TREE), memo, sizeof(memo)/sizeof(PARSE_MEMO));
  //print_parse_tree(stderr, pt_size, pt, bnf, token);
  fprintf(stderr, "L:%03d DONE: create parse tree. Total parse tree step:%d\n", __LINE__ , pt_size);

  if ((fp = fopen("parse_tree.dot", "w")) == NULL) assert(0);
  origin_parse_tree_to_dot(fp, 0, pt, bnf, token, "12.0", NULL, "#FF0000", "#000000");
  fclose(fp);
  fprintf(stderr, "L:%03d DONE: write parse_tree.dot\n", __LINE__);

  static SYMBOL symbol[10000];
  static int array[10000];
  initialize_symbol_table(symbol, sizeof(symbol)/sizeof(SYMBOL), array, sizeof(array)/sizeof(int));
  fprintf(stderr, "L:%03d DONE: initialize symbol table\n", __LINE__);

  static TYPE   type[10000];
  create_type_table(block, token, pt, bnf, type, sizeof(type)/sizeof(TYPE), symbol);
  fprintf(stderr, "L:%03d DONE: create type table\n", __LINE__);

  create_symbol_table(block, token, bnf, pt, symbol);
  fprintf(stderr, "L:%03d DONE: create symbol table\n", __LINE__);

//  register_type_and_symbol_size(block, token, bnf, pt, type, symbol);
//  fprintf(stderr, "L:%03d DONE: register type and symbol size\n", __LINE__);

  if ((fp = fopen("table.txt", "w")) == NULL) assert(0);
  fprintf(fp, "TYPE TABLE\n");
  print_type_table(fp, token, bnf, type);
  fprintf(fp, "\nSYMBOL TABLE\n");
  print_symbol_table_all(fp, token, bnf, pt, symbol);
  fclose(fp);
  fprintf(stderr, "L:%03d DONE: write table.txt\n", __LINE__);

//  translate_pt_to_ast(pt, bnf);
//  fprintf(stderr, "L:%03d DONE: create abstruct tree\n", __LINE__);
//
//  if ((fp = fopen("abstruct_tree.dot", "w")) == NULL) assert(0);
//  origin_parse_tree_to_dot(fp, 0, pt, bnf, token, "12.0", NULL, "#FF0000", "#000000");
//  fclose(fp);
//  fprintf(stderr, "L:%03d DONE: write abstruct_tree.dot\n", __LINE__);
//
//  static char code[100000];
//  generate_code(code, sizeof(code)/sizeof(char), block, token, bnf, pt, symbol);
//  fprintf(stderr, "L:%03d DONE: generate code\n", __LINE__);
//
//  if ((fp = fopen("code.asm", "w")) == NULL) assert(0);
//  fprintf(fp, code);
//  fclose(fp);
//  fprintf(stderr, "L:%03d DONE: write code.asm\n", __LINE__);
//
//  fprintf(stderr, "L:%03d DONE: all\n", __LINE__);
  return 0;
}

static void read_file(const char* filename, char* str, const int str_max_size) {/*{{{*/
  FILE *fp;
  if ((fp = fopen(filename, "r")) == NULL) {
    fprintf(stderr, "Error: Failed to open %s\n", filename);

  } else {
    int ch;
    int i=0;
    while ((ch = fgetc(fp)) != EOF) {
      str[i] = (char) ch;
      i++;

      if (i >= str_max_size) {
        fprintf(stderr, "Error: Too Large File:%s\n", filename);
        break;
      }
    }
  }
  fclose(fp);
}/*}}}*/
