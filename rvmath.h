#pragma once

#include "rv32i.h"

word_t sign_extend (word_t word, int size);
int ls_size (ls_size_t size);
word_t signed_add (word_t base, word_t dx);
