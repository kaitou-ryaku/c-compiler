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

  const int type_used_size = search_unused_type_index(type);
  for (int i=0; i<type_used_size; i++) type[i].used_size = type_used_size;
}/*}}}*/
static void initialize_type_table(TYPE* type, const int type_max_size) {/*{{{*/
  for (int i=0; i<type_max_size; i++) {
    type[i].id         = i;
    type[i].total_size = type_max_size;
    type[i].used_size  = 0;
    type[i].sign       = SYMBOL_TYPE_UNUSED;
    type[i].length     = SYMBOL_TYPE_UNUSED;
    type[i].body       = SYMBOL_TYPE_UNUSED;
    type[i].token_id   = -1;
    type[i].alias_id   = -1;
    type[i].block      = -1;
    type[i].byte       = -1;
  }
}/*}}}*/
static int register_default_type(const BNF* bnf, TYPE* type) {/*{{{*/

  const int sign_size = 2;
  const int length_size = 3;
  const int body_size = 5;
  const int all_size = sign_size*length_size*body_size;

  for (int sign=0; sign<sign_size; sign++) {
    for (int length=0; length<length_size; length++) {
      for (int body=0; body<body_size; body++) {

        const int index = sign*length_size*body_size + length*body_size + body;
        if (sign == SYMBOL_TYPE_SIGNED) {
          type[index].sign = SYMBOL_TYPE_SIGNED;
        } else if (sign == SYMBOL_TYPE_UNSIGNED) {
          type[index].sign = SYMBOL_TYPE_UNSIGNED;
        } else {
          assert(0);
        }

        int body_factor;
        if (body == SYMBOL_TYPE_INT) {
          type[index].body = SYMBOL_TYPE_INT;
          body_factor = 4;
        } else if (body == SYMBOL_TYPE_VOID) {
          type[index].body = SYMBOL_TYPE_VOID;
          body_factor = 0;
        } else if (body == SYMBOL_TYPE_CHAR) {
          type[index].body = SYMBOL_TYPE_CHAR;
          body_factor = 1;
        } else if (body == SYMBOL_TYPE_FLOAT) {
          type[index].body = SYMBOL_TYPE_FLOAT;
          body_factor = 4;
        } else if (body == SYMBOL_TYPE_DOUBLE) {
          type[index].body = SYMBOL_TYPE_DOUBLE;
          body_factor = 8;
        } else {
          assert(0);
        }

        if (length == SYMBOL_TYPE_MEDIUM) {
          type[index].length = SYMBOL_TYPE_MEDIUM;
          type[index].byte = body_factor;
        } else if (length == SYMBOL_TYPE_LONG) {
          type[index].length = SYMBOL_TYPE_LONG;
          type[index].byte = body_factor*2;
        } else if (length == SYMBOL_TYPE_SHORT) {
          type[index].length = SYMBOL_TYPE_SHORT;
          type[index].byte = body_factor/2;
        } else {
          assert(0);
        }
      }
    }
  }

  for (int j=0; j<all_size; j++) type[j].block = 0;

  return all_size;
}/*}}}*/
extern void print_type_table(FILE* fp, const LEX_TOKEN* token, const BNF* bnf, const TYPE* type) {/*{{{*/
  for (int i=0; i<type[i].used_size; i++) {
    fprintf(fp, "%03d", i);
    fprintf(fp, " | block:%2d", type[i].block);
    fprintf(fp, " | byte:%3d" , type[i].byte);
    fprintf(fp, " | ");

    if (type[i].body == SYMBOL_TYPE_TYPEDEF_NAME) {
      assert(type[i].token_id >= 0);
      assert(type[i].alias_id >= 0);
      print_token_name(fp, token[type[i].token_id]);
      fprintf(fp, " --> ");
      fprintf(fp, "%03d", type[i].alias_id);

    } else if (type[i].body == SYMBOL_TYPE_STRUCT) {
      fprintf(fp, "struct");

    } else {
      if (type[i].sign == SYMBOL_TYPE_SIGNED  ) fprintf(fp, "signed  ");
      if (type[i].sign == SYMBOL_TYPE_UNSIGNED) fprintf(fp, "unsigned");
      fprintf(fp, " | ");
      if (type[i].length == SYMBOL_TYPE_MEDIUM) fprintf(fp, "      ");
      if (type[i].length == SYMBOL_TYPE_SHORT ) fprintf(fp, "short ");
      if (type[i].length == SYMBOL_TYPE_LONG  ) fprintf(fp, "long  ");
      fprintf(fp, " | ");
      if (type[i].body == SYMBOL_TYPE_INT         ) fprintf(fp, "int    ");
      if (type[i].body == SYMBOL_TYPE_VOID        ) fprintf(fp, "void   ");
      if (type[i].body == SYMBOL_TYPE_CHAR        ) fprintf(fp, "char   ");
      if (type[i].body == SYMBOL_TYPE_FLOAT       ) fprintf(fp, "float  ");
      if (type[i].body == SYMBOL_TYPE_DOUBLE      ) fprintf(fp, "double ");
      if (type[i].body == SYMBOL_TYPE_STRUCT      ) fprintf(fp, "struct ");
      if (type[i].body == SYMBOL_TYPE_TYPEDEF_NAME) fprintf(fp, "typedef");
    }
    fprintf(fp, "\n");
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
    type[type_empty_id].sign = SYMBOL_TYPE_UNUSED;
    type[type_empty_id].length = SYMBOL_TYPE_UNUSED;
    type[type_empty_id].body = SYMBOL_TYPE_STRUCT;
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
      specifier = register_type_specifier(member_empty_id, specifier, bnf, pt, member);
    } else if (is_pt_name("TYPE_QUALIFIER", pt[specifier], bnf)) {
      specifier = register_type_qualifier(member_empty_id, specifier, bnf, pt, member);
    } else {
      assert(0);
    }
  }
}/*}}}*/
static int search_unused_type_index(const TYPE* type) {/*{{{*/
  int ret;
  for (ret=0; ret<type[0].total_size; ret++) {
    if (type[ret].body == SYMBOL_TYPE_UNUSED) break;
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
    assert(storage_class_specifier >= 0);
    assert(is_pt_name("STORAGE_CLASS_SPECIFIER", pt[storage_class_specifier], bnf));

    const int type_specifier = search_pt_index_right("TYPE_SPECIFIER", storage_class_specifier, pt, bnf);
    assert(type_specifier >= 0);

    const int declaration_specifiers = pt[storage_class_specifier].up;
    assert(declaration_specifiers >= 0);
    assert(is_pt_name("DECLARATION_SPECIFIERS", pt[declaration_specifiers], bnf));

    const int declaration = pt[declaration_specifiers].up;
    assert(declaration >= 0);
    assert(is_pt_name("DECLARATION", pt[declaration], bnf));

    const int init_declarator_list = search_pt_index_right("INIT_DECLARATOR_LIST", declaration_specifiers, pt, bnf);
    assert(init_declarator_list >= 0);
    assert(is_pt_name("INIT_DECLARATOR_LIST", pt[init_declarator_list], bnf));

    const int init_declarator = pt[init_declarator_list].down;
    assert(init_declarator >= 0);
    assert(is_pt_name("INIT_DECLARATOR", pt[init_declarator], bnf));

    const int declarator = pt[init_declarator].down;
    assert(declarator >= 0);
    assert(is_pt_name("DECLARATOR", pt[declarator], bnf));

    const int direct_declarator = pt[declarator].down;
    assert(direct_declarator >= 0);
    assert(is_pt_name("DIRECT_DECLARATOR", pt[direct_declarator], bnf));

    const int identifier = pt[direct_declarator].down;
    assert(identifier >= 0);
    assert(is_pt_name("identifier", pt[identifier], bnf));

    // 登録
    const int type_empty_id = search_unused_type_index(type);
    type[type_empty_id].sign   = SYMBOL_TYPE_UNUSED;
    type[type_empty_id].length = SYMBOL_TYPE_UNUSED;
    type[type_empty_id].body   = SYMBOL_TYPE_TYPEDEF_NAME;
    type[type_empty_id].token_id = pt[identifier].token_begin_index;
    type[type_empty_id].block = block[pt[pt_top_index].token_begin_index].here;

    // 参照元の型を解析
    SYMBOL origin_symbol[1];
    int origin_array[100];
    initialize_symbol_table(origin_symbol, sizeof(origin_symbol)/sizeof(SYMBOL), origin_array, sizeof(origin_array)/sizeof(int));
    register_type_specifier(0, type_specifier, bnf, pt, origin_symbol);

    // 参照元を登録
    int alias_id=0;
    // 参照先が構造体の場合
    if (origin_symbol[0].type_body == SYMBOL_TYPE_STRUCT) {
      for (alias_id=0; alias_id<type_empty_id-1; alias_id++) {
        const int empty_token_id = origin_symbol[0].body_token_id;
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
        if (!(origin_symbol[0].type_sign   == type[alias_id].sign  )) continue;
        if (!(origin_symbol[0].type_length == type[alias_id].length)) continue;
        if (!(origin_symbol[0].type_body   == type[alias_id].body  )) continue;
        break;
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
}/*}}}*/
extern int search_type_table_by_declare_token(const int token_index , const BNF* bnf , const TYPE* type) {/*{{{*/
  int type_index;
  const int used_size = type[0].used_size;
  for (type_index=0; type_index<used_size; type_index++) {
    if (type[type_index].token_id >= 0) {
      if (type[type_index].token_id == token_index) {
        if (type[type_index].body == SYMBOL_TYPE_STRUCT || type[type_index].body == SYMBOL_TYPE_TYPEDEF_NAME) break;
      }
    }
  }
  if (type_index == used_size) type_index = -1;

  return type_index;
}/*}}}*/
