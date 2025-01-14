#include "computing_core.h"

int8_t ComputingCore::get_layer_and_neurons_n(uint32_t &layer, uint32_t &n) {
    return 1;
}

void ComputingCore::read_ram_word(uint32_t addr, uint32_t &word) {
    while (ram_rd_i.read() || ram_wr_i.read()) {
        wait();
    }
    ram_read_req.write(true);
    ram_write_req.write(false);
    ram_addr.write(addr);
    while (!ram_rd_i.read()) {
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
    for (uint32_t i = 1; i < layer; ++i) {
        offset += this_n * prev_n;
        prev_n = this_n;
        read_ram_word(offset, this_n);
		offset++;
    }

    /*printf("layer %u, neuron_id %u, prev_layer_addr %u, neurons_in_this %u, "*/
    /*       "neurons_in_prev %u, edges_address_start %u, result_addr %u\n",*/
    /*       layer, this_layer_neuron_id, prev_layer_address, this_n, prev_n, offset, result_address);*/

    *(local_mem) = prev_n;
    uint32_t local_offset = 1;

    for (uint32_t i = 0; i < prev_n; ++i) {
        uint32_t edge;
        uint32_t neuron;
        read_ram_word(offset + this_n * i + this_layer_neuron_id, edge);
        read_ram_word(prev_layer_address_i - i, neuron);

        *(local_mem + local_offset) = edge;
        local_offset += 1;
        *(local_mem + local_offset) = neuron;
        local_offset += 1;
    }

    rd_completed.write(true);
}

void ComputingCore::compute_sum() {
    uint32_t prev_n, offset;

    prev_n = *(local_mem);
    offset = 1;

    for (uint32_t i = 0; i < prev_n; ++i) {
        uint32_t edge = *(local_mem + offset);
        offset += 1;
        uint32_t neuron = *(local_mem + offset);
        offset += 1;

        float floated_edge = (float)*((float *)&edge);
        float floated_neuron = (float)*((float *)&neuron);

        while (alu_ready.read() != true)
            wait();
        alu_neuron.write(floated_neuron);
        alu_weight.write(floated_edge);
        alu_req.write(true);
        while (alu_ready.read() != false)
            wait();
        alu_req.write(false);
        while (alu_ready.read() != true)
            wait();

        /*std::cout << "read edge: " << floated_edge*/
        /*          << " and neuron: " << floated_neuron << " and sum "*/
        /*          << alu_res.read() << "\n";*/
    }
    activation = alu_res.read();
    sum_completed.write(true);
}

void ComputingCore::compute_activation() {
    activation = 1 / (1 + exp(-activation));
    activation_completed.write(true);
}

void ComputingCore::write_ram_result() {

    while (ram_rd_i.read() || ram_wr_i.read()) {
        wait();
    }

    ram_addr.write(result_address);
    uint32_t uinted_activation = *((uint32_t *)&activation);
    ram_data_write.write(uinted_activation);
    ram_read_req.write(false);
    ram_write_req.write(true);
    while (!ram_wr_i.read()) {
        wait();
    }
    ram_addr.write(sc_lv<32>('Z'));
    ram_data_write.write(sc_lv<32>('Z'));
    ram_read_req.write(false);
    ram_write_req.write(false);

    wr_completed.write(true);
}

void ComputingCore::fsm_controller() {
    while (true) {
        if (rst_i.read()) {
            ram_addr.write(sc_lv<32>('Z'));
            ram_data_write.write(sc_lv<64>('Z'));
            alu_req.write(false);
            is_finished.write(true);
        }

        switch (state) {
        case IDLE:
            if (is_task_i) {
                this_neuron_cords = this_neuron_cords_i;
                prev_layer_address = prev_layer_address_i;
                result_address = res_address_i;
                rd_completed.write(false);
                state = READING;
                activation = 0;
                alu_reset.write(true);
            }
            break;

        case READING:
            is_finished.write(false);
            read_ram_layer();
            alu_reset.write(false);
			sum_completed.write(false);
			state = COMPUTING_SUM;
            compute_sum();
            break;
        case COMPUTING_SUM:
            if (sum_completed.read()) {
                state = COMPUTING_ACTIVATION;
                compute_activation();
            }
            break;
        case COMPUTING_ACTIVATION:
            if (activation_completed.read()) {
                state = WRITING;
                write_ram_result();
            }
            break;
        case WRITING:
            if (wr_completed.read()) {
                printf("TASK[%u][%u] IS COMPLETED! res of neuron: %f\n", this_neuron_cords >> 16, this_neuron_cords & 0x0000ffff, activation);
                is_finished.write(true);
                state = IDLE;
            }
            break;
        }
        wait();
    }
}
