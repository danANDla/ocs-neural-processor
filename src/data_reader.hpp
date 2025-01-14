#pragma once

#include <vector>
#include <string>
#include <cinttypes>

typedef struct {
	std::vector<std::vector<double>> inputs;
	std::vector<std::vector<double>> outputs;
} DataSet;

typedef struct {
	std::vector<std::size_t> layers_sizes;
	std::vector<std::vector<double>> layers;
} NNetwork;

DataSet read_train_set_from_file(const std::string fname);
NNetwork read_network_from_file(const std::string fname);
int8_t write_net_to_mem(void* const _mem, const NNetwork* const net);
std::vector<std::uint8_t> read_image_from_file(const std::string fname);
