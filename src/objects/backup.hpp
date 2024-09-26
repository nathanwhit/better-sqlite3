#ifndef BACKUP_H
#define BACKUP_H

#include <node_object_wrap.h>
#include <sqlite3.h>

class Database;

class Backup : public node::ObjectWrap {
public:
  static v8 ::Local<v8 ::Function> Init(v8 ::Isolate *isolate,
                                        v8 ::Local<v8 ::External> data);

  // Used to support ordered containers.
  static inline bool Compare(Backup const *const a, Backup const *const b) {
    return a->id < b->id;
  }

  // Whenever this is used, db->RemoveBackup must be invoked beforehand.
  void CloseHandles();

  ~Backup();

private:
  explicit Backup(Database *db, sqlite3 *dest_handle,
                  sqlite3_backup *backup_handle, sqlite3_uint64 id,
                  bool unlink);

  static void JS_new(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_transfer(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  static void JS_close(const v8 ::FunctionCallbackInfo<v8 ::Value> &info);

  Database *const db;
  sqlite3 *const dest_handle;
  sqlite3_backup *const backup_handle;
  const sqlite3_uint64 id;
  bool alive;
  bool unlink;
};

#endif
