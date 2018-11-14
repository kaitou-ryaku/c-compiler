#include "../min-bnf-parser/include/text.h"
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
extern bool is_same_token_str(const int a, const int b, const LEX_TOKEN* token) {/*{{{*/
  assert(a < token[0].used_size);
  assert(b < token[0].used_size);
  bool ret;
  if (is_same_word(token[a].src, token[a].begin, token[a].end, token[b].src, token[b].begin, token[b].end)) {
    ret = true;
  } else {
    ret = false;
  }
  return ret;
}/*}}}*/
extern bool inside_scope(const int token_child_index, const int token_parent_index, const BLOCK* block) {/*{{{*/
  bool ret=false;

  int seek_index = token_child_index;
  while (seek_index >= 0) {
    // parentの宣言ブロックにいる場合
    if (block[token_parent_index].here == block[seek_index].here) {
      ret=true;
      break;
    }

    // parentの宣言ブロッキにいない場合、一つ上のブロックに移動
    else {
      int up_seek_index = seek_index;
      while (up_seek_index >= 0) {
        if (block[up_seek_index].here == block[seek_index].up) {
          seek_index = up_seek_index;
          break;
        }
        up_seek_index--;
      }
      if (up_seek_index < 0) break;
    }
  }

  return ret;
}/*}}}*/
