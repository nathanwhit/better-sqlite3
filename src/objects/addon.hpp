#ifndef ADDON_H
#define ADDON_H

#include "../util/constants.hpp"
#include "../util/macros.hpp"
#include "database.hpp"
#include <sqlite3.h>

struct Addon {
  static void
  JS_setErrorConstructor(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void Cleanup(void *ptr);

  explicit Addon(v8::Isolate *isolate)
      : privileged_info(NULL), next_id(0), cs(isolate) {}

  inline sqlite3_uint64 NextId() { return next_id++; }

  v8::Global<v8::Function> Statement;
  v8::Global<v8::Function> StatementIterator;
  v8::Global<v8::Function> Backup;
  v8::Global<v8::Function> SqliteError;
  NODE_ARGUMENTS_POINTER privileged_info;
  sqlite3_uint64 next_id;
  CS cs;
  std::set<Database *, Database::CompareDatabase> dbs;
};

#endif
