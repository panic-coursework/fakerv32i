#include <stdlib.h>

#include "reg-file.h"
#include "reg.h"
#include "rv32i.h"

reg_file_t *reg_file_create (clk_t *clk) {
  reg_file_t *reg_file =
    (reg_file_t *) malloc(sizeof(reg_file_t));
  for (int i = 0; i < REG_COUNT; ++i) {
    reg_file->rs_id[i] =
      reg_mut_create(sizeof(rs_id_t), clk);
  }
  return reg_file;
}
void reg_file_free (reg_file_t *reg_file) {
  for (int i = 0; i < REG_COUNT; ++i) {
    reg_mut_free(reg_file->rs_id[i]);
  }
  free(reg_file);
}
