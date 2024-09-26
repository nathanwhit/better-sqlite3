#include "statement-iterator.hpp"
#include "../util/binder.hpp"
#include "../util/data.hpp"
#include "../util/query-macros.hpp"

v8 ::Local<v8 ::Function>
StatementIterator::Init(v8 ::Isolate *isolate, v8 ::Local<v8 ::External> data) {
  v8::Local<v8::FunctionTemplate> t =
      NewConstructorTemplate(isolate, data, JS_new, "StatementIterator");
  SetPrototypeMethod(isolate, data, t, "next", JS_next);
  SetPrototypeMethod(isolate, data, t, "return", JS_return);
  SetPrototypeSymbolMethod(isolate, data, t, v8::Symbol::GetIterator(isolate),
                           JS_symbolIterator);
  return t->GetFunction(OnlyContext).ToLocalChecked();
}
StatementIterator::StatementIterator(Statement *stmt, bool bound)
    : node::ObjectWrap(), stmt(stmt), handle(stmt->handle),
      db_state(stmt->db->GetState()), bound(bound), safe_ints(stmt->safe_ints),
      mode(stmt->mode), alive(true), logged(!db_state->has_logger) {
  assert(stmt != NULL);
  assert(handle != NULL);
  assert(stmt->bound == bound);
  assert(stmt->alive == true);
  assert(stmt->locked == false);
  assert(db_state->iterators < USHRT_MAX);
  stmt->locked = true;
  db_state->iterators += 1;
}
void StatementIterator::JS_new(
    const v8 ::FunctionCallbackInfo<v8 ::Value> &info) {
  UseAddon;
  if (!addon->privileged_info)
    return ThrowTypeError("Disabled constructor");
  assert(info.IsConstructCall());

  StatementIterator *iter;
  {
    NODE_ARGUMENTS info = *addon->privileged_info;
    STATEMENT_START_LOGIC(REQUIRE_STATEMENT_RETURNS_DATA, DOES_ADD_ITERATOR);
    iter = new StatementIterator(stmt, bound);
  }
  UseIsolate;
  UseContext;
  iter->Wrap(info.This());
  SetFrozen(isolate, ctx, info.This(), addon->cs.statement,
            addon->privileged_info->This());

  info.GetReturnValue().Set(info.This());
}
void StatementIterator::JS_next(
    const v8 ::FunctionCallbackInfo<v8 ::Value> &info) {
  StatementIterator *iter = Unwrap<StatementIterator>(info.This());
  REQUIRE_DATABASE_NOT_BUSY(iter->db_state);
  if (iter->alive)
    iter->Next(info);
  else
    info.GetReturnValue().Set(DoneRecord(OnlyIsolate, iter->db_state->addon));
}
void StatementIterator::JS_return(
    const v8 ::FunctionCallbackInfo<v8 ::Value> &info) {
  StatementIterator *iter = Unwrap<StatementIterator>(info.This());
  REQUIRE_DATABASE_NOT_BUSY(iter->db_state);
  if (iter->alive)
    iter->Return(info);
  else
    info.GetReturnValue().Set(DoneRecord(OnlyIsolate, iter->db_state->addon));
}
void StatementIterator::JS_symbolIterator(
    const v8 ::FunctionCallbackInfo<v8 ::Value> &info) {
  info.GetReturnValue().Set(info.This());
}
void StatementIterator::Next(NODE_ARGUMENTS info) {
  assert(alive == true);
  db_state->busy = true;
  if (!logged) {
    logged = true;
    if (stmt->db->Log(OnlyIsolate, handle)) {
      db_state->busy = false;
      Throw();
      return;
    }
  }
  int status = sqlite3_step(handle);
  db_state->busy = false;
  if (status == SQLITE_ROW) {
    UseIsolate;
    UseContext;
    info.GetReturnValue().Set(NewRecord(
        isolate, ctx, Data::GetRowJS(isolate, ctx, handle, safe_ints, mode),
        db_state->addon, false));
  } else {
    if (status == SQLITE_DONE)
      Return(info);
    else
      Throw();
  }
}

void StatementIterator::Return(NODE_ARGUMENTS info) {
  Cleanup();
  STATEMENT_RETURN_LOGIC(DoneRecord(OnlyIsolate, db_state->addon));
}

void StatementIterator::Throw() {
  Cleanup();
  Database *db = stmt->db;
  STATEMENT_THROW_LOGIC();
}

void StatementIterator::Cleanup() {
  assert(alive == true);
  alive = false;
  stmt->locked = false;
  db_state->iterators -= 1;
  sqlite3_reset(handle);
}

v8::Local<v8::Object> StatementIterator::NewRecord(v8::Isolate *isolate,
                                                   v8::Local<v8::Context> ctx,
                                                   v8::Local<v8::Value> value,
                                                   Addon *addon, bool done) {
  v8::Local<v8::Object> record = v8::Object::New(isolate);
  record->Set(ctx, addon->cs.value.Get(isolate), value).FromJust();
  record->Set(ctx, addon->cs.done.Get(isolate), v8::Boolean::New(isolate, done))
      .FromJust();
  return record;
}
v8::Local<v8::Object> StatementIterator::DoneRecord(v8::Isolate *isolate,
                                                    Addon *addon) {
  return NewRecord(isolate, OnlyContext, v8::Undefined(isolate), addon, true);
}
