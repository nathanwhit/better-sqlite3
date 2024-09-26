#ifndef BIND_MAP_H
#define BIND_MAP_H

#include "macros.hpp"

class BindMap {
public:
  // This nested class represents a single mapping between a parameter name
  // and its associated parameter index in a prepared statement.
  class Pair {
    friend class BindMap;

  public:
    inline int GetIndex() { return index; }

    inline v8::Local<v8::String> GetName(v8::Isolate *isolate) {
      return name.Get(isolate);
    }

  private:
    explicit Pair(v8::Isolate *isolate, const char *name, int index)
        : name(isolate, InternalizedFromUtf8(isolate, name, -1)), index(index) {
    }

    explicit Pair(v8::Isolate *isolate, Pair *pair)
        : name(isolate, pair->name), index(pair->index) {}

    const v8::Global<v8::String> name;
    const int index;
  };

  explicit BindMap(char _) {
    assert(_ == 0);
    pairs = NULL;
    capacity = 0;
    length = 0;
  }

  ~BindMap() {
    while (length)
      pairs[--length].~Pair();
    FREE_ARRAY<Pair>(pairs);
  }

  inline Pair *GetPairs() { return pairs; }

  inline int GetSize() { return length; }

  // Adds a pair to the bind map, expanding the capacity if necessary.
  void Add(v8::Isolate *isolate, const char *name, int index);

private:
  void Grow(v8::Isolate *isolate);

  Pair *pairs;
  int capacity;
  int length;
};

#endif
