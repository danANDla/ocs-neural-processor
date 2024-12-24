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
      prev_layer_address_i("prev_layer_address_i"),
      result_address_i("result_address_i"),
      output_address_o("output_address_o"),
	  output_data_o("output_data_o"),
	  is_working_o("is_working_o")
{
	output_data_o.initialize(0);
	output_address_o.initialize(0);
	
	SC_METHOD(fsm_controller);
	sensitive << clk_i.pos();
}

void ComputingCore::fsm_controller() {
	switch (state) {
		case IDLE:
			if(is_task_i) {
				this_neuron_cords = this_neuron_cords_i;
				prev_layer_address = prev_layer_address_i;
				result_address = result_address_i;
				rd_completed = SC_LOGIC_0;

				state = READING;
			}
			break;

		case READING:
			if(rd_completed.is_01()) { // ожидаем успешного прочтения через общую шину
				state = COMPUTING_SUM;	
			}
			break;

		case COMPUTING_SUM:

			break;

		case COMPUTING_ACTIVATION:
			break;
		case WRITING:
			break;
	}
}
