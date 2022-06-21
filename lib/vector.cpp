#include <vector>

extern "C" {

#include "vector.h"

struct vector_t {
  std::vector<void *> vec;
};

vector_t *vector_create () {
  return new vector_t;
}
void vector_destroy (vector_t *vec) {
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

}
