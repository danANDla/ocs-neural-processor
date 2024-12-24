#include "testbenches.h"
#include "netreader.h"

void NetReaderTest::_test() {
	wait(20, SC_NS);
	rst.write(true);
	wait(12, SC_NS);
	rst.write(false);
}

void netreadertest() {
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
	netreadertest.data_i(data_i);
	netreadertest.data_o(data_o);
	netreadertest.address(address);

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
    sc_trace(wf, data_o, "data_o");
    sc_trace(wf, reader.rd_o, "rd_o");
    sc_trace(wf, reader.wr_o, "wr_o");
    sc_trace(wf, rst, "rst");

    sc_start(sc_time(100, SC_NS));
    sc_close_vcd_trace_file(wf);
}
