#ifndef _COMPUTING_CORE_H
#define _COMPUTING_CORE_H

#include "alu_linear.hpp"
#include "systemc.h"
#include <cinttypes>

SC_MODULE(ComputingCore) {
  public:
    enum State {
        IDLE = 0,
        READING,
        COMPUTING_SUM,
        COMPUTING_ACTIVATION,
        WRITING
    };

    sc_in<bool> clk_i, rst_i, is_task_i;
    sc_in<uint32_t> this_neuron_cords_i;
    sc_in<uint32_t> prev_layer_address_i, res_address_i;
    sc_out<bool> is_finished;

    sc_in<bool> ram_rd_i, ram_wr_i, ram_discrete;
    sc_out<bool> ram_read_req, ram_write_req;
    sc_out_rv<32> ram_addr, ram_data_write;
    sc_in<uint32_t> ram_data_read;

    SC_CTOR(ComputingCore)
        : clk_i("clk_i"), is_task_i("is_task_i"),
          this_neuron_cords_i("this_neuron_cords_i"),
          prev_layer_address_i("prev_layer_address_i"),
          res_address_i("res_address_i"), alu("alu") {
        SC_THREAD(fsm_controller);
        sensitive << clk_i.pos();

        alu.clk_i(clk_i);
        alu.rst_i(alu_reset);
        alu.data_i(alu_req);
        alu.weight_i(alu_weight);
        alu.neuron_i(alu_neuron);
        alu.res_o(alu_res);
        alu.ready_o(alu_ready);
    }

    void fsm_controller();
    void wait_task();
    void read_ram_layer();
    void compute_sum();
    void compute_activation();
    void write_ram_result();

    int8_t get_layer_and_neurons_n(uint32_t & l, uint32_t & n);
    void read_ram_word(uint32_t addr, uint32_t & word);

  private:
    ALU_linear alu;

    float line_func_add(float sum, float w, float n_v);
    sc_signal<bool> rd_completed, sum_completed, activation_completed,
        wr_completed;

    uint32_t local_mem[2048];
    uint32_t this_neuron_cords;
    uint32_t prev_layer_address;
    uint32_t result_address;
    float activation;

    sc_signal<bool> alu_req, alu_ready, alu_reset;
    sc_signal<float> alu_weight, alu_neuron, alu_res;

#define NUMBER_OF_NEURONS *((uint32_t *)((uint8_t *)local_mem + 0));

    State state;
};

#endif
