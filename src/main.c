#include "../min-bnf-parser/include/min-bnf-parser.h"
#include "../include/parse_tree.h"
#include "../include/ast.h"
#include <stdio.h>

int main(void) {

  const char* lex_str = ""
    #include "../bnf/mod_c_lex.bnf"
  ;
  const char* syntax_str = ""
    #include "../bnf/mod_c_syntax.bnf"
  ;
  const char* pair_str = ""
    #include "../bnf/mod_c_pair.txt"
  ;
  const char* src_str = ""
    #include "../bnf/mod_c_source.txt"
  ;

  BNF bnf[255]; // char:255文字のアルファベットに対応
  initialize_bnf(bnf, sizeof(bnf)/sizeof(BNF));

  char           lex_name[2000];
  char           lex_def[2000];
  char           lex_simple[2000];
  MIN_REGEX_NODE lex_node[2000];

  const int lex_size = create_lex(
    lex_str, bnf
    , lex_name   , sizeof(lex_name)   / sizeof(char)
    , lex_def    , sizeof(lex_def)    / sizeof(char)
    , lex_simple , sizeof(lex_simple) / sizeof(char)
    , lex_node   , sizeof(lex_node)   / sizeof(MIN_REGEX_NODE)
  );

  char           work[2000];
  char           syntax_name[4000];
  char           syntax_def[20000];
  char           syntax_simple[20000];
  MIN_REGEX_NODE syntax_node[2000];

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
      fprintf(stderr, "Failed to open %s.\n", filename);
    }
    syntax_to_dot(fp, bnf, "12.0", "0.2", "#FF0000", "#FF0000", "#000000");
    fclose(fp);
  }

  PAIR_BNF pair_bnf[100];

  const int pair_size = create_pair_bnf(
    pair_str, bnf
    , pair_bnf, sizeof(pair_bnf) / sizeof(PAIR_BNF)
  );

  fprintf(stderr, "pair size : %d\n", pair_size);
  print_pair_bnf(stderr, pair_bnf, bnf);

  LEX_TOKEN token[1000];
  const int token_size = match_lexer(token, sizeof(token)/sizeof(LEX_TOKEN), bnf, src_str);
  print_token(stderr, bnf, token, token_size);

  PARSE_TREE pt[5000];
  static bool memo[255*2000*2000];
  const int pt_size = parse_token_list(token, bnf, pair_bnf, pt, sizeof(pt)/sizeof(PARSE_TREE), memo, sizeof(memo)/sizeof(bool));

  fprintf(stderr, "TOTAL PARSE TREE STEP:%d\n", pt_size);
  // print_parse_tree(stderr, pt_size, pt, bnf, token);

  translate_pt_to_ast(pt, bnf);

  {
    FILE *fp;
    char *filename = "parse_tree.dot";
    if ((fp = fopen(filename, "w")) == NULL) {
      fprintf(stderr, "Failed to open %s.\n", filename);
    }

    origin_parse_tree_to_dot(fp, 0, pt, bnf, token, "12.0", NULL, "#FF0000", "#000000");

    fclose(fp);
  }

  return 0;
}
