#include "netreader.h"
#include <iostream>

void NetReader::handle_req(sc_bv<CONCURRENT_DEVICES_LOG> selector) {
	uint sel = selector.to_uint();
	if(sel == 1) {
		if(read_request1.read()) {
			handle_read_req();
		} else if (write_request1.read()) {
			handle_write_req();
		}
	}
}

void NetReader::execute() {
	while(true) {
		if(rst_i.read()) {
			rd_o.write(false);
			wr_o.write(false);
			data_o.write(sc_lv<32>('Z'));
		} else {
			if(!(rd_o.read() || wr_o.read())) {
				if(read_request1.read() || write_request1.read()) {
					handle_req(1);
				}
			}
		}
		wait();
	}
}

void NetReader::handle_read_req() {
	uint64_t rd_addr = address_i.read().to_uint64();
	data_o.write(rd_addr < MEM_SIZE ? m_data[rd_addr] : 0);
	wait();
	data_o.write(sc_lv<32>('Z'));
}

void NetReader::handle_write_req() {
	
}
