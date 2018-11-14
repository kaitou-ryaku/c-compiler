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
extern void register_symbol_size(const TYPE* type, SYMBOL* symbol) {/*{{{*/
  const int symbol_count = search_unused_symbol_index(symbol);
  for (int i=0; i<symbol_count; i++) {
    symbol[i].byte = sizeof_symbol_array(4, symbol[i].array, symbol[i].array_size);
  }
}/*}}}*/
extern void register_type_and_symbol_size(/*{{{*/
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

    // print_token_name(stderr, token[token_index-1]);
    // fprintf(stderr, " <");
    // print_token_name(stderr, token[token_index]);
    // fprintf(stderr, "> ");
    // print_token_name(stderr, token[token_index+1]);
    // fprintf(stderr, "      ");
    if (type_index >= 0)  {
      if (type[type_index].byte >= 0) continue;
      const char* bnf_name = bnf[type[type_index].bnf_id].name;

      // TODO typedef int hoge;のみ対応。int*やint[10]には未対応
      if (strcmp("typedef", bnf_name)==0) {
        const int alias_id = type[type_index].alias_id;

        int origin_type_index=0;
        while (origin_type_index < type[0].used_size) {
          if (is_same_token_str(alias_id, origin_type_index, token)) {
            type[type_index].byte = type[origin_type_index].byte;
            break;
          }
          origin_type_index++;
        }
        assert(origin_type_index < type[0].used_size);

      } else if (strcmp("struct", bnf_name)==0) {

      } else {
        assert(0);
      }

    } else if (symbol_index >= 0) {
      if (symbol[symbol_index].byte >= 0) continue;

    } else {
      assert(0);
    }
  }
}/*}}}*/
