#include "rvmath.h"

word_t sign_extend (word_t word, int size) {
  int mask = word & (1 << (size - 1));
  for (int i = 0; i < 5; ++i) mask |= (mask << (1 << i));
  return word | mask;
}
