#ifndef CUSTOM_FUNCTION_H
#define CUSTOM_FUNCTION_H

#include "data-converter.hpp"
#include "v8-isolate.h"
class Database;
class CustomFunction : protected DataConverter {
public:
  explicit CustomFunction(v8::Isolate *isolate, Database *db, const char *name,
                          v8::Local<v8::Function> fn, bool safe_ints)
      : name(name), db(db), isolate(isolate), fn(isolate, fn),
        safe_ints(safe_ints) {}

  virtual ~CustomFunction() {}

  static void xDestroy(void *self);

  static void xFunc(sqlite3_context *invocation, int argc,
                    sqlite3_value **argv);

protected:
  void PropagateJSError(sqlite3_context *invocation);

  std::string GetDataErrorPrefix();

private:
  const std::string name;
  Database *const db;

protected:
  v8::Isolate *const isolate;
  const v8::Global<v8::Function> fn;
  const bool safe_ints;
};

#endif
