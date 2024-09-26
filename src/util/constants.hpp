#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <v8.h>

class CS {
public:
  v8::Local<v8::String> Code(v8::Isolate *isolate, int code);

  explicit CS(v8::Isolate *isolate);

  v8::Global<v8::String> database;
  v8::Global<v8::String> reader;
  v8::Global<v8::String> source;
  v8::Global<v8::String> memory;
  v8::Global<v8::String> readonly;
  v8::Global<v8::String> name;
  v8::Global<v8::String> next;
  v8::Global<v8::String> length;
  v8::Global<v8::String> done;
  v8::Global<v8::String> value;
  v8::Global<v8::String> changes;
  v8::Global<v8::String> lastInsertRowid;
  v8::Global<v8::String> statement;
  v8::Global<v8::String> column;
  v8::Global<v8::String> table;
  v8::Global<v8::String> type;
  v8::Global<v8::String> totalPages;
  v8::Global<v8::String> remainingPages;

private:
  static void SetString(v8::Isolate *isolate, v8::Global<v8::String> &constant,
                        const char *str);

  void SetCode(v8::Isolate *isolate, int code, const char *str);

  std::unordered_map<int, v8::Global<v8::String>> codes;
};

#endif
