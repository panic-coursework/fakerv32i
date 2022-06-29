#pragma once

#include <stdio.h>

#define debug_log(...) \
  fprintf(stderr, "In %s (%s:%d) : ", __PRETTY_FUNCTION__, __FILE__, __LINE__); \
  fprintf(stderr, __VA_ARGS__); \
  fprintf(stderr, "\n")
