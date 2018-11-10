#include <stdio.h>

int main(void) {
  int a[5];
  a[0] = 3;
  fprintf(stderr, "%d\n", *a);
  return 0;
}
