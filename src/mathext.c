#include "mathext.h"

int factorial (int x) {
  int ret = 1;
  while (x)
    ret *= x--;
  return ret;
}

int permutations (int n, int r) {
  return (double)factorial(n) / (double)factorial(n - r);
}

int combinations (int n, int r) {
  return (double)factorial(n) / (
           (double)factorial(r) *
           (double)factorial(n - r)
         );
}
