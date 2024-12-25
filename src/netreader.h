#ifndef _NET_READER_H
#define _NET_READER_H

#include "systemc.h"
#include <cinttypes>

#define CONCURRENT_DEVICES_LOG 2
#define MEM_SIZE 2048

SC_MODULE(NetReader) {
  public:
    sc_in<bool> clk_i;
	sc_in<bool> rst_i;
	sc_in_rv<32> data_i;
	sc_in_rv<32> address_i;
	sc_out<uint32_t> data_o;
    sc_out<bool> rd_o;
    sc_out<bool> wr_o;

    sc_in<bool> read_request1, write_request1;
    sc_in<bool> read_request2, write_request2;

    SC_CTOR(NetReader) {
        SC_THREAD(execute);
        sensitive << clk_i.pos();
        dont_initialize();

		m_data[0] = 4;
		m_data[1] = 49;
		m_data[2] = 24;
		for(uint16_t i = 0; i < 24*49; ++i) {
			m_data[3+i] = 0x10101010;
		}

		for(uint16_t i = 0; i < 49; ++i) {
			m_data[MEM_SIZE - 1 - i] = 0xffff;
		}
    };

  private:
    uint32_t m_data[MEM_SIZE];

	void handle_req(sc_bv<CONCURRENT_DEVICES_LOG> selector);
	void handle_write_req();
	void handle_read_req();
	void execute();
};

#endif // _NET_READER_H
