#include "testbenches.h"
#include "netreader.h"
#include "computing_core.h"

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

void netreadertest(){

    sc_clock clk("clk", sc_time(10, SC_NS));
	sc_signal_rv<32> data_i;
	sc_signal<uint32_t> data_o;
	sc_signal_rv<32> address;
	sc_signal<bool> rst;
	sc_signal<bool> rd_o;
	sc_signal<bool> wr_o;
	sc_signal<bool> rd_req1, wr_req1, rd_req2, wr_req2;

	rd_req1.write(false);
	wr_req1.write(false);
	rd_req2.write(false);
	wr_req2.write(false);

	NetReaderTest netreadertest("netreadertest");
	netreadertest.rst(rst);
	netreadertest.data_to_write(data_i);
	netreadertest.data_to_read(data_o);
	netreadertest.address(address);
	netreadertest.rd_req1(rd_req1);
	netreadertest.wr_req1(wr_req1);
	netreadertest.rd_req2(rd_req2);
	netreadertest.wr_req2(wr_req2);

	NetReader reader("reader");
	reader.clk_i(clk);
	reader.rst_i(rst);
	reader.data_i(data_i);
	reader.data_o(data_o);
	reader.address_i(address);
	reader.rd_o(rd_o);
	reader.wr_o(wr_o);
	reader.read_request1(rd_req1);
	reader.write_request1(wr_req1);
	reader.read_request2(rd_req2);
	reader.write_request2(wr_req2);

    sc_trace_file *wf = sc_create_vcd_trace_file("wave");
    sc_trace(wf, clk, "clk");
    sc_trace(wf, rst, "rst");
    sc_trace(wf, reader.rd_o, "rd_o");
    sc_trace(wf, reader.wr_o, "wr_o");
    sc_trace(wf, netreadertest.address, "addr");
    sc_trace(wf, netreadertest.data_to_read, "data_to_read");
    sc_trace(wf, netreadertest.data_to_write, "data_to_write");
    sc_trace(wf, netreadertest.rd_req1, "rd_req1");
    sc_trace(wf, netreadertest.wr_req1, "wr_req1");

    sc_start(sc_time(100, SC_NS));
    sc_close_vcd_trace_file(wf);
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

void netreader_computing_test(){
	NNetwork good_net = read_network_from_file("/home/danandla/BOTAY/byk_ocs/labs/lab1/networks/geometra.txt");
	std::vector<uint8_t> img = read_image_from_file("/home/danandla/BOTAY/byk_ocs/labs/lab3/ocs-neural-processor/images/generated/i1");

    sc_clock clk("clk", sc_time(10, SC_NS));
	sc_signal_rv<32> address;
	sc_signal_rv<32> data_i;
	sc_signal<uint32_t> data_o;
	sc_signal<bool> rst;
	sc_signal<bool> task;
	sc_signal<bool> rd_o, wr_o, rd_req1, wr_req1, rd_req2, wr_req2;

	sc_signal<uint32_t> cords;
	cords.write(0x00010000);
	sc_signal<uint32_t> prev_layer, resulting_neuron_addr;
	prev_layer.write(0x000007ff);
	resulting_neuron_addr.write(0x000007ff - 49);

	BenchComputingNetReader bench("bench");
	bench.rst(rst);
	bench.is_task(task);


	NetReader reader("reader");
	reader.clk_i(clk);
	reader.rst_i(rst);
	reader.data_i(data_i);
	reader.data_o(data_o);
	reader.address_i(address);
	reader.rd_o(rd_o);
	reader.wr_o(wr_o);
	reader.read_request1(rd_req1);
	reader.write_request1(wr_req1);
	reader.read_request2(rd_req2);
	reader.write_request2(wr_req2);
	reader.write_net_to_mem(&good_net);
	reader.write_image_to_mem(img);

	ComputingCore computing("computing");
	computing.clk_i(clk);
	computing.rst_i(rst);
	computing.is_task_i(task);
	computing.this_neuron_cords_i(cords);
	computing.prev_layer_address_i(prev_layer);
	computing.res_address_i(resulting_neuron_addr);

	computing.ram_addr(address);
	computing.ram_data_read(data_o);
	computing.ram_data_write(data_i);
	computing.ram_read_req(rd_req1);
	computing.ram_write_req(wr_req1);
	computing.ram_rd_i(rd_o);
	computing.ram_wr_i(wr_o);

    sc_trace_file *wf = sc_create_vcd_trace_file("wave");
    sc_trace(wf, clk, "clk");
    sc_trace(wf, rst, "rst");
	sc_trace(wf, task, "task");
    sc_trace(wf, reader.rd_o, "rd_o");
    sc_trace(wf, reader.wr_o, "wr_o");
    sc_trace(wf, reader.address_i, "addr");
    sc_trace(wf, reader.data_i, "data_to_read");
    sc_trace(wf, reader.data_o, "data_to_write");
    sc_trace(wf, reader.rd_o, "rd_req1");
    sc_trace(wf, reader.wr_o, "wr_req1");

    sc_start(sc_time(100000, SC_NS));
    sc_close_vcd_trace_file(wf);
}
