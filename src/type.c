#include "../include/common.h"
#include "../include/type.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

const int TYPE_UNUSED_STATE = -1;
const int TYPE_BASIC_STATE = 1;
const int TYPE_STRUCT_STATE = 2;
const int TYPE_MEMBER_STATE = 3;

static void initialize_type_table(TYPE* type, const int type_max_size);
static int register_default_type(const BNF* bnf, TYPE* type);
static int search_lex_bnf(const BNF* bnf, const char* name);
static void print_type_table(FILE* fp, const LEX_TOKEN* token, const BNF* bnf, const TYPE* type);

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
    type[i].state      = TYPE_UNUSED_STATE;
    type[i].kind       = -1;
    type[i].byte       = -1;
  }
}/*}}}*/
static int register_default_type(const BNF* bnf, TYPE* type) {/*{{{*/
  int i=0;

  type[i].state = TYPE_BASIC_STATE;
  type[i].kind  = search_lex_bnf(bnf, "char");
  type[i].byte  = 1;
  i++;

  type[i].state = TYPE_BASIC_STATE;
  type[i].kind  = search_lex_bnf(bnf, "double");
  type[i].byte  = 8;
  i++;

  type[i].state = TYPE_BASIC_STATE;
  type[i].kind  = search_lex_bnf(bnf, "float");
  type[i].byte  = 4;
  i++;

  type[i].state = TYPE_BASIC_STATE;
  type[i].kind  = search_lex_bnf(bnf, "int");
  type[i].byte  = 4;
  i++;

  type[i].state = TYPE_BASIC_STATE;
  type[i].kind  = search_lex_bnf(bnf, "long");
  type[i].byte  = 8;
  i++;

  type[i].state = TYPE_BASIC_STATE;
  type[i].kind  = search_lex_bnf(bnf, "short");
  type[i].byte  = 2;
  i++;

  type[i].state = TYPE_BASIC_STATE;
  type[i].kind  = search_lex_bnf(bnf, "signed");
  type[i].byte  = 4;
  i++;

  type[i].state = TYPE_BASIC_STATE;
  type[i].kind  = search_lex_bnf(bnf, "unsigned");
  type[i].byte  = 4;
  i++;

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
  while (type[i].state != TYPE_UNUSED_STATE) {
    fprintf(fp, "%03d ", i);
    if (type[i].state == TYPE_BASIC_STATE) {
      fprintf(fp, "BASIC  ");
      fprintf(fp, "%s", bnf[type[i].kind].name);
    } else if (type[i].state == TYPE_STRUCT_STATE) {
      fprintf(fp, "STRUCT ");
    } else if (type[i].state == TYPE_MEMBER_STATE) {
      fprintf(fp, "MEMBER ");
    }
    fprintf(fp, "\n");
    i++;
  }
}/*}}}*/
