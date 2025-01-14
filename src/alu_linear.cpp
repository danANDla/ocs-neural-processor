#include "alu_linear.hpp"

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

