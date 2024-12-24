#ifndef _TESTBENCHES_H
#define _TESTBENCHES_H

#include "systemc.h"

SC_MODULE(NetReaderTest) {
  public:
    sc_in_rv<64> data_i;
    sc_out_rv<64> data_o;
    sc_out_rv<32> address;
	sc_out<bool> rst;

    SC_CTOR(NetReaderTest) { SC_THREAD(_test); }

  private:
    void _test();
};

void netreadertest();

#endif // _TESTBENCHES_H
