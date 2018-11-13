#ifndef __C_COMPILER_MEMORY__
#define __C_COMPILER_MEMORY__

#include "common.h"

int sizeof_symbol_array(const int byte, const int* array, const int array_size);
void register_symbol_size(const TYPE* type, SYMBOL* symbol);

#endif
