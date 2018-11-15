#include "../min-bnf-parser/include/min-bnf-parser.h"
#include "../include/ast.h"
#include "../include/common.h"
#include "../include/pt_common.h"
#include <stdbool.h>
#include <string.h>
#include <assert.h>

// 関数プロトタイプ/*{{{*/
static bool delete_lift_solitary_pt(const int index, PARSE_TREE* pt);
static bool delete_solitary_container_recursive(const int top, PARSE_TREE* pt, const BNF* bnf);
static void delete_solitary_container(PARSE_TREE* pt, const BNF* bnf);
static bool delete_syntax_symbol_recursive(const int top, PARSE_TREE* pt, const BNF* bnf);
static void delete_syntax_symbol(PARSE_TREE* pt, const BNF* bnf);
static bool two_operatir_to_binary_tree_recursive(const int top, PARSE_TREE* pt, const BNF* bnf);
static void two_operatir_to_binary_tree(PARSE_TREE* pt, const BNF* bnf);
/*}}}*/

extern void translate_pt_to_ast(PARSE_TREE* pt, const BNF* bnf) {/*{{{*/
  delete_syntax_symbol(pt, bnf);
  delete_solitary_container(pt, bnf);
  two_operatir_to_binary_tree(pt, bnf);
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

  if ( is_pt_name("CONSTANT_EXPRESSION"       , pt[top], bnf)
    || is_pt_name("CONDITIONAL_EXPRESSION"    , pt[top], bnf)
    || is_pt_name("LOGICAL_OR_EXPRESSION"     , pt[top], bnf)
    || is_pt_name("LOGICAL_AND_EXPRESSION"    , pt[top], bnf)
    || is_pt_name("INCLUSIVE_OR_EXPRESSION"   , pt[top], bnf)
    || is_pt_name("EXCLUSIVE_OR_EXPRESSION"   , pt[top], bnf)
    || is_pt_name("AND_EXPRESSION"            , pt[top], bnf)
    || is_pt_name("EQUALITY_EXPRESSION"       , pt[top], bnf)
    || is_pt_name("RELATIONAL_EXPRESSION"     , pt[top], bnf)
    || is_pt_name("SHIFT_EXPRESSION"          , pt[top], bnf)
    || is_pt_name("ADDITIVE_EXPRESSION"       , pt[top], bnf)
    || is_pt_name("MULTIPLICATIVE_EXPRESSION" , pt[top], bnf)
    || is_pt_name("CAST_EXPRESSION"           , pt[top], bnf)
    || is_pt_name("UNARY_EXPRESSION"          , pt[top], bnf)
    || is_pt_name("POSTFIX_EXPRESSION"        , pt[top], bnf)

    || is_pt_name("PRIMARY_EXPRESSION"        , pt[top], bnf)
    || is_pt_name("EXPRESSION"                , pt[top], bnf)
    || is_pt_name("ASSIGNMENT_EXPRESSION"     , pt[top], bnf)

    || is_pt_name("STATEMENT"                 , pt[top], bnf)
    || is_pt_name("LABELED_STATEMENT"         , pt[top], bnf)
    || is_pt_name("EXPRESSION_STATEMENT"      , pt[top], bnf)
    || is_pt_name("SELECTION_STATEMENT"       , pt[top], bnf)
    || is_pt_name("ITERATION_STATEMENT"       , pt[top], bnf)
    || is_pt_name("JUMP_STATEMENT"            , pt[top], bnf)

    || is_pt_name("CONSTANT"                  , pt[top], bnf)
    || is_pt_name("ASSIGNMENT_OPERATOR"       , pt[top], bnf)
    || is_pt_name("UNARY_OPERATOR"            , pt[top], bnf)
    || is_pt_name("INITIALIZER"               , pt[top], bnf)
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
static bool delete_syntax_symbol_recursive(const int top, PARSE_TREE* pt, const BNF* bnf) {/*{{{*/
  bool is_del;

  if ( is_pt_name("lbrace"    , pt[top], bnf)
    || is_pt_name("lbracket"  , pt[top], bnf)
    || is_pt_name("lparen"    , pt[top], bnf)
    || is_pt_name("rbrace"    , pt[top], bnf)
    || is_pt_name("rbracket"  , pt[top], bnf)
    || is_pt_name("rparen"    , pt[top], bnf)
    || is_pt_name("semicolon" , pt[top], bnf)
    || is_pt_name("comma"     , pt[top], bnf)
  ) {
    is_del = delete_lift_solitary_pt(top, pt);
  } else {
    is_del = false;
  }

  if (!is_del && (pt[top].down  >= 0)) is_del = delete_syntax_symbol_recursive(pt[top].down, pt, bnf);
  if (!is_del && (pt[top].right >= 0)) is_del = delete_syntax_symbol_recursive(pt[top].right, pt, bnf);
  return is_del;
}/*}}}*/
static void delete_syntax_symbol(PARSE_TREE* pt, const BNF* bnf) {/*{{{*/
  bool is_del = true;
  while (is_del) is_del = delete_syntax_symbol_recursive(0, pt, bnf);
}/*}}}*/
static bool two_operatir_to_binary_tree_recursive(const int top, PARSE_TREE* pt, const BNF* bnf) {/*{{{*/
  bool is_change = false;
  if ( is_pt_name("LOGICAL_OR_EXPRESSION"      , pt[top], bnf)
    || is_pt_name("LOGICAL_AND_EXPRESSION"     , pt[top], bnf)
    || is_pt_name("INCLUSIVE_OR_EXPRESSION"    , pt[top], bnf)
    || is_pt_name("EXCLUSIVE_OR_EXPRESSION"    , pt[top], bnf)
    || is_pt_name("AND_EXPRESSION"             , pt[top], bnf)
    || is_pt_name("EQUALITY_EXPRESSION"        , pt[top], bnf)
    || is_pt_name("RELATIONAL_EXPRESSION"      , pt[top], bnf)
    || is_pt_name("SHIFT_EXPRESSION"           , pt[top], bnf)
    || is_pt_name("ADDITIVE_EXPRESSION"        , pt[top], bnf)
    || is_pt_name("MULTIPLICATIVE_EXPRESSION"  , pt[top], bnf)
    || is_pt_name("CAST_EXPRESSION"            , pt[top], bnf)
    || is_pt_name("ASSIGNMENT_EXPRESSION"      , pt[top], bnf)
  ) {
    if ( (pt[top].down >= 0)
      && (pt[pt[top].down].right >= 0)
      && (pt[pt[pt[top].down].right].right >= 0)
    ) {
      is_change = true;
      const int fst = pt[top].down;
      const int ope = pt[fst].right;
      const int snd = pt[ope].right;

      assert(is_lex(bnf[pt[ope].bnf_id]));

      pt[top].down = ope;
      pt[ope].up   = top;
      pt[ope].left  = pt[fst].left; assert(pt[ope].left < 0);
      pt[ope].right = pt[snd].right;
      pt[ope].down  = fst;

      pt[fst].left  = -1;
      pt[fst].right = snd;
      pt[fst].up    = ope;

      pt[snd].left  = fst;
      pt[snd].right = -1;
      pt[snd].up    = ope;

      two_operatir_to_binary_tree_recursive(fst, pt, bnf);
      two_operatir_to_binary_tree_recursive(snd, pt, bnf);
    }

    else if (pt[top].down >= 0) {
      if (pt[pt[top].down].right < 0) {
        assert(is_pt_name("CAST_EXPRESSION", pt[top], bnf));
      } else {
        is_change = delete_lift_solitary_pt(top, pt);
        two_operatir_to_binary_tree_recursive(pt[top].down, pt, bnf);
      }
    }

    else if (pt[top].down < 0) {
      is_change = delete_lift_solitary_pt(top, pt);
    } else {
      assert(0);
    }
  }

  if (!is_change && (pt[top].down  >= 0)) is_change = two_operatir_to_binary_tree_recursive(pt[top].down, pt, bnf);
  if (!is_change && (pt[top].right >= 0)) is_change = two_operatir_to_binary_tree_recursive(pt[top].right, pt, bnf);
  return is_change;
}/*}}}*/
static void two_operatir_to_binary_tree(PARSE_TREE* pt, const BNF* bnf) {/*{{{*/
  bool is_change = true;
  while (is_change) is_change = two_operatir_to_binary_tree_recursive(0, pt, bnf);
}/*}}}*/
