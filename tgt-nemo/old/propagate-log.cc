#include <cassert>

#include <ivl_target.h>
#include "nemo_design.h"

// Logic devices are easy!
void Nemo_Design::propagate_log(const ivl_net_logic_t logic, ivl_signal_t aff_sig) {
	unsigned int    npins = ivl_logic_pins(logic);
	ivl_nexus_t     input_pin_nex;
	ivl_nexus_ptr_t nex_ptr;
	unsigned int    num_nexus_ptrs;
	ivl_signal_t    sig;
	ivl_net_logic_t prev_logic;
	ivl_lpm_t       prev_lpm;

	// Propagate over all input pins
	// pin 0 is output, so start with pin 1
	for (unsigned int i = 1; i < npins; i++) {
		input_pin_nex = ivl_logic_pin(logic, i);
		for (unsigned int i=0; i < ivl_nexus_ptrs(input_pin_nex); i++){
			nex_ptr = ivl_nexus_ptr(input_pin_nex, i);
			// assert(!ivl_nexus_ptr_log(nex_ptr) && "Logic unit connected directly to logic unit");
			if ((prev_logic = ivl_nexus_ptr_log(nex_ptr))){
				propagate_log(prev_logic, aff_sig);
			}
			// assert(!ivl_nexus_ptr_lpm(nex_ptr) && "Logic unit connected directly to lpm");	ivl_nexus_ptr_t = ivl_nexus_ptr(input_pin_nex, i);
			if ((prev_lpm = ivl_nexus_ptr_lpm(nex_ptr))){
				propagate_lpm(prev_lpm, aff_sig);
			}
			sig = ivl_nexus_ptr_sig(nex_ptr);
			if (sig) {
				add_connection(aff_sig, sig);
			}
		}
	}
}
