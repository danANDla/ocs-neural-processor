#ifndef _ALU_LINERAR_H
#define _ALU_LINERAR_H

#include "systemc.h"
#include <cinttypes>

SC_MODULE(ALU_linear) {
  public:
    sc_in<bool> clk_i, rst_i, data_i;
    sc_in<float> weight_i, neuron_i;

    sc_out<float> res_o;
    sc_out<bool> ready_o;

    SC_HAS_PROCESS(ALU_linear);
    ALU_linear(sc_module_name nm);
    ~ALU_linear(){};

    void calculate();
	void execute();

  private:
	float sum, weight, neuron;	
};

#endif
