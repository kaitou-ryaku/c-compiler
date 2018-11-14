#include "../min-bnf-parser/include/min-bnf-parser.h"
#include "../include/ast.h"
#include "../include/common.h"
#include "../include/symbol.h"
#include "../include/typedef.h"
#include "../include/type.h"
#include "../include/memory.h"
#include <stdio.h>

static void read_file(const char* filename, char* str, const int str_max_size);
int main(void) {

  static char lex_str[100000];
  static char syntax_str[100000];
  static char pair_str[1000];
  static char source_str[100000];
  read_file("bnf/mod_c_lex.bnf"   , lex_str   , sizeof(lex_str)    / sizeof(char));
  // read_file("bnf/kandr.bnf"       , syntax_str, sizeof(syntax_str) / sizeof(char));
  read_file("bnf/mod_c_syntax.bnf", syntax_str, sizeof(syntax_str) / sizeof(char));
  read_file("bnf/mod_c_pair.txt"  , pair_str  , sizeof(pair_str)   / sizeof(char));
  read_file("bnf/mod_c_source.txt", source_str, sizeof(source_str) / sizeof(char));

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

  const int bnf_size = lex_size + syntax_size;
  fprintf(stderr, "total bnfsize : %d\n", bnf_size);

  {
    FILE *fp;
    char *filename = "syntax.dot";
    if ((fp = fopen(filename, "w")) == NULL) {
      fprintf(stderr, "Error: Failed to open %s\n", filename);
    }
    syntax_to_dot(fp, bnf, "12.0", "0.2", "#FF0000", "#FF0000", "#000000");
    fclose(fp);
  }

  static PAIR_BNF pair_bnf[1000];

  const int pair_size = create_pair_bnf(
    pair_str, bnf
    , pair_bnf, sizeof(pair_bnf) / sizeof(PAIR_BNF)
  );

  fprintf(stderr, "pair size : %d\n", pair_size);
  print_pair_bnf(stderr, pair_bnf, bnf);

  static LEX_TOKEN token[1000];
  const int token_size = match_lexer(token, sizeof(token)/sizeof(LEX_TOKEN), bnf, source_str);
  fprintf(stderr, "TOTAL TOKEN SIZE:%d\n", token_size);
  // print_token(stderr, bnf, token, token_size);

  static BLOCK block[10000];
  create_block(block, sizeof(block)/sizeof(BLOCK), token);
  print_block(stderr, block, token);

  replace_typedef(stderr, block, token, bnf);
  print_token(stderr, bnf, token, token_size);

  static PARSE_TREE pt[500000];
  static PARSE_MEMO memo[2000000]; // これがメモリを一番食う
  const int pt_size = parse_token_list(token, bnf, pair_bnf, pt, sizeof(pt)/sizeof(PARSE_TREE), memo, sizeof(memo)/sizeof(PARSE_MEMO));

  fprintf(stderr, "TOTAL PARSE TREE STEP:%d\n", pt_size);
  //print_parse_tree(stderr, pt_size, pt, bnf, token);

  static SYMBOL symbol[10000];
  static int array[10000];
  initialize_symbol_table(symbol, sizeof(symbol)/sizeof(SYMBOL), array, sizeof(array)/sizeof(int));
  static TYPE   type[10000];
  create_type_table(block, token, pt, bnf, type, sizeof(type)/sizeof(TYPE), symbol);
  create_symbol_table(block, token, bnf, pt, symbol);

  translate_pt_to_ast(pt, bnf);
  //print_parse_tree(stderr, pt_size, pt, bnf, token);

  {
    FILE *fp;
    char *filename = "parse_tree.dot";
    if ((fp = fopen(filename, "w")) == NULL) {
      fprintf(stderr, "Error: Failed to open %s\n", filename);
    }

    origin_parse_tree_to_dot(fp, 0, pt, bnf, token, "12.0", NULL, "#FF0000", "#000000");

    fclose(fp);
  }

  register_type_and_symbol_size(block, token, bnf, pt, type, symbol);
  print_type_table(stderr, token, bnf, type);
  print_symbol_table_all(token, bnf, pt, symbol);

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
