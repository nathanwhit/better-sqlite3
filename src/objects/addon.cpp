#include "addon.hpp"
void Addon::Cleanup(void *ptr) {
  Addon *addon = static_cast<Addon *>(ptr);
  for (Database *db : addon->dbs)
    db->CloseHandles();
  addon->dbs.clear();
  delete addon;
}

void Addon::JS_setErrorConstructor(
    const v8 ::FunctionCallbackInfo<v8 ::Value> &info) {
  REQUIRE_ARGUMENT_FUNCTION(FIRST_ARG, v8::Local<v8::Function> SqliteError);
  OnlyAddon->SqliteError.Reset(OnlyIsolate, SqliteError);
}
