#include "netreader.h"
#include <iostream>

void NetReader::poll_requests() {
	is_your_discrete[discrete].write(true);
	for(uint16_t i = 0; i < 3; ++i) {
		if(read_reqs[discrete].read()) {
			handle_read_req();

			is_your_discrete[discrete].write(false);
			return;
		} else if(write_reqs[discrete].read()) {
			handle_write_req();
			
			is_your_discrete[discrete].write(false);
			return;
		}
		wait();
	}
	is_your_discrete[discrete].write(false);
}

void NetReader::execute() {
	while(true) {
		if(rst_i.read()) {
			rd_o.write(false);
			wr_o.write(false);
			data_o.write(0);
			discrete = 0;
			for(uint16_t i = 0; i < CORE_NUMBER + 1; ++i) {
				is_your_discrete[i].write(false);
			}
		} else {
			if(!(rd_o.read() || wr_o.read())) {
				poll_requests();
				discrete++;
				if(discrete == CORE_NUMBER + 1) discrete = 0;
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

int8_t NetReader::write_image_to_mem(std::vector<uint8_t>& pixels) {
	uint32_t* mem =  (uint32_t*) m_data;
	size_t offset = MEM_SIZE - 1;

	for(uint8_t p: pixels) {
		*((float*)(mem + offset)) = float(p - '0');
		offset--;
	}
	return 1;
}
