#ifndef BINDER_H
#define BINDER_H

#include "macros.hpp"
#include <node_buffer.h>
#include <sqlite3.h>

class Statement;

class Binder {
public:
  explicit Binder(sqlite3_stmt *_handle) {
    handle = _handle;
    param_count = sqlite3_bind_parameter_count(_handle);
    anon_index = 0;
    success = true;
  }

  bool Bind(NODE_ARGUMENTS info, int argc, Statement *stmt);

private:
  struct Result {
    int count;
    bool bound_object;
  };

  static bool IsPlainObject(v8::Isolate *isolate, v8::Local<v8::Object> obj);

  void Fail(void (*Throw)(const char *_), const char *message);

  int NextAnonIndex();

  // Binds the value at the given index or throws an appropriate error.
  void BindValue(v8::Isolate *isolate, v8::Local<v8::Value> value, int index);

  // Binds each value in the array or throws an appropriate error.
  // The number of successfully bound parameters is returned.
  int BindArray(v8::Isolate *isolate, v8::Local<v8::Array> arr);

  // Binds all named parameters using the values found in the given object.
  // The number of successfully bound parameters is returned.
  // If a named parameter is missing from the object, an error is thrown.
  // This should only be invoked once per instance.
  int BindObject(v8::Isolate *isolate, v8::Local<v8::Object> obj,
                 Statement *stmt);

  // Binds all parameters using the values found in the arguments object.
  // Anonymous parameter values can be directly in the arguments object or in an
  // Array. Named parameter values can be provided in a plain Object argument.
  // Only one plain Object argument may be provided.
  // If an error occurs, an appropriate error is thrown.
  // The return value is a struct indicating how many parameters were
  // successfully bound and whether or not it tried to bind an object.
  Result BindArgs(NODE_ARGUMENTS info, int argc, Statement *stmt);

  sqlite3_stmt *handle;
  int param_count;
  int anon_index; // This value should only be used by NextAnonIndex()
  bool success;   // This value should only be set by Fail()
};

#endif
