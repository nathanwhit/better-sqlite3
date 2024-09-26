#ifndef STATEMENT_ITERATOR_H
#define STATEMENT_ITERATOR_H

#include "../util/macros.hpp"
#include "addon.hpp"
#include "statement.hpp"
#include <node_object_wrap.h>

class StatementIterator : public node::ObjectWrap {
public:
  static v8 ::Local<v8 ::Function> Init(v8 ::Isolate *isolate,
                                        v8 ::Local<v8 ::External> data);

  // The ~Statement destructor currently covers any state this object creates.
  // Additionally, we actually DON'T want to revert stmt->locked or db_state
  // ->iterators in this destructor, to ensure deterministic database access.
  ~StatementIterator() {}

private:
  explicit StatementIterator(Statement *stmt, bool bound);

  static void JS_new(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_next(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_return(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void
  JS_symbolIterator(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  void Next(NODE_ARGUMENTS info);

  void Return(NODE_ARGUMENTS info);

  void Throw();

  void Cleanup();

  static v8::Local<v8::Object> NewRecord(v8::Isolate *isolate,
                                         v8::Local<v8::Context> ctx,
                                         v8::Local<v8::Value> value,
                                         Addon *addon, bool done);

  static v8::Local<v8::Object> DoneRecord(v8::Isolate *isolate, Addon *addon);

  Statement *const stmt;
  sqlite3_stmt *const handle;
  Database::State *const db_state;
  const bool bound;
  const bool safe_ints;
  const char mode;
  bool alive;
  bool logged;
};

#endif
