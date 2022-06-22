#pragma once

#define debug_log(...) _debug_log(__FILE__, __LINE__, \
                                  __VA_ARGS__)

void _debug_log (const char *filename, int line,
                 const char *format, ...);
