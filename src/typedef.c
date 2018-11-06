#include "../min-bnf-parser/include/min-bnf-parser.h"
#include "../min-bnf-parser/include/text.h"
#include "../include/common.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

//関数プロトタイプ/*{{{*/
int replace_typedef_declare(FILE* fp, LEX_TOKEN* token, const BNF* bnf);
void replace_typedef_use(const BLOCK* block, LEX_TOKEN* token, const BNF* bnf);
/*}}}*/
void replace_typedef(FILE* fp, const BLOCK* block, LEX_TOKEN* token, const BNF* bnf) {/*{{{*/
  const int typedef_count = replace_typedef_declare(fp, token, bnf);
  fprintf(stderr, "Typedef is total %d\n", typedef_count);
  replace_typedef_use(block, token, bnf);
}/*}}}*/
int replace_typedef_declare(FILE* fp, LEX_TOKEN* token, const BNF* bnf) {/*{{{*/
  // BNFのLEXのtypedef_nameに対応するindexを取得
  int index_typedef_name = 0;
  while (index_typedef_name >= 0) {
    if (strcmp(bnf[index_typedef_name].name, "typedef_name") == 0) break;
    index_typedef_name = search_bnf_next_lex(index_typedef_name, bnf);
  }
  assert(index_typedef_name >= 0);

  int ret = 0;

  bool semicolon_wait = false;
  bool rbracket_wait = false;
  int bracket_count = 0;
  for (int i=0; i<token[0].used_size; i++) {

    // typedefを発見 -> semicolon_wait をgrueにする
    if (is_token_kind("typedef", token[i], bnf)) {
      assert(!semicolon_wait);
      semicolon_wait = true;
      fprintf(fp, "TYPEDEF ");
    }

    // typedefの後の非構造体に{を発見 -> 構造体に入る。対応する}が現れるまで rbracket_wait をtrueにする
    else if (semicolon_wait == true && rbracket_wait == false && is_token_kind("lbracket", token[i], bnf)) {
      rbracket_wait = true;
      assert(bracket_count == 0);
      bracket_count++;
      fprintf(fp, "STRUCT ");
    }

    // typedefの後の構造体中に{が現れた -> 括弧のカウントを増やす
    else if (semicolon_wait == true && rbracket_wait == true && is_token_kind("lbracket", token[i], bnf)) {
      bracket_count++;
    }

    // typedefの後の構造体中に}が現れた -> 括弧のカウントを減らす。もし括弧数が0になれば、構造体部分の終わり。
    else if (semicolon_wait == true && rbracket_wait == true && is_token_kind("rbracket", token[i], bnf)) {
      bracket_count--;
      if (bracket_count == 0) rbracket_wait = false;
    }

    // typedefの後の非構造体で;が現れた -> その前のトークンが型名エイリアス
    else if (semicolon_wait == true && rbracket_wait == false && is_token_kind("semicolon", token[i], bnf)) {
      token[i-1].kind = index_typedef_name;
      semicolon_wait = false;

      print_token_name(fp, token[i-1]);
      fprintf(fp, "\n");
      ret++;
    }
  }

  return ret;
}/*}}}*/
void replace_typedef_use(const BLOCK* block, LEX_TOKEN* token, const BNF* bnf) {/*{{{*/
  // BNFのLEXのtypedef_nameに対応するindexを取得
  int index_typedef_name = 0;
  while (index_typedef_name >= 0) {
    if (strcmp(bnf[index_typedef_name].name, "typedef_name") == 0) break;
    index_typedef_name = search_bnf_next_lex(index_typedef_name, bnf);
  }
  assert(index_typedef_name >= 0);

  for (int def=0; def<token[0].used_size; def++) {
    if (!is_token_kind("typedef_name", token[def], bnf)) continue;

    for (int use=def; use<token[0].used_size; use++) {
      if (!is_token_kind("identifier", token[use], bnf)) continue;
      if (!is_same_word(token[def].src, token[def].begin, token[def].end, token[use].src, token[use].begin, token[use].end)) continue;

      const int def_scope = block[def].here;
      int seek_index = use;
      while (seek_index >= 0) {
        // typedefの宣言ブロックにいる場合、置換
        if (def_scope == block[seek_index].here) {
          token[use].kind = index_typedef_name;
          break;
        }

        // typedefの宣言ブロックにいない場合、一つ上のブロックに移動
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
    }
  }
}/*}}}*/
