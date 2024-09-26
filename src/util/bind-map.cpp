#include "bind-map.hpp"
#include <v8.h>

void BindMap::Add(v8::Isolate *isolate, const char *name, int index) {
  assert(name != NULL);
  if (capacity == length)
    Grow(isolate);
  new (pairs + length++) Pair(isolate, name, index);
}

void BindMap::Grow(v8::Isolate *isolate) {
  assert(capacity == length);
  capacity = (capacity << 1) | 2;
  Pair *new_pairs = ALLOC_ARRAY<Pair>(capacity);
  for (int i = 0; i < length; ++i) {
    new (new_pairs + i) Pair(isolate, pairs + i);
    pairs[i].~Pair();
  }
  FREE_ARRAY<Pair>(pairs);
  pairs = new_pairs;
}
