#include "../min-bnf-parser/include/min-bnf-parser.h"
#include "../include/common.h"
#include "../include/memory.h"
#include "../include/symbol.h"
#include "../include/pt_common.h"
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
  for (int token_index=0; token_index<token[0].used_size; token_index++) {
    bool is_type_default=false;
    bool is_type_struct=false;
    bool is_type_typedef=false;
    int type_index;
    for (type_index=0; type_index<type[0].used_size; type_index++) {
      if (token_index == type[type_index].alias_id) {
        is_type_typedef=true;
        break;
      }
    }

    bool is_symbol=false;
    int symbol_index;
    for (symbol_index=0; symbol_index<symbol[0].used_size; symbol_index++) {
      if (token_index == symbol[symbol_index].token_id) {
        is_symbol=true;
        break;
      }
    }

    const bool is_type   = (is_type_default || is_type_struct || is_type_typedef);
    if ((!is_type) && (!is_symbol)) continue;
    if (is_type && is_symbol) assert(0);

  }
}/*}}}*/
