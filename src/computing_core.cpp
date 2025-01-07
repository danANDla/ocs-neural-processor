#include "computing_core.h"

ALU_linear::ALU_linear(sc_module_name mn) :
	sc_module(mn) {
	SC_METHOD(calculate);	
	sensitive << clk_i.pos();
	res_o.initialize(0);
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
	if(data_i) {
		res_o = sum + weight * neuron;
	}
}

ComputingCore::ComputingCore(sc_module_name mn)
    : sc_module(mn), clk_i("clk_i"), 
	  is_task_i("is_task_i"),
	  this_neuron_cords_i("this_neuron_cords_i"),
      prev_layer_address_i("prev_layer_address_i")
{
	
	SC_THREAD(fsm_controller);
	sensitive << clk_i.pos();
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
	rd_completed.write(true);
}

void ComputingCore::read_ram_layer() {
	uint32_t layer = this_neuron_cords >> 16;
	uint32_t this_layer_neuron_id = this_neuron_cords & 0x0000ffff;
	
	uint32_t offset = 2;
	uint32_t prev_n, this_n;
	read_ram_word(1, prev_n);
	read_ram_word(2, this_n);
	for(uint32_t i = 3; i < layer; ++i) {
		offset += this_n * prev_n;
		prev_n = this_n;
		read_ram_word(offset, this_n);
	}

	printf("layer %u, neuron_id %u, prev_layer_addr %u, neurons_in_this %u, neurons_in_prev %u\n", layer, this_layer_neuron_id, prev_layer_address, this_n, prev_n);

	for(uint32_t i = 0; i < prev_n; ++i) {
		uint32_t edge;
		uint32_t neuron;
		read_ram_word(offset + prev_n * this_layer_neuron_id + i, edge);
		read_ram_word(prev_layer_address_i - i, neuron);
		float floated_edge = (float) *((float*)&edge);
		std::cout << "read edge: " << floated_edge << " and nueron: " << neuron << "\n";
	}

}

void ComputingCore::fsm_controller() {
	while(true) {
		if(rst_i.read()) {
			ram_addr.write(sc_lv<32>('Z'));
			ram_data_write.write(sc_lv<64>('Z'));
		}

		switch (state) {
			case IDLE:
				if(is_task_i) {
					std::cout<< "got task\n";
					this_neuron_cords = this_neuron_cords_i;
					prev_layer_address = prev_layer_address_i;
					state = READING;
					rd_completed.write(false);
					read_ram_layer();
				}
				break;

			case READING:
				if(rd_completed.read()) { // ожидаем успешного прочтения через общую шину
					state = COMPUTING_SUM;	
					std::cout << "completed reading\n";
				}
				break;

			case COMPUTING_SUM:

				break;

			case COMPUTING_ACTIVATION:
				break;
			case WRITING:
				break;
		}
		wait();
	}
}
