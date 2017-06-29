#include <cassert>

#include "ivl_target.h"
#include "nemo.h"

// Logic devices are easy!
void propagate_log(const ivl_net_logic_t logic, ivl_signal_t aff_sig, Dot_File& df) {
	unsigned int    num_input_pins = ivl_logic_pins(logic);
	unsigned int    num_nexus_ptrs;
	
	// Device Pointers
	ivl_signal_t    sig;
	ivl_net_logic_t prev_logic;
	ivl_lpm_t       prev_lpm;

	// Nexus Pointers
	ivl_nexus_t     input_pin_nexus;
	ivl_nexus_ptr_t nexus_ptr;

	// Propagate over all input pins
	// pin 0 is output, so start with pin 1
	for (unsigned int i = 1; i < num_input_pins; i++) {
		input_pin_nexus = ivl_logic_pin(logic, i);

		// Iterate over all nexus pointers for a given input pin nexus
		for (unsigned int i = 0; i < ivl_nexus_ptrs(input_pin_nexus); i++){
			nexus_ptr = ivl_nexus_ptr(input_pin_nexus, i);
			if ((prev_logic = ivl_nexus_ptr_log(nexus_ptr))){
				// Nexus pointer points to a logic device
				propagate_log(prev_logic, aff_sig, df);
			}
			if ((prev_lpm = ivl_nexus_ptr_lpm(nexus_ptr))){
				// Nexus pointer points to a lpm device
				propagate_lpm(prev_lpm, aff_sig, df);
			}
			if ((sig = ivl_nexus_ptr_sig(nexus_ptr))) {
				// Nexus pointer points to a signal
				df.add_connection(aff_sig, sig);
			}
		}
	}
}
