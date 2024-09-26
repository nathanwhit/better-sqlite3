#include "data.hpp"
#include "macros.hpp"
#include <node_buffer.h>
void Data::ResultValueFromJS(v8::Isolate *isolate, sqlite3_context *invocation,
                             v8::Local<v8::Value> value,
                             DataConverter *converter) {
  JS_VALUE_TO_SQLITE(result, value, isolate, invocation);
  converter->ThrowDataConversionError(invocation, value->IsBigInt());
}

int Data::BindValueFromJS(v8::Isolate *isolate, sqlite3_stmt *handle, int index,
                          v8::Local<v8::Value> value) {
  JS_VALUE_TO_SQLITE(bind, value, isolate, handle, index);
  return value->IsBigInt() ? SQLITE_TOOBIG : -1;
}

void Data::GetArgumentsJS(v8::Isolate *isolate, v8::Local<v8::Value> *out,
                          sqlite3_value **values, int argument_count,
                          bool safe_ints) {
  assert(argument_count > 0);
  for (int i = 0; i < argument_count; ++i) {
    out[i] = Data::GetValueJS(isolate, values[i], safe_ints);
  }
}

v8::Local<v8::Value> Data::GetRowJS(v8::Isolate *isolate,
                                    v8::Local<v8::Context> ctx,
                                    sqlite3_stmt *handle, bool safe_ints,
                                    char mode) {
  if (mode == FLAT)
    return GetFlatRowJS(isolate, ctx, handle, safe_ints);
  if (mode == PLUCK)
    return GetValueJS(isolate, handle, 0, safe_ints);
  if (mode == EXPAND)
    return GetExpandedRowJS(isolate, ctx, handle, safe_ints);
  if (mode == RAW)
    return GetRawRowJS(isolate, ctx, handle, safe_ints);
  assert(false);
  return v8::Local<v8::Value>();
}

v8::Local<v8::Value> Data::GetRawRowJS(v8::Isolate *isolate,
                                       v8::Local<v8::Context> ctx,
                                       sqlite3_stmt *handle, bool safe_ints) {
  v8::Local<v8::Array> row = v8::Array::New(isolate);
  int column_count = sqlite3_column_count(handle);
  for (int i = 0; i < column_count; ++i) {
    row->Set(ctx, i, Data::GetValueJS(isolate, handle, i, safe_ints))
        .FromJust();
  }
  return row;
}

v8::Local<v8::Value> Data::GetExpandedRowJS(v8::Isolate *isolate,
                                            v8::Local<v8::Context> ctx,
                                            sqlite3_stmt *handle,
                                            bool safe_ints) {
  v8::Local<v8::Object> row = v8::Object::New(isolate);
  int column_count = sqlite3_column_count(handle);
  for (int i = 0; i < column_count; ++i) {
    const char *table_raw = sqlite3_column_table_name(handle, i);
    v8::Local<v8::String> table =
        InternalizedFromUtf8(isolate, table_raw == NULL ? "$" : table_raw, -1);
    v8::Local<v8::String> column =
        InternalizedFromUtf8(isolate, sqlite3_column_name(handle, i), -1);
    v8::Local<v8::Value> value =
        Data::GetValueJS(isolate, handle, i, safe_ints);
    if (row->HasOwnProperty(ctx, table).FromJust()) {
      row->Get(ctx, table)
          .ToLocalChecked()
          .As<v8::Object>()
          ->Set(ctx, column, value)
          .FromJust();
    } else {
      v8::Local<v8::Object> nested = v8::Object::New(isolate);
      row->Set(ctx, table, nested).FromJust();
      nested->Set(ctx, column, value).FromJust();
    }
  }
  return row;
}

v8::Local<v8::Value> Data::GetFlatRowJS(v8::Isolate *isolate,
                                        v8::Local<v8::Context> ctx,
                                        sqlite3_stmt *handle, bool safe_ints) {
  v8::Local<v8::Object> row = v8::Object::New(isolate);
  int column_count = sqlite3_column_count(handle);
  for (int i = 0; i < column_count; ++i) {
    row->Set(ctx,
             InternalizedFromUtf8(isolate, sqlite3_column_name(handle, i), -1),
             Data::GetValueJS(isolate, handle, i, safe_ints))
        .FromJust();
  }
  return row;
}

v8::Local<v8::Value> Data::GetValueJS(v8::Isolate *isolate,
                                      sqlite3_value *value, bool safe_ints) {
  SQLITE_VALUE_TO_JS(value, isolate, safe_ints, value);
}

v8::Local<v8::Value> Data::GetValueJS(v8::Isolate *isolate,
                                      sqlite3_stmt *handle, int column,
                                      bool safe_ints) {
  SQLITE_VALUE_TO_JS(column, isolate, safe_ints, handle, column);
}
