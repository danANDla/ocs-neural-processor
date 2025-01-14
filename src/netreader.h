#ifndef _NET_READER_H
#define _NET_READER_H

#include "data_reader.hpp"
#include "systemc.h"
#include <cinttypes>

#define CORE_NUMBER 2

#define MEM_SIZE 1024*2 

SC_MODULE(NetReader) {
  public:
    sc_in<bool> clk_i;
	sc_in<bool> rst_i;
	sc_in_rv<32> data_i;
	sc_in_rv<32> address_i;
	sc_out<uint32_t> data_o;

    sc_out<bool> rd_o;
    sc_out<bool> wr_o;

	sc_in<bool> read_reqs[CORE_NUMBER + 1];
	sc_in<bool> write_reqs[CORE_NUMBER + 1];
	sc_out<bool> is_your_discrete[CORE_NUMBER + 1];

    SC_CTOR(NetReader) {
        SC_THREAD(execute);
        sensitive << clk_i.pos();
        dont_initialize();

		for(uint16_t i = 0; i < 49; ++i) {
			m_data[MEM_SIZE - 1 - i] = 0xffff;
		}
    };

	int8_t write_net_to_mem(const NNetwork* const net);
	int8_t write_image_to_mem(std::vector<uint8_t> & pixels);

  private:
	void handle_write_req();
	void handle_read_req();
	void execute();
	void poll_requests();

	uint16_t discrete;
    uint32_t m_data[MEM_SIZE];
};


#endif // _NET_READER_H
