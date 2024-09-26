#ifndef CUSTOM_TABLE_H
#define CUSTOM_TABLE_H

#include "data-converter.hpp"
#include "v8-isolate.h"
#include <sqlite3.h>

struct Addon;

class Database;

class CustomTable {
private:
  // Although this function does nothing, we cannot use xConnect directly,
  // because that would cause SQLite to register an eponymous virtual table.
  static int xCreate(sqlite3 *db_handle, void *_self, int argc,
                     const char *const *argv, sqlite3_vtab **output,
                     char **errOutput);
  // This method uses the factory function to instantiate a new virtual table.
  static int xConnect(sqlite3 *db_handle, void *_self, int argc,
                      const char *const *argv, sqlite3_vtab **output,
                      char **errOutput);

  static int xDisconnect(sqlite3_vtab *vtab);

  static int xOpen(sqlite3_vtab *vtab, sqlite3_vtab_cursor **output);

  static int xClose(sqlite3_vtab_cursor *cursor);

  // This method uses a fresh cursor to start a new scan of a virtual table.
  // The args and idxNum are provided by xBestIndex (idxStr is unused).
  // idxNum is a bitmap that provides the proper indices of the received args.
  static int xFilter(sqlite3_vtab_cursor *_cursor, int idxNum,
                     const char *idxStr, int argc, sqlite3_value **argv);

  // This method advances a virtual table's cursor to the next row.
  // SQLite will call this method repeatedly, driving the generator function.
  static int xNext(sqlite3_vtab_cursor *_cursor);

  // If this method returns 1, SQLite will stop scanning the virtual table.
  static int xEof(sqlite3_vtab_cursor *cursor);

  // This method extracts some column from the cursor's current row.
  static int xColumn(sqlite3_vtab_cursor *_cursor, sqlite3_context *invocation,
                     int column);

  // This method outputs the rowid of the cursor's current row.
  static int xRowid(sqlite3_vtab_cursor *cursor, sqlite_int64 *output);

  // This method tells SQLite how to *plan* queries on our virtual table.
  // It gets invoked (typically multiple times) during db.prepare().
  static int xBestIndex(sqlite3_vtab *vtab, sqlite3_index_info *output);

public:
  explicit CustomTable(v8::Isolate *isolate, Database *db, const char *name,
                       v8::Local<v8::Function> factory);

  static void Destructor(void *self) {
    delete static_cast<CustomTable *>(self);
  }

  static sqlite3_module MODULE;

  static sqlite3_module EPONYMOUS_MODULE;

private:
  // This nested class is instantiated on each CREATE VIRTUAL TABLE statement.
  class VTab {
    friend class CustomTable;
    explicit VTab(CustomTable *parent, v8::Local<v8::Function> generator,
                  std::vector<std::string> parameter_names, bool safe_ints);

    static inline CustomTable::VTab *Upcast(sqlite3_vtab *vtab) {
      return reinterpret_cast<VTab *>(vtab);
    }

    inline sqlite3_vtab *Downcast() {
      return reinterpret_cast<sqlite3_vtab *>(this);
    }

    sqlite3_vtab base;
    CustomTable *const parent;
    const int parameter_count;
    const bool safe_ints;
    const v8::Global<v8::Function> generator;
    const std::vector<std::string> parameter_names;
  };

  // This nested class is instantiated each time a virtual table is scanned.
  class Cursor {
    friend class CustomTable;
    static inline CustomTable::Cursor *Upcast(sqlite3_vtab_cursor *cursor) {
      return reinterpret_cast<Cursor *>(cursor);
    }

    inline sqlite3_vtab_cursor *Downcast() {
      return reinterpret_cast<sqlite3_vtab_cursor *>(this);
    }

    inline CustomTable::VTab *GetVTab() { return VTab::Upcast(base.pVtab); }

    sqlite3_vtab_cursor base;
    v8::Global<v8::Object> iterator;
    v8::Global<v8::Function> next;
    v8::Global<v8::Array> row;
    bool done;
    sqlite_int64 rowid;
  };

  // This nested class is used by Data::ResultValueFromJS to report errors.
  class TempDataConverter : DataConverter {
    friend class CustomTable;
    explicit TempDataConverter(CustomTable *parent)
        : parent(parent), status(SQLITE_OK) {}

    void PropagateJSError(sqlite3_context *invocation);

    std::string GetDataErrorPrefix() {
      return std::string("Virtual table module \"") + parent->name +
             "\" yielded";
    }

    CustomTable *const parent;
    int status;
  };

  void PropagateJSError();

  Addon *const addon;
  v8::Isolate *const isolate;
  Database *const db;
  const std::string name;
  const v8::Global<v8::Function> factory;
};

#endif
