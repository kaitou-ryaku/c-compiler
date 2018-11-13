#include "../include/common.h"
#include "../include/memory.h"
#include "../include/symbol.h"
#include <assert.h>

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
