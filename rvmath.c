#include "rvmath.h"
#include "lib/log.h"
#include "rv32i.h"
#include <assert.h>

word_t sign_extend (word_t word, int size) {
  int mask = word & (1 << (size - 1));
  for (int i = 0; i < 5; ++i) mask |= (mask << (1 << i));
  return word | mask;
}

int ls_size (ls_size_t size) {
  switch (size) {
    case LS_WORD: return 4;
    case LS_HWORD: case LS_HWORDU: return 2;
    case LS_BYTE: case LS_BYTEU: return 1;
    default:
    debug_log("unknown ls size %d", size);
    assert(0);
  }
}
