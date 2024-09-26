#ifndef DATA_CONVERTER_H
#define DATA_CONVERTER_H

#include <sqlite3.h>
#include <string>

class DataConverter {
public:
  void ThrowDataConversionError(sqlite3_context *invocation, bool isBigInt);

protected:
  virtual void PropagateJSError(sqlite3_context *invocation) = 0;
  virtual std::string GetDataErrorPrefix() = 0;
};

#endif
