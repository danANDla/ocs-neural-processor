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

std::vector<std::uint8_t> read_image_from_file(const std::string fname) {
	std::ifstream image;
	image.open(fname);

	std::vector<uint8_t> pixels(49);
	for(size_t i = 0; i < 49; ++i) {
		image >> pixels[i];
	}
	return pixels;
}
