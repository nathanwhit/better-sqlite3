#include "objects/addon.hpp"
#include "objects/statement-iterator.hpp"
#include <node.h>

NODE_MODULE_INIT(/* exports, context */) {
  v8::Isolate *isolate = context->GetIsolate();
  v8::HandleScope scope(isolate);

  // Initialize addon instance.
  Addon *addon = new Addon(isolate);
  v8::Local<v8::External> data = v8::External::New(isolate, addon);
  node::AddEnvironmentCleanupHook(isolate, Addon::Cleanup, addon);

  // Create and export native-backed classes and functions.
  exports
      ->Set(context, InternalizedFromLatin1(isolate, "Database"),
            Database::Init(isolate, data))
      .FromJust();
  exports
      ->Set(context, InternalizedFromLatin1(isolate, "Statement"),
            Statement::Init(isolate, data))
      .FromJust();
  exports
      ->Set(context, InternalizedFromLatin1(isolate, "StatementIterator"),
            StatementIterator::Init(isolate, data))
      .FromJust();
  exports
      ->Set(context, InternalizedFromLatin1(isolate, "Backup"),
            Backup::Init(isolate, data))
      .FromJust();
  exports
      ->Set(context, InternalizedFromLatin1(isolate, "setErrorConstructor"),
            v8::FunctionTemplate::New(isolate, Addon::JS_setErrorConstructor,
                                      data)
                ->GetFunction(context)
                .ToLocalChecked())
      .FromJust();

  // Store addon instance data.
  addon->Statement.Reset(
      isolate,
      exports->Get(context, InternalizedFromLatin1(isolate, "Statement"))
          .ToLocalChecked()
          .As<v8::Function>());
  addon->StatementIterator.Reset(
      isolate,
      exports
          ->Get(context, InternalizedFromLatin1(isolate, "StatementIterator"))
          .ToLocalChecked()
          .As<v8::Function>());
  addon->Backup.Reset(
      isolate, exports->Get(context, InternalizedFromLatin1(isolate, "Backup"))
                   .ToLocalChecked()
                   .As<v8::Function>());
}
