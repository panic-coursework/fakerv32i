#include "log.h"

#include <stdarg.h>
#include <stdio.h>

void _debug_log (const char *filename, int line,
                 const char *format, ...) {
  fprintf(stderr, "In %s:%d :\n", filename, line);
  va_list args;
  va_start(args, format);
  fprintf(stderr, format, args);
  va_end(args);
}
