#if !defined _TESTCONVERTER_H_
#define _TESTCONVERTER_H_

#include "ConverterModel.h"

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
/// This is the simplest possible Converter, for testing
class TestConverter: public ConverterModel {
 public:
  TestConverter(){};
  virtual ~TestConverter(){};
};

#endif
