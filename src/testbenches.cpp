#include "testbenches.h"
#include "netreader.h"

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
	sc_signal_rv<64> data_i, data_o;
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
