#include <stdlib.h>

#include "queue.h"
#include "reg.h"
#include "rv32i.h"

queue_t *queue_create (size_t capacity, size_t size,
                       clk_t *clk) {
  queue_t *queue = (queue_t *) malloc(sizeof(queue_t));
  queue->data =
    (reg_mut_t **) malloc(sizeof(reg_mut_t *) * capacity);
  for (int i = 0; i < capacity; ++i) {
    queue->data[i] = reg_mut_create(size, clk);
  }
  queue->head = reg_mut_create(sizeof(int), clk);
  queue->tail = reg_mut_create(sizeof(int), clk);
  queue->capacity = capacity;
  return queue;
}
void queue_free (queue_t *queue) {
  for (int i = 0; i < queue->capacity; ++i) {
    reg_mut_free(queue->data[i]);
  }
  free(queue->data);
  reg_mut_free(queue->head);
  reg_mut_free(queue->tail);
  free(queue);
}

int _queue_trunc (int id, size_t capacity) {
  if (id < 0) id += capacity;
  return id % capacity;
}
reg_mut_t *queue_nth (queue_t *queue, int ix) {
  int head = *rm_read(queue->head, int);
  ix = _queue_trunc(ix + head, queue->capacity);
  return queue->data[ix];
}
reg_mut_t *queue_id (queue_t *queue, int id) {
  return queue->data[id];
}
size_t queue_length (queue_t *queue) {
  int head = *rm_read(queue->head, int);
  int tail = *rm_read(queue->tail, int);
  return (tail + queue->capacity - head) % queue->capacity;
}
bool queue_empty (queue_t *queue) {
  int head = *rm_read(queue->head, int);
  int tail = *rm_read(queue->tail, int);
  return head == tail;
}
bool queue_full (queue_t *queue) {
  int head = *rm_read(queue->head, int);
  int tail = *rm_read(queue->tail, int);
  return head == _queue_trunc(tail + 1, queue->capacity);
}
reg_mut_t *queue_push (queue_t *queue) {
  int head = *rm_read(queue->head, int);
  int tail = *rm_read(queue->tail, int);
  int next = _queue_trunc(tail + 1, queue->capacity);
  if (head == next) return NULL;
  rm_write(queue->tail, int) = next;
  return queue->data[tail];
}
reg_mut_t *queue_pop (queue_t *queue) {
  int head = *rm_read(queue->head, int);
  int tail = *rm_read(queue->tail, int);
  if (head == tail) return NULL;
  rm_write(queue->head, int) =
    _queue_trunc(head + 1, queue->capacity);
  return queue->data[head];
}
reg_mut_t *queue_first (queue_t *queue) {
  int head = *rm_read(queue->head, int);
  int tail = *rm_read(queue->tail, int);
  if (head == tail) return NULL;
  return queue->data[head];
}
