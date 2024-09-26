#ifndef DATA_H
#define DATA_H

#include "data-converter.hpp"
#include <node.h>
#include <sqlite3.h>

#define JS_VALUE_TO_SQLITE(to, value, isolate, ...)                            \
  if (value->IsNumber()) {                                                     \
    return sqlite3_##to##_double(__VA_ARGS__,                                  \
                                 value.As<v8::Number>()->Value());             \
  } else if (value->IsBigInt()) {                                              \
    bool lossless;                                                             \
    int64_t v = value.As<v8::BigInt>()->Int64Value(&lossless);                 \
    if (lossless) {                                                            \
      return sqlite3_##to##_int64(__VA_ARGS__, v);                             \
    }                                                                          \
  } else if (value->IsString()) {                                              \
    v8::String::Utf8Value utf8(isolate, value.As<v8::String>());               \
    return sqlite3_##to##_text(__VA_ARGS__, *utf8, utf8.length(),              \
                               SQLITE_TRANSIENT);                              \
  } else if (node::Buffer::HasInstance(value)) {                               \
    const char *data = node::Buffer::Data(value);                              \
    return sqlite3_##to##_blob(__VA_ARGS__, data ? data : "",                  \
                               node::Buffer::Length(value), SQLITE_TRANSIENT); \
  } else if (value->IsNull() || value->IsUndefined()) {                        \
    return sqlite3_##to##_null(__VA_ARGS__);                                   \
  }

#define SQLITE_VALUE_TO_JS(from, isolate, safe_ints, ...)                      \
  switch (sqlite3_##from##_type(__VA_ARGS__)) {                                \
  case SQLITE_INTEGER:                                                         \
    if (safe_ints) {                                                           \
      return v8::BigInt::New(isolate, sqlite3_##from##_int64(__VA_ARGS__));    \
    }                                                                          \
  case SQLITE_FLOAT:                                                           \
    return v8::Number::New(isolate, sqlite3_##from##_double(__VA_ARGS__));     \
  case SQLITE_TEXT:                                                            \
    return StringFromUtf8(                                                     \
        isolate,                                                               \
        reinterpret_cast<const char *>(sqlite3_##from##_text(__VA_ARGS__)),    \
        sqlite3_##from##_bytes(__VA_ARGS__));                                  \
  case SQLITE_BLOB:                                                            \
    return node::Buffer::Copy(                                                 \
               isolate,                                                        \
               static_cast<const char *>(sqlite3_##from##_blob(__VA_ARGS__)),  \
               sqlite3_##from##_bytes(__VA_ARGS__))                            \
        .ToLocalChecked();                                                     \
  default:                                                                     \
    assert(sqlite3_##from##_type(__VA_ARGS__) == SQLITE_NULL);                 \
    return v8::Null(isolate);                                                  \
  }                                                                            \
  assert(false);

namespace Data {

static const char FLAT = 0;
static const char PLUCK = 1;
static const char EXPAND = 2;
static const char RAW = 3;

v8::Local<v8::Value> GetValueJS(v8::Isolate *isolate, sqlite3_stmt *handle,
                                int column, bool safe_ints);

v8::Local<v8::Value> GetValueJS(v8::Isolate *isolate, sqlite3_value *value,
                                bool safe_ints);

v8::Local<v8::Value> GetFlatRowJS(v8::Isolate *isolate,
                                  v8::Local<v8::Context> ctx,
                                  sqlite3_stmt *handle, bool safe_ints);

v8::Local<v8::Value> GetExpandedRowJS(v8::Isolate *isolate,
                                      v8::Local<v8::Context> ctx,
                                      sqlite3_stmt *handle, bool safe_ints);

v8::Local<v8::Value> GetRawRowJS(v8::Isolate *isolate,
                                 v8::Local<v8::Context> ctx,
                                 sqlite3_stmt *handle, bool safe_ints);

v8::Local<v8::Value> GetRowJS(v8::Isolate *isolate, v8::Local<v8::Context> ctx,
                              sqlite3_stmt *handle, bool safe_ints, char mode);

void GetArgumentsJS(v8::Isolate *isolate, v8::Local<v8::Value> *out,
                    sqlite3_value **values, int argument_count, bool safe_ints);

int BindValueFromJS(v8::Isolate *isolate, sqlite3_stmt *handle, int index,
                    v8::Local<v8::Value> value);

void ResultValueFromJS(v8::Isolate *isolate, sqlite3_context *invocation,
                       v8::Local<v8::Value> value, DataConverter *converter);

} // namespace Data

#endif
