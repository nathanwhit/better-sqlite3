#ifndef STATEMENT_H
#define STATEMENT_H

#include "../util/bind-map.hpp"
#include "../util/macros.hpp"
#include <node.h>
#include <node_buffer.h>
#include <node_object_wrap.h>

#include <node.h>
#include <node_object_wrap.h>
#include <sqlite3.h>

class Database;

class Statement : public node::ObjectWrap {
  friend class StatementIterator;

public:
  INIT(Init) {
    v8::Local<v8::FunctionTemplate> t =
        NewConstructorTemplate(isolate, data, JS_new, "Statement");
    SetPrototypeMethod(isolate, data, t, "run", JS_run);
    SetPrototypeMethod(isolate, data, t, "get", JS_get);
    SetPrototypeMethod(isolate, data, t, "all", JS_all);
    SetPrototypeMethod(isolate, data, t, "iterate", JS_iterate);
    SetPrototypeMethod(isolate, data, t, "bind", JS_bind);
    SetPrototypeMethod(isolate, data, t, "pluck", JS_pluck);
    SetPrototypeMethod(isolate, data, t, "expand", JS_expand);
    SetPrototypeMethod(isolate, data, t, "raw", JS_raw);
    SetPrototypeMethod(isolate, data, t, "safeIntegers", JS_safeIntegers);
    SetPrototypeMethod(isolate, data, t, "columns", JS_columns);
    SetPrototypeGetter(isolate, data, t, "busy", JS_busy);
    return t->GetFunction(OnlyContext).ToLocalChecked();
  }

  // Used to support ordered containers.
  static inline bool Compare(Statement const *const a,
                             Statement const *const b) {
    return a->extras->id < b->extras->id;
  }

  // Returns the Statement's bind map (creates it upon first execution).
  BindMap *GetBindMap(v8::Isolate *isolate) {
    if (has_bind_map)
      return &extras->bind_map;
    BindMap *bind_map = &extras->bind_map;
    int param_count = sqlite3_bind_parameter_count(handle);
    for (int i = 1; i <= param_count; ++i) {
      const char *name = sqlite3_bind_parameter_name(handle, i);
      if (name != NULL)
        bind_map->Add(isolate, name + 1, i);
    }
    has_bind_map = true;
    return bind_map;
  }

  // Whenever this is used, db->RemoveStatement must be invoked beforehand.
  void CloseHandles() {
    if (alive) {
      alive = false;
      sqlite3_finalize(handle);
    }
  }

  ~Statement();

private:
  // A class for holding values that are less often used.
  class Extras {
    friend class Statement;
    explicit Extras(sqlite3_uint64 id) : bind_map(0), id(id) {}
    BindMap bind_map;
    const sqlite3_uint64 id;
  };

  explicit Statement(Database *db, sqlite3_stmt *handle, sqlite3_uint64 id,
                     bool returns_data);

  static void JS_new(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_run(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_get(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_all(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);
  static void JS_iterate(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_bind(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_pluck(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_expand(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_raw(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void
  JS_safeIntegers(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_columns(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  NODE_GETTER(JS_busy) {
    Statement *stmt = Unwrap<Statement>(info.This());
    info.GetReturnValue().Set(stmt->alive && stmt->locked);
  }

  Database *const db;
  sqlite3_stmt *const handle;
  Extras *const extras;
  bool alive;
  bool locked;
  bool bound;
  bool has_bind_map;
  bool safe_ints;
  char mode;
  const bool returns_data;
};

#endif
