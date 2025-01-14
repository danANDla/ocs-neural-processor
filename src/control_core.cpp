#include "control_core.hpp"
#include "netreader.h"
#include <cstdio>

ControlCore::ControlCore(sc_module_name mn)
    : sc_module(mn), clk_i("clk_i"), rst_i("clk_i"),
      computing_mode_i("computing_mode_i"), shared_mem("shared_ram"),
      q(std::queue<ComputingTask>()) {
    SC_THREAD(fsm_controller);
    sensitive << clk_i.pos();

    std::cout << "creating\n";

    shared_mem.clk_i(clk_i);
    shared_mem.rst_i(rst_i);
    shared_mem.data_i(ram_data_i);
    shared_mem.data_o(ram_data_o);
    shared_mem.address_i(ram_address);
    shared_mem.rd_o(ram_rd_o);
    shared_mem.wr_o(ram_wr_o);

    shared_mem.read_reqs[0](ram_rd_requests[0]);
    shared_mem.write_reqs[0](ram_wr_requests[0]);
	shared_mem.is_your_discrete[0](ram_discretes[0]);

    char buff[40];
    for (uint8_t i = 0; i < CORE_NUMBER; ++i) {

        std::sprintf(buff, "core%u", i);
        std::cout << buff << "\n";

        shared_mem.read_reqs[i + 1](ram_rd_requests[i + 1]);
        shared_mem.write_reqs[i + 1](ram_wr_requests[i + 1]);
		shared_mem.is_your_discrete[i + 1](ram_discretes[i + 1]);

        core[i] = new ComputingCore(buff);
        core[i]->clk_i(clk_i);
        core[i]->rst_i(rst_i);
        core[i]->is_task_i(tasks[i]);
        core[i]->this_neuron_cords_i(cords);
        core[i]->prev_layer_address_i(prev_layer);
        core[i]->res_address_i(resulting_neuron_addr);
        core[i]->ram_addr(ram_address);
        core[i]->ram_data_read(ram_data_o);
        core[i]->ram_data_write(ram_data_i);
        core[i]->ram_read_req(ram_rd_requests[i + 1]);
        core[i]->ram_write_req(ram_wr_requests[i + 1]);
        core[i]->ram_rd_i(ram_rd_o);
        core[i]->ram_wr_i(ram_wr_o);
		core[i]->ram_discrete(ram_discretes[i + 1]);
        core[i]->is_finished(cores_finished[i]);
    }
}

void ControlCore::read_ram_word(uint32_t addr, uint32_t &word) {

    while (!(ram_discretes[0].read() && !ram_wr_o.read() && !ram_rd_o.read())) {
        wait();
    }
    ram_address.write(addr);
    ram_rd_requests[0].write(true);
    ram_wr_requests[0].write(false);
    wait();
    wait();
    word = ram_data_o.read();

    ram_data_i.write(sc_lv<32>('Z'));
    ram_address.write(sc_lv<32>('Z'));
    ram_rd_requests[0].write(false);
    ram_wr_requests[0].write(false);
}

void ControlCore::read_net() {
    uint32_t layers_n;
    read_ram_word(0, layers_n); // layer 0
    uint32_t offset = 3;
    uint32_t prev_n, this_n;
    read_ram_word(1, prev_n); // layer 0
    read_ram_word(2, this_n); // layer 1

    layers.push_back(prev_n);
    layers.push_back(this_n);
    for (uint32_t i = 1; i < layers_n; ++i) {
        // printf("layer %u, neurons_in_this %u, neurons_in_prev %u\n" ,i,
        // this_n, prev_n);
        offset += this_n * prev_n;
        prev_n = this_n;
        if (i != layers_n - 1) {
            read_ram_word(offset, this_n);
            layers.push_back(this_n);
        }
    }

    for (uint16_t l : layers)
        std::cout << l << " ";
    std::cout << std::endl;
}

