#ifndef _CONTROL_CORE_H
#define _CONTROL_CORE_H

#include "computing_core.h"
#include "netreader.h"
#include "systemc.h"
#include <cinttypes>
#include <queue>

typedef struct {
	uint16_t layer;
	uint16_t id_in_layer;
	uint32_t result_address;
	uint32_t prev_layer_addr;
} ComputingTask;

SC_MODULE(ControlCore) {
  public:
    enum State {
        IDLE = 0,
        CHECKING_LAYER,
        CREATING_TASK_Q,
        WAITING_FREE_COMPUTING_CORE,
        CHECKING_ANY_TASK_IN_Q,
		ASSIGNING_TASK_TO_COMPUTING_CORE,
		SHOWING_RESULT
    };

    sc_in<bool> clk_i, rst_i, computing_mode_i;
	sc_out<bool> finish;

    SC_HAS_PROCESS(ControlCore);
    ControlCore(sc_module_name nm);
    ~ControlCore(){};


    void fsm_controller();

    void check_for_layers();
    void create_task_q();
    void check_task_in_q();
    void assign_task();
    void show_result();

	int8_t get_layer_and_neurons_n(uint32_t& l, uint32_t& n);
	void read_ram_word(uint32_t addr, uint32_t& word);

	ComputingCore* core[CORE_NUMBER];
	NetReader shared_mem;
  private:
	void read_net();

    State state;
	sc_signal<bool> check_layers_completed, create_q_completed, wait_free_completed, check_q_completed, assign_completed;
    uint8_t local_mem[2048];

	sc_signal<bool> cores_finished[CORE_NUMBER];
	sc_signal<bool> tasks[CORE_NUMBER];
	sc_signal<uint32_t> cords;
	sc_signal<uint32_t> prev_layer, resulting_neuron_addr;
	sc_signal<bool> GND;

	sc_signal_rv<32> ram_address, ram_data_i;
	sc_signal<uint32_t> ram_data_o;
	sc_signal<bool> ram_rd_o, ram_wr_o;
	sc_signal<bool> ram_wr_requests[CORE_NUMBER + 1];
	sc_signal<bool> ram_rd_requests[CORE_NUMBER + 1];
	sc_signal<bool> ram_discretes[CORE_NUMBER + 1];

	std::vector<uint16_t> layers;
	std::queue<ComputingTask> q;
	uint16_t done_layers;
	int8_t free_c;
};

#endif
