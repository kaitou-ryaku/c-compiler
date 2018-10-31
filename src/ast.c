#include "../min-bnf-parser/include/min-bnf-parser.h"
#include <stdbool.h>
#include <string.h>

// 関数プロトタイプ/*{{{*/
static bool is_pt_name(const char* name, const PARSE_TREE pt, const BNF* bnf);
static bool delete_lift_solitary_pt(const int index, PARSE_TREE* pt);
static bool delete_solitary_container_recursive(const int top, PARSE_TREE* pt, const BNF* bnf);
static void delete_solitary_container(PARSE_TREE* pt, const BNF* bnf);
static void delete_syntax_symbol(PARSE_TREE* pt, const BNF* bnf);
static void delete_syntax_symbol(PARSE_TREE* pt, const BNF* bnf);
/*}}}*/

extern void translate_pt_to_ast(PARSE_TREE* pt, const BNF* bnf) {/*{{{*/
  delete_syntax_symbol(pt, bnf);
  delete_solitary_container(pt, bnf);
}/*}}}*/
static bool is_pt_name(const char* name, const PARSE_TREE pt, const BNF* bnf) {/*{{{*/
  bool ret = false;
  if (0 == strcmp(name, bnf[pt.bnf_id].name)) ret = true;
  return ret;
}/*}}}*/
static bool delete_lift_solitary_pt(const int index, PARSE_TREE* pt) {/*{{{*/
  // 削除したらtrue, 削除しなかったらfalseを返す
  bool ret;
  const PARSE_TREE del = pt[index];

  const bool alone     = (del.left <  0 && del.right <  0); // 削除対象の横に誰もいない
  const bool leftside  = (del.left <  0 && del.right >= 0); // 削除対象が左端
  const bool rightside = (del.left >= 0 && del.right <  0); // 削除対象が右端
  const bool center    = (del.left >= 0 && del.right >= 0); // 削除対象の両隣にノードが存在

  // 頂上のノードの場合は何もしない/*{{{*/
  if (del.up < 0) {
    ret = false;
  }/*}}}*/
  // 削除対象が孤独ノードで、子無しの場合、単に削除/*{{{*/
  else if (del.down < 0) {
    ret = true;

    if (alone) {
      pt[del.up].down = -1;
    }

    if (leftside) {
      pt[del.up].down = del.right;
      pt[del.right].left = -1;
    }

    if (rightside) {
      pt[del.left].right = -1;
    }

    if (center) {
      pt[del.left].right = del.right;
      pt[del.right].left = del.left;
    }
  }/*}}}*/
  // 削除対象が孤独ノードで、一人っ子有りの場合、削除対象を子で置換/*{{{*/
  else if (pt[del.down].left < 0 && pt[del.down].right < 0) {
    ret = true;

    if (alone) {
      pt[del.up].down = del.down;
      pt[del.down].up = del.up;
    }

    if (leftside) {
      pt[del.up].down = del.down;
      pt[del.down].up = del.up;

      pt[del.right].left = del.down;
      pt[del.down].right = del.right;
    }

    if (rightside) {
      pt[del.down].up = del.up;

      pt[del.left].right = del.down;
      pt[del.down].left  = del.left;
    }

    if (center) {
      pt[del.down].up = del.up;

      pt[del.right].left = del.down;
      pt[del.down].right = del.right;

      pt[del.left].right = del.down;
      pt[del.down].left  = del.left;
    }
  }/*}}}*/
  // 削除対象が孤独ノードではなかった場合、何もしない/*{{{*/
  else {
    ret = false;
  }/*}}}*/

  return ret;
}/*}}}*/
static bool delete_solitary_container_recursive(const int top, PARSE_TREE* pt, const BNF* bnf) {/*{{{*/
  bool is_del;

  if ( is_pt_name("CONSTANT_EXPRESSION", pt[top], bnf)
    || is_pt_name("CONDITIONAL_EXPRESSION", pt[top], bnf)
    || is_pt_name("LOGICAL_OR_EXPRESSION", pt[top], bnf)
    || is_pt_name("LOGICAL_AND_EXPRESSION", pt[top], bnf)
    || is_pt_name("INCLUSIVE_OR_EXPRESSION", pt[top], bnf)
    || is_pt_name("EXCLUSIVE_OR_EXPRESSION", pt[top], bnf)
    || is_pt_name("AND_EXPRESSION", pt[top], bnf)
    || is_pt_name("EQUALITY_EXPRESSION", pt[top], bnf)
    || is_pt_name("RELATIONAL_EXPRESSION", pt[top], bnf)
    || is_pt_name("SHIFT_EXPRESSION", pt[top], bnf)
    || is_pt_name("ADDITIVE_EXPRESSION", pt[top], bnf)
    || is_pt_name("MULTIPLICATIVE_EXPRESSION", pt[top], bnf)
    || is_pt_name("CAST_EXPRESSION", pt[top], bnf)
    || is_pt_name("UNARY_EXPRESSION", pt[top], bnf)
    || is_pt_name("POSTFIX_EXPRESSION", pt[top], bnf)
    || is_pt_name("ARGUMENT_EXPRESSION_LIST", pt[top], bnf)
    || is_pt_name("PRIMARY_EXPRESSION", pt[top], bnf)
    || is_pt_name("EXPRESSION", pt[top], bnf)
    || is_pt_name("ASSIGNMENT_EXPRESSION", pt[top], bnf)
    || is_pt_name("COMPOUND_STATEMENT", pt[top], bnf)
    || is_pt_name("STATEMENT", pt[top], bnf)
    || is_pt_name("LABELED_STATEMENT", pt[top], bnf)
    || is_pt_name("EXPRESSION_STATEMENT", pt[top], bnf)
    || is_pt_name("SELECTION_STATEMENT", pt[top], bnf)
    || is_pt_name("ITERATION_STATEMENT", pt[top], bnf)
    || is_pt_name("JUMP_STATEMENT", pt[top], bnf)
  ) {
    is_del = delete_lift_solitary_pt(top, pt);
  } else {
    is_del = false;
  }

  if (!is_del && (pt[top].down  >= 0)) is_del = delete_solitary_container_recursive(pt[top].down, pt, bnf);
  if (!is_del && (pt[top].right >= 0)) is_del = delete_solitary_container_recursive(pt[top].right, pt, bnf);
  return is_del;
}/*}}}*/
static void delete_solitary_container(PARSE_TREE* pt, const BNF* bnf) {/*{{{*/
  bool is_del = true;
  while (is_del) is_del = delete_solitary_container_recursive(0, pt, bnf);
}/*}}}*/
static void delete_syntax_symbol(PARSE_TREE* pt, const BNF* bnf) {/*{{{*/
  fprintf(stderr, "hoge%d\n", pt[0].used_size);
  for (int i=0; i<pt[0].used_size; i++) {
    if ( is_pt_name("lbrace"    , pt[i], bnf)
      || is_pt_name("lbracket"  , pt[i], bnf)
      || is_pt_name("lparen"    , pt[i], bnf)
      || is_pt_name("rbrace"    , pt[i], bnf)
      || is_pt_name("rbracket"  , pt[i], bnf)
      || is_pt_name("rparen"    , pt[i], bnf)
      || is_pt_name("semicolon" , pt[i], bnf)
      || is_pt_name("comma"     , pt[i], bnf)
    ) delete_lift_solitary_pt(i, pt);
  }
}/*}}}*/
