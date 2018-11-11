#include "../include/common.h"
#include "../include/type.h"
#include <assert.h>
#include <stdio.h>

extern int create_default_type(
  const LEX_TOKEN* token
  , PARSE_TREE* pt
  , const BNF* bnf
  , TYPE* type
  , const int type_max_size
) {
  return 3;
}
