#include "testbenches.h"
#include "netreader.h"
#include "control_core.hpp"

void NetReaderTest::rst_test() {
	wait(20, SC_NS);
	rst.write(true);
	wait(12, SC_NS);
	rst.write(false);
}

void NetReaderTest::read_test() {
	wait(20, SC_NS);
	rst.write(true);
	wait(12, SC_NS);
	rst.write(false);

	address.write(4);

	wait(8, SC_NS);
	rd_req1.write(true);
	wait(8, SC_NS);
	rd_req1.write(false);
}

void NetReaderTest::write_read_test() {
	wait(20, SC_NS);
	rst.write(true);
	wait(12, SC_NS);
	rst.write(false);

	address.write(4);

	wait(8, SC_NS);
	data_to_write.write(sc_lv<64>(0x2020202020202020));
	wr_req1.write(true);
	wait(8, SC_NS);
	wr_req1.write(false);
	data_to_write.write(sc_lv<64>('Z'));

	wait(12, SC_NS);
	rd_req1.write(true);
	wait(8, SC_NS);
	rd_req1.write(false);
}
void BenchComputingNetReader::computing_core_readlayer_test() {
	wait(20, SC_NS);
	rst.write(true);
	wait(12, SC_NS);
	rst.write(false);

	wait(8, SC_NS);
	is_task.write(true);
	wait(8, SC_NS);
	is_task.write(false);
}

void control_test(){
	std::cout << "number of cores: "  << CORE_NUMBER << "\n";

	NNetwork good_net = read_network_from_file("/home/danandla/BOTAY/byk_ocs/labs/lab1/networks/geometra.txt");
	std::vector<uint8_t> img = read_image_from_file("/home/danandla/BOTAY/byk_ocs/labs/lab3/ocs-neural-processor/images/generated/i1");

    sc_clock clk("clk", sc_time(10, SC_NS));
	sc_signal<bool> rst;
	sc_signal<bool> task;
	sc_signal<bool> finished;
	BenchComputingNetReader bench("bench");
	bench.rst(rst);
	bench.is_task(task);

	ControlCore control("controlcore");
	control.clk_i(clk);
	control.rst_i(rst);
	control.computing_mode_i(task);
	control.shared_mem.write_image_to_mem(img);
	control.shared_mem.write_net_to_mem(&good_net);
	control.finish(finished);


    sc_trace_file *wf = sc_create_vcd_trace_file("wave");
    sc_trace(wf, clk, "clk");
    sc_trace(wf, rst, "rst");
	sc_trace(wf, task, "start_computing");
    sc_trace(wf, control.shared_mem.rd_o, "rd_o");
    sc_trace(wf, control.shared_mem.wr_o, "wr_o");
    sc_trace(wf, control.shared_mem.address_i, "addr");
    sc_trace(wf, control.shared_mem.data_i, "data_to_read");
    sc_trace(wf, control.shared_mem.data_o, "data_to_write");
    sc_trace(wf, control.shared_mem.read_reqs[0], "rd_req0");
    sc_trace(wf, control.shared_mem.write_reqs[0], "wr_req0");
	sc_trace(wf, control.shared_mem.is_your_discrete[0], "discrete0");

	for(uint8_t i = 0; i < CORE_NUMBER; ++i) {
		char buff[100];
		sprintf(buff, "task_i_%u", i);
		sc_trace(wf, control.core[i]->is_task_i, buff);

		sprintf(buff, "finished_i_%u", i);
		sc_trace(wf, control.core[i]->is_finished, buff);

		sprintf(buff, "rd_req%u", i+1);
		sc_trace(wf, control.shared_mem.read_reqs[i + 1], buff);
		sprintf(buff, "wr_req%u", i+1);
		sc_trace(wf, control.shared_mem.write_reqs[i + 1], buff);


		sprintf(buff, "discrete%u", i+1);
		sc_trace(wf, control.shared_mem.is_your_discrete[i + 1], buff);
	}


    sc_start(sc_time(200000, SC_NS));
    sc_close_vcd_trace_file(wf);
}
