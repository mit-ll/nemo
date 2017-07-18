#include <cassert>

#include "nemo.h"

void expand_log(const ivl_net_logic_t logic, ivl_signal_t aff_sig, Dot_File& df, set<ivl_signal_t>& critical_sigs, set<ivl_signal_t>& explored_sigs, bool expand_search) {
	unsigned num_input_pins = ivl_logic_pins(logic);
	
	// Device Pointers
	ivl_signal_t    sig;
	ivl_net_logic_t prev_logic;
	ivl_lpm_t       prev_lpm;

	// Nexus Pointers
	ivl_nexus_t     input_pin_nexus;
	ivl_nexus_ptr_t nexus_ptr;

	if (DEBUG_PRINTS){ 
		printf("		Logic (%s) is of type %d\n", ivl_logic_basename(logic), ivl_logic_type(logic));
		printf("		num inputs = %d\n", num_input_pins);
	}

	// Propagate over all input pins
	// pin 0 is output, so start with pin 1
	for (unsigned i = 1; i < num_input_pins; i++) {
		input_pin_nexus = ivl_logic_pin(logic, i);

		if (DEBUG_PRINTS){ printf("			num nexus pointers at input = %d\n", ivl_nexus_ptrs(input_pin_nexus)); }
		// Iterate over all nexus pointers for LPM input pin 
		// nexus to find the input signal
		for (unsigned j = 0; j < ivl_nexus_ptrs(input_pin_nexus); j++){
			nexus_ptr = ivl_nexus_ptr(input_pin_nexus, j);
			if ((sig = ivl_nexus_ptr_sig(nexus_ptr))) {
				// Do not expand local IVL compiler generated signals
				// unless they are outputs of constants
				connect_signals(aff_sig, sig, critical_sigs, explored_sigs, df, expand_search);
			}
			else if ((prev_logic = ivl_nexus_ptr_log(nexus_ptr)) != logic){
				assert(!prev_logic && "Logic unit connected directly to logic unit\n");
			} else if ((prev_lpm = ivl_nexus_ptr_lpm(nexus_ptr))){
				assert(false && "Logic unit connected directly to lpm\n");
			}	
		}
	}
}