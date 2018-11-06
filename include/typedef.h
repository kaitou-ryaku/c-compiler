#ifndef __C_COMPILER_TYPEDEF__
#define __C_COMPILER_TYPEDEF__

#include <stdio.h>
#include "common.h"

void replace_typedef(FILE* fp, const BLOCK* block, LEX_TOKEN* token, const BNF* bnf);

#endif
