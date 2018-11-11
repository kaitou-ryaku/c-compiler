#include "../include/common.h"
#include "../include/type.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// 関数プロトタイプ/*{{{*/
static void initialize_type_table(TYPE* type, const int type_max_size);
static int register_default_type(const BNF* bnf, TYPE* type);
static int search_lex_bnf(const BNF* bnf, const char* name);
static void print_type_table(FILE* fp, const LEX_TOKEN* token, const BNF* bnf, const TYPE* type);
/*}}}*/
extern int create_type_table(/*{{{*/
  const LEX_TOKEN* token
  , PARSE_TREE* pt
  , const BNF* bnf
  , TYPE* type
  , const int type_max_size
) {
  initialize_type_table(type, type_max_size);
  const int default_type_size = register_default_type(bnf, type);
  print_type_table(stderr, token, bnf, type);
  return 3;
}/*}}}*/
static void initialize_type_table(TYPE* type, const int type_max_size) {/*{{{*/
  for (int i=0; i<type_max_size; i++) {
    type[i].id         = i;
    type[i].total_size = type_max_size;
    type[i].used_size  = 0;
    type[i].bnf_id     = -1;
    type[i].token_id   = -1;
    type[i].block      = -1;
    type[i].byte       = -1;
  }
}/*}}}*/
static int register_default_type(const BNF* bnf, TYPE* type) {/*{{{*/
  int i=0;

  type[i].bnf_id = search_lex_bnf(bnf, "char");
  type[i].byte  = 1;
  i++;

  type[i].bnf_id = search_lex_bnf(bnf, "double");
  type[i].byte  = 8;
  i++;

  type[i].bnf_id = search_lex_bnf(bnf, "float");
  type[i].byte  = 4;
  i++;

  type[i].bnf_id = search_lex_bnf(bnf, "int");
  type[i].byte  = 4;
  i++;

  type[i].bnf_id = search_lex_bnf(bnf, "long");
  type[i].byte  = 8;
  i++;

  type[i].bnf_id = search_lex_bnf(bnf, "short");
  type[i].byte  = 2;
  i++;

  type[i].bnf_id = search_lex_bnf(bnf, "signed");
  type[i].byte  = 4;
  i++;

  type[i].bnf_id = search_lex_bnf(bnf, "unsigned");
  type[i].byte  = 4;
  i++;

  for (int j=0; j<i; j++) type[j].block = 0;

  return i;
}/*}}}*/
static int search_lex_bnf(const BNF* bnf, const char* name) {/*{{{*/
  int bnf_id;
  for (bnf_id=0; bnf_id<bnf[0].total_size; bnf_id++) {
    if (0==strcmp(bnf[bnf_id].name, name)) break;
  }
  if (bnf_id == bnf[0].total_size) bnf_id = -1;
  return bnf_id;
}/*}}}*/
static void print_type_table(FILE* fp, const LEX_TOKEN* token, const BNF* bnf, const TYPE* type) {/*{{{*/
  int i=0;
  while (type[i].bnf_id >= 0) {
    fprintf(fp, "%03d ", i);
    fprintf(fp, "%s", bnf[type[i].bnf_id].name);
    fprintf(fp, "\n");
    i++;
  }
}/*}}}*/
