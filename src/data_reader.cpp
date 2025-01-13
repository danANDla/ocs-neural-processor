#include "data_reader.hpp"

#include <fstream>
#include <iostream>
#include <cinttypes>

DataSet read_train_set_from_file(const std::string fname) {
	std::ifstream train;
	train.open(fname);
	DataSet r;
	size_t inputs_number, outputs_number, sz;
	train >> inputs_number >> outputs_number >> sz;

	for(size_t i = 0; i < sz; ++i) {
		std::vector<double> inputs(inputs_number);
		for(size_t j = 0; j < inputs_number; ++j) {
			train >> inputs[j];
		}
		std::vector<double> outputs(outputs_number);
		for(size_t j = 0; j < outputs_number; ++j) {
			train >> outputs[j];
		}
		r.inputs.push_back(inputs);
		r.outputs.push_back(outputs);
	}
	return r;
}

NNetwork read_network_from_file(const std::string fname) {
	std::ifstream train;
	train.open(fname);
	NNetwork r;
	size_t layers_n;
	train >> layers_n;
	r.layers_sizes = std::vector<std::size_t>(layers_n);
	for(size_t i = 0; i < layers_n; ++i) {
		train >> r.layers_sizes[i];
	}

	r.layers = std::vector<std::vector<double>> (layers_n - 1);
	for(size_t i = 0; i < layers_n - 1; ++i) {
		r.layers[i] = std::vector<double>(r.layers_sizes[i] * r.layers_sizes[i+1]);
		for(size_t j = 0; j < r.layers[i].size(); ++j) {
			train >> r.layers[i][j];		
		}
	}
	return r;
}

int8_t write_net_to_mem(void* const _mem, const NNetwork* const net) {
	uint32_t* mem =  (uint32_t*) _mem;
	mem[0] = (uint32_t) net->layers_sizes.size();
	mem[1] = (uint32_t) net->layers_sizes[0];
	mem[2] = (uint32_t) net->layers_sizes[1];

	size_t offset = 3;
	size_t forward_layer = 2;

	for(size_t i = 0; i < net->layers_sizes.size() - 1; ++i) {
		for(size_t j = 0; j < net->layers[i].size(); ++j) {
			mem[offset] = net->layers[i][j];
			offset++;
		}
		mem[offset] = (uint32_t) net->layers_sizes[forward_layer];
		forward_layer++;
		offset++;
	}

	return 1;
}

int8_t write_image_to_mem(void* const _mem, const std::string fname, const size_t first_addr) {
	uint32_t* mem =  (uint32_t*) _mem;
	size_t offset = first_addr;

	std::ifstream image;
	image.open(fname);

	for(size_t i = 0; i < 49; ++i) {
		uint8_t pixel;
		image >> pixel;
		mem[offset] = (uint32_t) pixel;
		offset--;
	}
	return 1;
}
