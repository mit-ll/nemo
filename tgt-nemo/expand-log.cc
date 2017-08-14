#include <cassert>

#include "nemo.h"

void expand_log(const ivl_net_logic_t logic, ivl_signal_t aff_sig, Dot_File& df, set<ivl_signal_t>& sigs_to_expand, set<ivl_signal_t>& explored_sigs, bool expand_search) {
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
		printf("		num inputs = %d\n", (num_input_pins - 1));
	}

	// Propagate over all input pins
	// pin 0 is output, so start with pin 1
	for (unsigned i = 1; i < num_input_pins; i++) {
		input_pin_nexus = ivl_logic_pin(logic, i);
		
		if (DEBUG_PRINTS){ printf("			num nexus pointers at input = %d\n", ivl_nexus_ptrs(input_pin_nexus)); }
		// Iterate over all nexus pointers for Logic input pin 
		// nexus to find the input signal
		for (unsigned j = 0; j < ivl_nexus_ptrs(input_pin_nexus); j++){
			nexus_ptr = ivl_nexus_ptr(input_pin_nexus, j);
			if ((sig = ivl_nexus_ptr_sig(nexus_ptr))) {
				// If logic device is of IVL_LO_BUFZ type, this is a connection
				// of many pieces of logic to a single net that is the right
				// hand side of a continous assign statement. Only make connection of 
				// the "wire" object, and add it to list for expansion.
				if (ivl_logic_type(logic) != IVL_LO_BUFZ){
					connect_signals(aff_sig, sig, sigs_to_expand, explored_sigs, df, expand_search);
				} else if (ivl_signal_scope(sig) == ivl_signal_scope(aff_sig)) {
					connect_signals(aff_sig, sig, sigs_to_expand, explored_sigs, df, expand_search);
				} else if (DEBUG_LOGIC_ASSIGN_CONNECTION) {
					printf("Info <expand_log()>: Ignoring connection of IVL_LO_BUFZ input %s.%s\n", ivl_scope_basename(ivl_signal_scope(sig)), ivl_signal_basename(sig));
				}
			}
			else if ((prev_logic = ivl_nexus_ptr_log(nexus_ptr)) != logic){
				assert(!prev_logic && "Logic unit connected directly to logic unit\n");
			} else if ((prev_lpm = ivl_nexus_ptr_lpm(nexus_ptr))){
				assert(false && "Logic unit connected directly to lpm\n");
			}	
		}
	}
}
