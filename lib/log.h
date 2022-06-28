#pragma once

#include <stdio.h>

#define debug_log(...)                                \
  fprintf(stderr, "In %s:%d : ", __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__);                       \
  fprintf(stderr, "\n")
