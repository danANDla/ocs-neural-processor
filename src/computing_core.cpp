#include "computing_core.h"

ALU_linear::ALU_linear(sc_module_name mn)
    : sc_module(mn), clk_i("clk_i"), 
	  rst_i("is_task_i"),
	  data_i("data_i"),
	  weight_i("w_i"),
	  neuron_i("n_i"),
	  res_o("reso_o"),
	  ready_o("ready_o")
{
	SC_THREAD(execute);
	sensitive << clk_i.pos();
}


void ALU_linear::execute() {
	while(true) {
		if(rst_i.read()) {
			std::cout << "reseting alu\n";
			res_o.write(0xffff);	
			ready_o.write(true);	
			sum = 0;
		} else {
			if(data_i.read()) {
				weight = weight_i.read();
				neuron = neuron_i.read();
				ready_o.write(false);;
				calculate();
			}
		}
		wait();
	}
}

/**
 * @function line_func_add(float a, float b, float c)
 *
 * @param sum Взвешенная сумма значений нейронов, которые обработаны
 * @param w Вес ребра следующего нейрона
 * @param n_v Значние следующего нейрона
 *
 * @returns a + b*c
 */
void ALU_linear::calculate() {
	sum = sum + weight * neuron;
	wait();
	wait();
	res_o = sum;
	ready_o.write(true);
}

ComputingCore::ComputingCore(sc_module_name mn)
    : sc_module(mn), clk_i("clk_i"), 
	  is_task_i("is_task_i"),
	  this_neuron_cords_i("this_neuron_cords_i"),
      prev_layer_address_i("prev_layer_address_i"),
	  res_address_i("res_address_i"),
	  alu("alu")
{
	
	SC_THREAD(fsm_controller);
	sensitive << clk_i.pos();

	alu.clk_i(clk_i);
	alu.rst_i(rst_i);
	alu.data_i(alu_req);
	alu.weight_i(alu_weight);
	alu.neuron_i(alu_neuron);
	alu.res_o(alu_res);
	alu.ready_o(alu_ready);
}

int8_t ComputingCore::get_layer_and_neurons_n(uint32_t& layer, uint32_t& n) {
	return 1;
}

void ComputingCore::read_ram_word(uint32_t addr, uint32_t& word) {
	while(ram_rd_i.read() || ram_wr_i.read()) {
		wait();
	}
	ram_addr.write(addr);
	ram_read_req.write(true);
	ram_write_req.write(false);
	while(!ram_rd_i.read()) {
		wait();
	}
	word = ram_data_read.read();
	
	ram_addr.write(sc_lv<32>('Z'));
	ram_read_req.write(false);
	ram_write_req.write(false);
}

void ComputingCore::read_ram_layer() {
	uint32_t layer = this_neuron_cords >> 16;
	uint32_t this_layer_neuron_id = this_neuron_cords & 0x0000ffff;
	
	uint32_t offset = 3;
	uint32_t prev_n, this_n;
	read_ram_word(1, prev_n); // layer 0
	read_ram_word(2, this_n); // layer 1
	for(uint32_t i = 1; i < layer; ++i) {
		std::cout <<" k " << "\n";
		offset += this_n * prev_n;
		prev_n = this_n;
		read_ram_word(offset, this_n);
	}

	printf("layer %u, neuron_id %u, prev_layer_addr %u, neurons_in_this %u, neurons_in_prev %u\n", layer, this_layer_neuron_id, prev_layer_address, this_n, prev_n);

	*((uint32_t*)local_mem) = prev_n;
	uint32_t local_offset = 4;

	for(uint32_t i = 0; i < prev_n; ++i) {
		uint32_t edge;
		uint32_t neuron;
		read_ram_word(offset + prev_n * this_layer_neuron_id + i, edge);
		read_ram_word(prev_layer_address_i - i, neuron);

		*((uint32_t*) ((uint8_t*) local_mem + local_offset)) = edge;
		local_offset += 4;
		*((uint32_t*) ((uint8_t*) local_mem + local_offset)) = neuron;
		local_offset += 4;

		float floated_edge = (float) *((float*)&edge);
		float floated_neuron = (float) *((float*)&neuron);
		std::cout << "read edge: " << floated_edge << " and neuron: " << floated_neuron << "\n";
	}

	rd_completed.write(true);
}

void ComputingCore::compute_sum() {
	uint32_t prev_n, offset;

	prev_n = *((uint32_t*)local_mem);
	offset = 4;

	for(uint32_t i = 0; i < prev_n; ++i) {
		uint32_t edge =	*((uint32_t*) ((uint8_t*) local_mem + offset)) ;
		offset += 4;
		uint32_t neuron = *((uint32_t*) ((uint8_t*) local_mem + offset)) ;
		offset += 4;

		float floated_edge = (float) *((float*)&edge);
		float floated_neuron = (float) *((float*)&neuron);

		while(alu_ready.read() != true) wait();
		alu_neuron.write(floated_neuron);
		alu_weight.write(floated_edge);
		alu_req.write(true);
		while(alu_ready.read() != false) wait();
		alu_req.write(false);
		while(alu_ready.read() != true) wait();

	}
	activation = alu_res.read();
	sum_completed.write(true);
}

void ComputingCore::compute_activation() {
	activation = 1 / (1 + exp(-activation));
	activation_completed.write(true);
}

void ComputingCore::write_ram_result() {

	while(ram_rd_i.read() || ram_wr_i.read()) {
		wait();
	}

	ram_addr.write(result_address);
	uint32_t uinted_activation = *((uint32_t*) &activation);
	ram_data_write.write(uinted_activation);
	ram_read_req.write(false);
	ram_write_req.write(true);
	while(!ram_wr_i.read()) {
		wait();
	}
	ram_addr.write(sc_lv<32>('Z'));
	ram_data_write.write(sc_lv<32>('Z'));
	ram_read_req.write(false);
	ram_write_req.write(false);


	wr_completed.write(true);
}

void ComputingCore::fsm_controller() {
	while(true) {
		if(rst_i.read()) {
			std::cout << "reseting computing\n";
			ram_addr.write(sc_lv<32>('Z'));
			ram_data_write.write(sc_lv<64>('Z'));
			alu_req.write(false);
		}

		switch (state) {
			case IDLE:
				if(is_task_i) {
					std::cout<< "got task\n";
					this_neuron_cords = this_neuron_cords_i;
					prev_layer_address = prev_layer_address_i;
					result_address = res_address_i;
					rd_completed.write(false);
					state = READING;
					read_ram_layer();
				}
				break;

			case READING:
				if(rd_completed.read()) { // ожидаем успешного прочтения через общую шину
					std::cout << "completed reading\n";
					sum_completed.write(false);
					state = COMPUTING_SUM;	
					compute_sum();
				}
				break;

			case COMPUTING_SUM:
				if(sum_completed.read()) {
					std::cout << "sum completed\n";
					state = COMPUTING_ACTIVATION;	
					compute_activation();
				}

				break;
			case COMPUTING_ACTIVATION:
				if(activation_completed.read()) {
					std::cout << "activation completed\n";
					state = WRITING;	
					write_ram_result();
				}
				break;
			case WRITING:
				if(wr_completed.read()) {
					std::cout << "TASK IS COMPLETED! res of neuron: " << activation << "\n";
					state = IDLE;	
				}
				break;
		}
		wait();
	}
}
