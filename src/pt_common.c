#include "../include/common.h"
#include "../include/pt_common.h"
#include <string.h>
#include <assert.h>

extern bool is_pt_name(const char* name, const PARSE_TREE pt, const BNF* bnf) {/*{{{*/
  bool ret = false;
  if (0 == strcmp(name, bnf[pt.bnf_id].name)) ret = true;
  return ret;
}/*}}}*/
extern int rightside_pt_index(const int pt_index, const PARSE_TREE* pt) {/*{{{*/
  assert(pt_index >= 0);
  int ret = pt_index;
  while (pt[ret].right >= 0) ret = pt[ret].right;
  return ret;
}/*}}}*/
