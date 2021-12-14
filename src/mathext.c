#include "mathext.h"

int factorial (int x) {
  int ret = 1;
  while (x)
    ret *= x--;
  return ret;
}
