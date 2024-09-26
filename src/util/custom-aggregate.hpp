#ifndef CUSTOM_AGGREGATE_H
#define CUSTOM_AGGREGATE_H

#include "custom-function.hpp"
#include <sqlite3.h>

class CustomAggregate : public CustomFunction {
public:
  explicit CustomAggregate(v8::Isolate *isolate, Database *db, const char *name,
                           v8::Local<v8::Value> start,
                           v8::Local<v8::Function> step,
                           v8::Local<v8::Value> inverse,
                           v8::Local<v8::Value> result, bool safe_ints);

  static void xStep(sqlite3_context *invocation, int argc,
                    sqlite3_value **argv) {
    xStepBase(invocation, argc, argv, &CustomAggregate::fn);
  }

  static void xInverse(sqlite3_context *invocation, int argc,
                       sqlite3_value **argv) {
    xStepBase(invocation, argc, argv, &CustomAggregate::inverse);
  }

  static void xValue(sqlite3_context *invocation) {
    xValueBase(invocation, false);
  }

  static void xFinal(sqlite3_context *invocation) {
    xValueBase(invocation, true);
  }

private:
  static void xStepBase(sqlite3_context *invocation, int argc,
                        sqlite3_value **argv,
                        const v8::Global<v8::Function> CustomAggregate::*ptrtm);

  static void xValueBase(sqlite3_context *invocation, bool is_final);

  struct Accumulator {
  public:
    v8::Global<v8::Value> value;
    bool initialized;
    bool is_window;
  };

  Accumulator *GetAccumulator(sqlite3_context *invocation);

  static void DestroyAccumulator(sqlite3_context *invocation);

  void PropagateJSError(sqlite3_context *invocation);

  const bool invoke_result;
  const bool invoke_start;
  const v8::Global<v8::Function> inverse;
  const v8::Global<v8::Function> result;
  const v8::Global<v8::Value> start;
};

#endif
