#include "../min-bnf-parser/include/min-bnf-parser.h"
#include "../include/common.h"
#include "../include/memory.h"
#include "../include/symbol.h"
#include "../include/type.h"
#include "../include/pt_common.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static const int ADDRESS_BYTE=8;

// 関数プロトタイプ/*{{{*/
static void register_typedef_size(const int type_index, TYPE* type);
static int register_symbol_size(
  const int symbol_index
  , const LEX_TOKEN* token
  , const BNF* bnf
  , const PARSE_TREE* pt
  , const TYPE* type
  , SYMBOL* symbol
);
static int register_struct_size(
  const int token_struct_index
  , const int type_index
  , const BLOCK* block
  , const LEX_TOKEN* token
  , const BNF* bnf
  , const PARSE_TREE* pt
  , TYPE* type
  , SYMBOL* symbol
);
static void register_type_and_symbol_size(
  const BLOCK* block
  , const LEX_TOKEN* token
  , const BNF* bnf
  , const PARSE_TREE* pt
  , TYPE* type
  , SYMBOL* symbol
);
static void fix_default_storage_class(SYMBOL* symbol);
static void register_local_variable_address(SYMBOL* symbol);
static void register_local_argument_address(SYMBOL* symbol);
/*}}}*/
extern int sizeof_symbol_array(const int byte, const int* array, const int array_size) {/*{{{*/
  int ret;

  // 型そのもの
  if (array_size == 0) {
    ret=byte;

  // 配列かポインタ
  } else if (array_size > 0){
    ret=1;
    for (int i=0; i<array_size; i++) {
      const int index=array_size-i-1;
      // ポインタの場合
      if (array[index] == 0) {
        ret=ret*ADDRESS_BYTE;
        break;

      // 配列の場合
      } else if (array[index] > 0){
        ret=ret*array[index];
        if (index == 0) {
          ret=ret*byte;
          break;
        }

      } else {
        assert(0);
      }
    }

  } else {
    assert(0);
  }

  return ret;
}/*}}}*/
extern void format_type_and_symbol_table(/*{{{*/
  const BLOCK* block
  , const LEX_TOKEN* token
  , const BNF* bnf
  , const PARSE_TREE* pt
  , TYPE* type
  , SYMBOL* symbol
) {
  register_type_and_symbol_size(block, token, bnf, pt, type, symbol);
  fix_default_storage_class(symbol);
  register_local_variable_address(symbol);
  register_local_argument_address(symbol);
}/*}}}*/
static void register_type_and_symbol_size(/*{{{*/
  const BLOCK* block
  , const LEX_TOKEN* token
  , const BNF* bnf
  , const PARSE_TREE* pt
  , TYPE* type
  , SYMBOL* symbol
) {
  // ソースを上から下までたどり、各変数とtypedefの型を決定
  for (int token_index=0; token_index<token[0].used_size; token_index++) {
    const int type_index = search_type_table_by_declare_token(token_index, bnf, type);
    const int symbol_index = search_symbol_table_by_declare_token(token_index, symbol);
    assert((type_index < 0) || (symbol_index < 0));
    if ((type_index < 0) && (symbol_index < 0)) continue;

    if (type_index >= 0)  {
      if (type[type_index].byte >= 0) continue;

      // TODO typedef int hoge;のみ対応。typedef int* hoge;や配列には未対応
      if (type[type_index].body == SYMBOL_TYPE_TYPEDEF_NAME) {
        register_typedef_size(type_index, type);

      } else if (type[type_index].body == SYMBOL_TYPE_STRUCT) {
        token_index = register_struct_size(token_index, type_index, block, token, bnf, pt, type, symbol);

      } else {
        assert(0);
      }

    } else if (symbol_index >= 0) {
      if (symbol[symbol_index].byte >= 0) continue;
      register_symbol_size(symbol_index, token, bnf, pt, type, symbol);

    } else {
      assert(0);
    }
  }
}/*}}}*/
static void register_typedef_size(const int type_index, TYPE* type) {/*{{{*/
  assert(type_index >= 0);
  const int alias_id = type[type_index].alias_id;

  int origin_type_index=0;
  while (origin_type_index < type[0].used_size) {
    if (alias_id == origin_type_index) {
      type[type_index].byte = type[origin_type_index].byte;
      break;
    }
    origin_type_index++;
  }
  assert(origin_type_index < type[0].used_size);
}/*}}}*/
static int register_symbol_size(const int symbol_index, const LEX_TOKEN* token, const BNF* bnf, const PARSE_TREE* pt, const TYPE* type, SYMBOL* symbol) {/*{{{*/
  assert(symbol[symbol_index].body_token_id >= 0);

  if (symbol[symbol_index].type_body == SYMBOL_TYPE_TYPEDEF_NAME) {
    const int symbol_token_id = symbol[symbol_index].body_token_id;

    int origin_type_index=0;
    while (origin_type_index < type[0].used_size) {
      const int origin_token_id = type[origin_type_index].token_id;
      if (is_same_token_str(symbol_token_id, origin_token_id, token)) {
        const int byte = type[origin_type_index].byte;
        symbol[symbol_index].original_byte = byte;
        symbol[symbol_index].byte = sizeof_symbol_array(byte, symbol[symbol_index].array, symbol[symbol_index].array_size);
        break;
      }
      origin_type_index++;
    }
    assert(origin_type_index < type[0].used_size);

  } else {
    int origin_type_index=0;
    while (origin_type_index < type[0].used_size) {
      if ( (symbol[symbol_index].type_body   == type[origin_type_index].body)
        && (symbol[symbol_index].type_sign   == type[origin_type_index].sign)
        && (symbol[symbol_index].type_length == type[origin_type_index].length)
      ) {
        const int byte = type[origin_type_index].byte;
        symbol[symbol_index].original_byte = byte;
        symbol[symbol_index].byte = sizeof_symbol_array(byte, symbol[symbol_index].array, symbol[symbol_index].array_size);
        break;
      }
      origin_type_index++;
    }
    assert(origin_type_index < type[0].used_size);
  }

  return symbol[symbol_index].byte;
  return 0;
}/*}}}*/
static int register_struct_size(/*{{{*/
  const int token_struct_index
  , const int type_index
  , const BLOCK* block
  , const LEX_TOKEN* token
  , const BNF* bnf
  , const PARSE_TREE* pt
  , TYPE* type
  , SYMBOL* symbol
) {

  assert(token_struct_index >= 0);
  assert(type_index >= 0);
  const int out_block = block[token_struct_index].here;

  // structの後の{まで移動
  int token_index = token_struct_index;
  while ((token_index < token[0].used_size) && block[token_index].here == out_block) {
    token_index++;
  }
  assert(token_index < token[0].used_size);

  // 構造体メンバのサイズをシンボルテーブルに登録
  int offset = 0;
  while ((token_index < token[0].used_size) && (block[token_index].here != out_block)) {
    const int symbol_index = search_symbol_table_by_declare_token(token_index, symbol);
    if (symbol_index >= 0) {
      assert(symbol[symbol_index].byte < 0);
      const int byte = register_symbol_size(symbol_index, token, bnf, pt, type, symbol);
      assert(byte >= 0);
      symbol[symbol_index].address = offset;
      offset = offset+byte;
    }
    token_index++;
  }
  assert(token_index < token[0].used_size);
  type[type_index].byte = offset;

  return token_index-1;
}/*}}}*/
static void fix_default_storage_class(SYMBOL* symbol) {/*{{{*/
  for (int line=0; line<symbol[0].used_size; line++) {
    if (symbol[line].storage == SYMBOL_STORAGE_REGISTER) {
      fprintf(stderr, "Error: storage class register is unsupported\n");
      assert(0);
    }
    const int kind = symbol[line].kind;
    assert(kind != SYMBOL_TABLE_UNUSED);
    const int block = symbol[line].block;

    // 変数/*{{{*/
    if (kind == SYMBOL_TABLE_VARIABLE) {
      assert(symbol[line].token_id >= 0);
      // 関数外変数のデフォルト値をstaticに
      if (block == 0) {
        if (symbol[line].storage == SYMBOL_STORAGE_AUTO) {
          symbol[line].storage = SYMBOL_STORAGE_STATIC;
        }
      }
      // 関数内変数のexternをautoに
      else if (block > 0){
        if (symbol[line].storage == SYMBOL_STORAGE_EXTERN) {
          symbol[line].storage = SYMBOL_STORAGE_AUTO;
        }
      }
      else {
        assert(0);
      }
    }/*}}}*/
    // 関数定義かプロトタイプ宣言 -> デフォルトはextern/*{{{*/
    if (kind == SYMBOL_TABLE_FUNCTION || kind == SYMBOL_TABLE_PROTOTYPE) {
      assert(symbol[line].token_id >= 0);
      if (symbol[line].storage == SYMBOL_STORAGE_AUTO) {
        symbol[line].storage = SYMBOL_STORAGE_EXTERN;
      }
    }/*}}}*/
    // 関数定義の引数 -> auto以外ダメ/*{{{*/
    if (kind == SYMBOL_TABLE_F_ARGUMENT   ) {
      // assert(symbol[line].token_id >= 0); // voidの場合は-1になる
      assert(symbol[line].storage == SYMBOL_STORAGE_AUTO);
    }/*}}}*/
    // プロトタイプ宣言の引数 -> auto以外ダメ/*{{{*/
    if (kind == SYMBOL_TABLE_P_ARGUMENT   ) {
      assert(symbol[line].storage == SYMBOL_STORAGE_AUTO);
      symbol[line].token_id = -1;
    }/*}}}*/
    // 構造体メンバ -> auto以外ダメ/*{{{*/
    if (kind == SYMBOL_TABLE_STRUCT_MEMBER) {
      assert(symbol[line].token_id >= 0);
      assert(symbol[line].storage == SYMBOL_STORAGE_AUTO);
    }/*}}}*/
  }
}/*}}}*/
static void register_local_variable_address(SYMBOL* symbol) {/*{{{*/
  for (int func=0; func<symbol[0].used_size; func++) {
    int address = 0;
    if (symbol[func].kind == SYMBOL_TABLE_FUNCTION) {
      for (int var=0; var<symbol[0].used_size; var++) {
        if ((symbol[var].kind == SYMBOL_TABLE_VARIABLE) && (symbol[var].storage == SYMBOL_STORAGE_AUTO) && (symbol[var].function_id == func)) {
          symbol[var].address = address;
          address += symbol[var].byte;
        }
      }
    }
  }
}/*}}}*/
static void register_local_argument_address(SYMBOL* symbol) {/*{{{*/
  for (int func=0; func<symbol[0].used_size; func++) {
    int address = ADDRESS_BYTE; // cdecl既約で、直近のアドレスには関数の呼び出し元アドレスが入る。引数はそれ以前にスタックに積まれた値を参照
    if (symbol[func].kind == SYMBOL_TABLE_FUNCTION) {
      for (int arg=0; arg<symbol[0].used_size; arg++) {
        if ((symbol[arg].kind == SYMBOL_TABLE_F_ARGUMENT) && (symbol[arg].function_id == func)) {
          assert(symbol[arg].storage == SYMBOL_STORAGE_AUTO);
          address += symbol[arg].byte;
          symbol[arg].address = address;
        }
      }
    }
  }
}/*}}}*/
