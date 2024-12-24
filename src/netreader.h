#ifndef _NET_READER_H
#define _NET_READER_H

#include "systemc.h"
#include <cinttypes>

#define CONCURRENT_DEVICES_LOG 2
#define MEM_SIZE 100

SC_MODULE(NetReader) {
  public:
    sc_in<bool> clk_i;
	sc_in<bool> rst_i;
	sc_in_rv<64> data_i;
	sc_in_rv<32> address_i;
	sc_out_rv<64> data_o;
    sc_out<bool> rd_o;
    sc_out<bool> wr_o;

    sc_in<bool> read_request1, write_request1;
    sc_in<bool> read_request2, write_request2;

    SC_CTOR(NetReader) {
        SC_THREAD(execute);
        sensitive << clk_i.pos();
        dont_initialize();

		for(uint64_t& d: m_data) {
			d = 0x1010101010101010;
		}
    };

  private:
    uint64_t m_data[MEM_SIZE];

	void handle_req(sc_bv<CONCURRENT_DEVICES_LOG> selector);
	void handle_write_req();
	void handle_read_req();
	void execute();
};

#endif // _NET_READER_H
