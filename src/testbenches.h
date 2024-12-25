#ifndef _TESTBENCHES_H
#define _TESTBENCHES_H

#include "systemc.h"

SC_MODULE(NetReaderTest) {
  public:
    sc_in<uint32_t> data_to_read;
    sc_out_rv<32> data_to_write;
    sc_out_rv<32> address;
	sc_out<bool> rst, wr_req1, rd_req1, wr_req2, rd_req2;

    SC_CTOR(NetReaderTest) { SC_THREAD(write_read_test); }

  private:
    void rst_test();
	void read_test();
	void write_read_test();
};

SC_MODULE(BenchComputingNetReader){
  public:
	sc_out<bool> rst, is_task; 

    SC_CTOR(BenchComputingNetReader) { SC_THREAD(computing_core_readlayer_test); }

  private:
	void computing_core_readlayer_test();

};

void netreadertest();
void netreader_computing_test();

#endif // _TESTBENCHES_H
