#include <vector>

extern "C" {

#include "vector.h"

struct vector_t {
  std::vector<void *> vec;
};

vector_t *vector_create () {
  return new vector_t;
}
void vector_free (vector_t *vec) {
  delete vec;
}
void vector_write (vector_t *vec, size_t ix, void *value) {
  vec->vec[ix] = value;
}
void *vector_read (vector_t *vec, size_t ix) {
  return vec->vec[ix];
}
size_t vector_push (vector_t *vec, void *value) {
  vec->vec.push_back(value);
  return vec->vec.size();
}
size_t vector_size (vector_t *vec) {
  return vec->vec.size();
}

int _vector_foreach(vector_t *vec, size_t i, void **var) {
  if (i >= vector_size(vec)) return 0;
  *var = vector_read(vec, i);
  return 1;
}

}
