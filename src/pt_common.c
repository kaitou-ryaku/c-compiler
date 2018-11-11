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
extern int search_pt_index_right(const char *name, const int pt_index, const PARSE_TREE* pt, const BNF* bnf) {/*{{{*/
  int current = pt_index;
  while (current >= 0) {
    if (is_pt_name(name, pt[current], bnf)) break;
    current = pt[current].right;
  }
  return current;
}/*}}}*/
extern int search_pt_index_left(const char *name, const int pt_index, const PARSE_TREE* pt, const BNF* bnf) {/*{{{*/
  int current = pt_index;
  while (current >= 0) {
    if (is_pt_name(name, pt[current], bnf)) break;
    current = pt[current].left;
  }
  return current;
}/*}}}*/
extern int search_pt_index_up(const char *name, const int pt_index, const PARSE_TREE* pt, const BNF* bnf) {/*{{{*/
  int current = pt_index;
  while (current >= 0) {
    if (is_pt_name(name, pt[current], bnf)) break;
    current = pt[current].up;
  }
  return current;
}/*}}}*/
extern bool delete_pt_recursive(const int index, PARSE_TREE* pt) {/*{{{*/
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
