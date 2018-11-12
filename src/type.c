#include "../min-bnf-parser/include/text.h"
#include "../include/common.h"
#include "../include/type.h"
#include "../include/symbol.h"
#include "../include/pt_common.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// 関数プロトタイプ/*{{{*/
static void initialize_type_table(TYPE* type, const int type_max_size);
static int register_default_type(const BNF* bnf, TYPE* type);
static int search_lex_bnf(const BNF* bnf, const char* name);
static void print_type_table(FILE* fp, const LEX_TOKEN* token, const BNF* bnf, const TYPE* type);
static void register_struct_recursive(
  const int pt_top_index
  , const BLOCK* block
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , TYPE* type
  , SYMBOL* member
);
static void register_specifier_qualifier_list(
  const   int member_empty_id
  , const int specifier_qualifier_list
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* member
);
static int search_unused_type_index(const TYPE* type);
static void register_typedef_recursive(
  const int pt_top_index
  , const BLOCK* block
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , TYPE* type
);
/*}}}*/
extern void create_type_table(/*{{{*/
  const BLOCK* block
  , const LEX_TOKEN* token
  , PARSE_TREE* pt
  , const BNF* bnf
  , TYPE* type
  , const int type_max_size
  , SYMBOL* member
) {
  // memberテーブルは初期化済みとする
  initialize_type_table(type, type_max_size);
  register_default_type(bnf, type);
  register_struct_recursive(0, block, token, bnf, pt, type, member);
  register_typedef_recursive(0, block, token, bnf, pt, type);
  delete_empty_external_declaration(bnf, pt);

  fprintf(stderr, "\nTYPE TABLE\n");
  print_type_table(stderr, token, bnf, type);
}/*}}}*/
static void initialize_type_table(TYPE* type, const int type_max_size) {/*{{{*/
  for (int i=0; i<type_max_size; i++) {
    type[i].id         = i;
    type[i].total_size = type_max_size;
    type[i].used_size  = 0;
    type[i].bnf_id     = -1;
    type[i].token_id   = -1;
    type[i].alias_id   = -1;
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
    fprintf(fp, "%03d", i);
    fprintf(fp, " | %-15s", bnf[type[i].bnf_id].name);
    fprintf(fp, " | block:%2d", type[i].block);

    if (0==strcmp("typedef", bnf[type[i].bnf_id].name)) {
      assert(type[i].token_id >= 0);
      assert(type[i].alias_id >= 0);

      fprintf(fp, " | ");

      print_token_name(fp, token[type[i].token_id]);
      fprintf(fp, " --> ");
      fprintf(fp, "%03d", type[i].alias_id);
    }

    fprintf(fp, "\n");

    i++;
  }
}/*}}}*/
static void register_struct_recursive(/*{{{*/
  const int pt_top_index
  , const BLOCK* block
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , TYPE* type
  , SYMBOL* member
) {

  int member_empty_id = search_unused_symbol_index(member);

  // 変数をシンボルテーブルに登録
  if (pt_top_index < 0) {
    ;
  }
  else if (is_pt_name("STRUCT_OR_UNION_SPECIFIER", pt[pt_top_index], bnf)) {
    const int struct_or_union = pt[pt_top_index].down;
    assert(struct_or_union >= 0);

    // 構造体名をtypeテーブルに追加
    const int struct_id = pt[struct_or_union].down; // const int struct = は予約語なので無理
    assert(struct_id >= 0);
    assert(is_pt_name("struct", pt[struct_id], bnf));
    const int type_empty_id = search_unused_type_index(type);
    type[type_empty_id].bnf_id = pt[struct_id].bnf_id;
    type[type_empty_id].token_id = pt[struct_id].token_begin_index;
    type[type_empty_id].block = block[pt[struct_id].token_begin_index].here;

    const int struct_declaration_list = search_pt_index_right("STRUCT_DECLARATION_LIST", struct_or_union, pt, bnf);
    assert(struct_declaration_list >= 0);

    int struct_declaration = pt[struct_declaration_list].down;
    assert(struct_declaration >= 0);

    // 構造体メンバをmemberテーブルに追加
    int count=0;
    while (struct_declaration >= 0) {
      assert(struct_declaration >= 0);
      assert(is_pt_name("STRUCT_DECLARATION", pt[struct_declaration], bnf));

      member[member_empty_id].kind = SYMBOL_TABLE_STRUCT_MEMBER;
      member[member_empty_id].function_id = type_empty_id;
      member[member_empty_id].argument_id = count;
      count++;

      const int specifier_qualifier_list = pt[struct_declaration].down;
      assert(specifier_qualifier_list >= 0);
      assert(is_pt_name("SPECIFIER_QUALIFIER_LIST", pt[specifier_qualifier_list], bnf));
      register_specifier_qualifier_list(member_empty_id, specifier_qualifier_list, bnf, pt, member);

      const int struct_declarator_list = pt[specifier_qualifier_list].right;
      assert(struct_declarator_list >= 0);
      assert(is_pt_name("STRUCT_DECLARATOR_LIST", pt[struct_declarator_list], bnf));

      const int struct_declarator = pt[struct_declarator_list].down;
      assert(struct_declarator >= 0);
      assert(is_pt_name("STRUCT_DECLARATOR", pt[struct_declarator], bnf));

      const int declarator = pt[struct_declarator].down;
      assert(declarator >= 0);
      assert(is_pt_name("DECLARATOR", pt[declarator], bnf));

      const int array_index = get_new_array_index(member[0].array, member[0].total_array_size);
      member[member_empty_id].array = &(member[0].array[array_index]);
      register_declarator(member_empty_id, declarator, token, bnf, pt, member);
      const int registered_array_index = get_new_array_index(member[0].array, member[0].total_array_size);
      member[member_empty_id].array_size = registered_array_index - array_index;

      member_empty_id++;
      struct_declaration = pt[struct_declaration].right;
    }

    // 構文木から構造体を削除
    assert(pt[pt_top_index].left < 0);
    assert(pt[pt_top_index].right < 0);
    const int up = pt[pt_top_index].up;
    pt[up].down = struct_id;
    pt[struct_id].up = up;
    pt[struct_id].left = -1;
    pt[struct_id].right = -1;
    assert(pt[struct_id].down < 0);
  }

  else {
    const int right = pt[pt_top_index].right;
    if (right >= 0) register_struct_recursive(right, block, token, bnf, pt, type, member);
    const int down  = pt[pt_top_index].down;
    if (down >= 0)  register_struct_recursive(down, block, token, bnf, pt, type, member);
  }
}/*}}}*/
static void register_specifier_qualifier_list(/*{{{*/
  const   int member_empty_id
  , const int specifier_qualifier_list
  , const BNF* bnf
  , PARSE_TREE* pt
  , SYMBOL* member
) {
  assert(specifier_qualifier_list >= 0);
  assert(is_pt_name("SPECIFIER_QUALIFIER_LIST", pt[specifier_qualifier_list], bnf));

  int specifier = pt[specifier_qualifier_list].down;
  assert(specifier >= 0);

  while (specifier >= 0) {
    const int keyword = pt[specifier].down;
    assert(keyword >= 0);

    if (is_pt_name("TYPE_SPECIFIER", pt[specifier], bnf)) {
      assert(member[member_empty_id].type < 0);
      member[member_empty_id].type = keyword;
    }
    if (is_pt_name("TYPE_QUALIFIER", pt[specifier], bnf)) {
      assert(member[member_empty_id].qualify < 0);
      member[member_empty_id].qualify = keyword;
    }

    specifier = pt[specifier].right;
  }
}/*}}}*/
static int search_unused_type_index(const TYPE* type) {/*{{{*/
  int ret;
  for (ret=0; ret<type[0].total_size; ret++) {
    if (type[ret].bnf_id == -1) break;
  }
  return ret;
}/*}}}*/
static void register_typedef_recursive(/*{{{*/
  const int pt_top_index
  , const BLOCK* block
  , const LEX_TOKEN* token
  , const BNF* bnf
  , PARSE_TREE* pt
  , TYPE* type
) {

  // 変数をシンボルテーブルに登録
  if (pt_top_index < 0) {
    ;
  }
  else if (is_pt_name("typedef", pt[pt_top_index], bnf)) {

    const int storage_class_specifier = pt[pt_top_index].up;
    assert(storage_class_specifier);
    assert(is_pt_name("STORAGE_CLASS_SPECIFIER", pt[storage_class_specifier], bnf));

    const int type_specifier = search_pt_index_right("TYPE_SPECIFIER", storage_class_specifier, pt, bnf);
    assert(type_specifier >= 0);

    const int type_lex_token = pt[type_specifier].down;
    assert(type_lex_token >= 0);

    const int declaration_specifiers = pt[storage_class_specifier].up;
    assert(declaration_specifiers);
    assert(is_pt_name("DECLARATION_SPECIFIERS", pt[declaration_specifiers], bnf));

    const int declaration = pt[declaration_specifiers].up;
    assert(declaration);
    assert(is_pt_name("DECLARATION", pt[declaration], bnf));

    const int init_declarator_list = search_pt_index_right("INIT_DECLARATOR_LIST", declaration_specifiers, pt, bnf);
    assert(init_declarator_list);
    assert(is_pt_name("INIT_DECLARATOR_LIST", pt[init_declarator_list], bnf));

    const int init_declarator = pt[init_declarator_list].down;
    assert(init_declarator);
    assert(is_pt_name("INIT_DECLARATOR", pt[init_declarator], bnf));

    const int declarator = pt[init_declarator].down;
    assert(declarator);
    assert(is_pt_name("DECLARATOR", pt[declarator], bnf));

    const int direct_declarator = pt[declarator].down;
    assert(direct_declarator);
    assert(is_pt_name("DIRECT_DECLARATOR", pt[direct_declarator], bnf));

    const int identifier = pt[direct_declarator].down;
    assert(identifier);
    assert(is_pt_name("identifier", pt[identifier], bnf));

    // 登録
    const int type_empty_id = search_unused_type_index(type);
    type[type_empty_id].bnf_id = search_lex_bnf(bnf, "typedef");
    type[type_empty_id].token_id = pt[identifier].token_begin_index;
    type[type_empty_id].block = block[pt[pt_top_index].token_begin_index].here;

    // 参照元を登録
    int alias_id;
    // 参照先が構造体の場合
    if (is_pt_name("struct", pt[type_lex_token], bnf)) {
      for (alias_id=0; alias_id<type_empty_id-1; alias_id++) {
        const int empty_token_id = pt[type_lex_token].token_begin_index;
        const int alias_token_id = type[alias_id].token_id;

        if (is_same_word(
          token[alias_token_id].src, token[alias_token_id].begin, token[alias_token_id].end,
          token[empty_token_id].src, token[empty_token_id].begin, token[empty_token_id].end
        )) {
          break;
        }
      }

    // 参照先が構造体以外(intやchar)の場合
    } else {
      for (alias_id=0; alias_id<type_empty_id-1; alias_id++) {
        const int empty_bnf_id = pt[type_lex_token].bnf_id;
        const int alias_bnf_id = type[alias_id].bnf_id;

        if (empty_bnf_id == alias_bnf_id) {
          break;
        }
      }
    }
    type[type_empty_id].alias_id = alias_id;

    // 構文木から構造体を削除
    delete_pt_recursive(declaration, pt);
  }

  else {
    const int right = pt[pt_top_index].right;
    if (right >= 0) register_typedef_recursive(right, block, token, bnf, pt, type);
    const int down  = pt[pt_top_index].down;
    if (down >= 0)  register_typedef_recursive(down, block, token, bnf, pt, type);
  }
}