void ControlCore::create_task_q() {
    std::cout << "create_task_q\n";
    uint16_t tasks = layers[done_layers];

    uint32_t prev_l_addr;
    uint32_t result_offset = 0;
    for (uint16_t i = 0; i < done_layers; ++i) {
        prev_l_addr = result_offset;
        result_offset += layers[i];
    }

    for (uint16_t i = 0; i < tasks; ++i) {
        ComputingTask t = {.layer = done_layers,
                           .id_in_layer = i,
                           .result_address = MEM_SIZE - 1 - result_offset - i,
                           .prev_layer_addr = MEM_SIZE - 1 - prev_l_addr};
        /*std::cout << done_layers << " " << i << " " << result_offset << " "*/
        /*          << prev_l_addr << std::endl;*/
        q.push(t);
    }
}


void ControlCore::show_result() {

	uint32_t offset = 0;
	for(size_t i = 0; i < layers.size() - 1; ++i) {
		offset  += layers[i];
	}

	int8_t res = 0;

	float ma;
	uint32_t neuron_val ;
	float floated_neuron;
	char buff[100] = "is circle";	

	read_ram_word(MEM_SIZE - 1 - offset, neuron_val);
	floated_neuron = *((float*) &neuron_val);
	ma = floated_neuron;
	std::cout << floated_neuron << " ";

	read_ram_word(MEM_SIZE - 1 - offset - 1, neuron_val);
	floated_neuron = *((float*) &neuron_val);
	if(floated_neuron > ma) {
		ma = floated_neuron;
		res = 1;	
		sprintf(buff, "is triangle");
	}
	std::cout << floated_neuron << " ";

	read_ram_word(MEM_SIZE - 1 - offset - 2, neuron_val);
	floated_neuron = *((float*) &neuron_val);
	if(floated_neuron > ma) {
		ma = floated_neuron;
		res = 2;	
		sprintf(buff, "is square");
	}
	std::cout << floated_neuron << " ";

	std::cout << "RESULT: " << buff << " \n";
}

void ControlCore::assign_task() {
    if (q.empty())
        return;

    ComputingTask task_for_c = q.front();
    q.pop();

    uint32_t layer = task_for_c.layer << 16;
    layer += task_for_c.id_in_layer;
    cords.write(layer);
    prev_layer.write(task_for_c.prev_layer_addr);
    resulting_neuron_addr.write(task_for_c.result_address);

    tasks[free].write(true);
	while(cores_finished[free].read() != false) wait();
    tasks[free].write(false);
}

void ControlCore::fsm_controller() {
    while (true) {
        if (rst_i.read()) {
            state = IDLE;
            done_layers = 0;
            layers.clear();
        }
        switch (state) {

        case IDLE:
            if (computing_mode_i) {
                std::cout << "starting\n";
                read_net();
                done_layers = 1;
                state = CHECKING_LAYER;
            }
            break;
        case CHECKING_LAYER:
            if (done_layers == layers.size()) {
                state = SHOWING_RESULT;
            } else {
                state = CREATING_TASK_Q;
				create_task_q();
            }
            break;
        case CREATING_TASK_Q:
            state = WAITING_FREE_COMPUTING_CORE;
            break;
        case WAITING_FREE_COMPUTING_CORE:
			for(size_t i = 0; i < CORE_NUMBER; ++i) {
				if(core[i]->is_finished.read() == true) {
                state = CHECKING_ANY_TASK_IN_Q;
				}
			}
            break;
        case CHECKING_ANY_TASK_IN_Q:
            if (q.empty()) {
				done_layers++;
                state = CHECKING_LAYER;
				// std::cout << "CHECK NEW LAYER";
            } else {
                state = ASSIGNING_TASK_TO_COMPUTING_CORE;
				assign_task();
            }
            break;
        case ASSIGNING_TASK_TO_COMPUTING_CORE:
            state = WAITING_FREE_COMPUTING_CORE;
            break;
        case SHOWING_RESULT:
			show_result();
			wait();
			state = IDLE;
            break;
        }
        wait();
    }
}
