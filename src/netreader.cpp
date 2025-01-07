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
			data_o.write(0);
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
	rd_o.write(true);
	uint64_t rd_addr = address_i.read().to_uint64();
	data_o.write(rd_addr < MEM_SIZE ? m_data[rd_addr] : 0);
	wait();
	rd_o.write(false);
	data_o.write(0);
}

void NetReader::handle_write_req() {
	wr_o.write(true);
	uint64_t rd_addr = address_i.read().to_uint64();
	uint64_t data = data_i.read().to_uint64();

	if(rd_addr < MEM_SIZE) {
		m_data[rd_addr] = data;
	}
	wait();
	wr_o.write(false);
}

int8_t NetReader::write_net_to_mem(const NNetwork* const net) {
	uint32_t* mem =  (uint32_t*) m_data;
	mem[0] = (uint32_t) net->layers_sizes.size();
	mem[1] = (uint32_t) net->layers_sizes[0];
	mem[2] = (uint32_t) net->layers_sizes[1];

	size_t offset = 3;
	size_t forward_layer = 2;

	for(size_t i = 0; i < net->layers_sizes.size() - 1; ++i) {
		for(size_t j = 0; j < net->layers[i].size(); ++j) {
			float floated = (float) net->layers[i][j];
			memcpy(mem + offset, &floated, sizeof(float));
			offset++;
		}
		mem[offset] = (uint32_t) net->layers_sizes[forward_layer];
		forward_layer++;
		offset++;
	}
	return 1;
}
