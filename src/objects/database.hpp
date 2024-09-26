#ifndef DATABASE_H
#define DATABASE_H

#include "backup.hpp"
#include "statement.hpp"
#include <node_object_wrap.h>
#include <set>
#include <sqlite3.h>
struct Addon;
class Statement;
class Backup;
class Database : public node::ObjectWrap {
public:
  static v8 ::Local<v8 ::Function> Init(v8 ::Isolate *isolate,
                                        v8 ::Local<v8 ::External> data);

  // Used to support ordered containers.
  class CompareDatabase {
  public:
    bool operator()(Database const *const a, Database const *const b) const;
  };
  class CompareStatement {
  public:
    bool operator()(Statement const *const a, Statement const *const b) const;
  };
  class CompareBackup {
  public:
    bool operator()(Backup const *const a, Backup const *const b) const;
  };

  // Proper error handling logic for when an sqlite3 operation fails.
  void ThrowDatabaseError();
  static void ThrowSqliteError(Addon *addon, sqlite3 *db_handle);
  static void ThrowSqliteError(Addon *addon, const char *message, int code);

  // Allows Statements to log their executed SQL.
  bool Log(v8::Isolate *isolate, sqlite3_stmt *handle);

  // Allow Statements to manage themselves when created and garbage collected.
  inline void AddStatement(Statement *stmt) { stmts.insert(stmts.end(), stmt); }
  inline void RemoveStatement(Statement *stmt) { stmts.erase(stmt); }

  // Allow Backups to manage themselves when created and garbage collected.
  inline void AddBackup(Backup *backup) {
    backups.insert(backups.end(), backup);
  }
  inline void RemoveBackup(Backup *backup) { backups.erase(backup); }

  // A view for Statements to see and modify Database state.
  // The order of these fields must exactly match their actual order.
  struct State {
    const bool open;
    bool busy;
    const bool safe_ints;
    const bool unsafe_mode;
    bool was_js_error;
    const bool has_logger;
    unsigned short iterators;
    Addon *const addon;
  };
  inline State *GetState() { return reinterpret_cast<State *>(&open); }
  inline sqlite3 *GetHandle() { return db_handle; }
  inline Addon *GetAddon() { return addon; }

  // Whenever this is used, addon->dbs.erase() must be invoked beforehand.
  void CloseHandles() {
    if (open) {
      open = false;
      for (Statement *stmt : stmts)
        stmt->CloseHandles();
      for (Backup *backup : backups)
        backup->CloseHandles();
      stmts.clear();
      backups.clear();
      int status = sqlite3_close(db_handle);
      assert(status == SQLITE_OK);
      ((void)status);
    }
  }

  ~Database();

private:
  explicit Database(v8::Isolate *isolate, Addon *addon, sqlite3 *db_handle,
                    v8::Local<v8::Value> logger);

  static void JS_new(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_prepare(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_exec(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_backup(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_serialize(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_function(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_aggregate(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_table(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void
  JS_loadExtension(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_close(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void
  JS_defaultSafeIntegers(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_unsafeMode(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_open(v8 ::Local<v8 ::Name> _,
                      const v8 ::PropertyCallbackInfo<v8 ::Value> &info);

  static void
  JS_inTransaction(v8 ::Local<v8 ::Name> _,
                   const v8 ::PropertyCallbackInfo<v8 ::Value> &info);

  static bool Deserialize(v8::Local<v8::Object> buffer, Addon *addon,
                          sqlite3 *db_handle, bool readonly);

  static void FreeSerialization(char *data, void *_);

  static const int
      MAX_BUFFER_SIZE = node::Buffer::kMaxLength > INT_MAX
                            ? INT_MAX
                            : static_cast<int>(node::Buffer::kMaxLength);
  static const int
      MAX_STRING_SIZE = v8::String::kMaxLength > INT_MAX
                            ? INT_MAX
                            : static_cast<int>(v8::String::kMaxLength);

  sqlite3 *const db_handle;
  bool open;
  bool busy;
  bool safe_ints;
  bool unsafe_mode;
  bool was_js_error;
  const bool has_logger;
  unsigned short iterators;
  Addon *const addon;
  const v8::Global<v8::Value> logger;
  std::set<Statement *, CompareStatement> stmts;
  std::set<Backup *, CompareBackup> backups;
};

#endif
