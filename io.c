#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#include "io.h"
#include "memory.h"
#include "reg-store.h"
#include "rv32i.h"

word_t _read_num () {
  word_t word;
  scanf("%x", &word);
  return word;
}

void read_to_memory (memory_t *mem) {
  char c = getchar();
  while (true) {
    assert(c == '@');
    addr_t addr = (addr_t) _read_num();
    do {
      byte_t octet = (byte_t) _read_num();
      mem_set(mem, addr, octet);
      ++addr;
      do {
        c = getchar();
      } while (c != '@' && c != EOF && !isalnum(c));
      if (isalnum(c)) ungetc(c, stdin);
    } while (c != '@' && c != EOF);
    if (c == EOF) break;
  }
}

#define RESULT_REG_ID 10
#define RESULT_MASK   255u

void write_result (reg_store_t *reg_store) {
  word_t res = reg_store_get(reg_store, RESULT_REG_ID);
  printf("%u\n", res & RESULT_MASK);
}
