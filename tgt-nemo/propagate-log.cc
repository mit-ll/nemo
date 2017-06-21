#include <cassert>

#include <ivl_target.h>
#include "ttb.h"

// Logic devices are easy!
void propagate_log(const ivl_net_logic_t logic, ivl_signal_t aff_sig, Nemo_Design& nemo_des) {
	unsigned int npins = ivl_logic_pins(logic);
	ivl_nexus_t  input_nex;
	ivl_signal_t sig;

	// Propagate over all input pins
	// pin 0 is output
	for (unsigned int i = 1; i < npins; i++) {
		input_nex = ivl_logic_pin(logic, i);
		// Why does below assume every nexus only has one pointer in it at idx 0?
		assert(!ivl_nexus_ptr_log(ivl_nexus_ptr(input_nex, 0)) && "Logic unit connected directly to logic unit");
		assert(!ivl_nexus_ptr_lpm(ivl_nexus_ptr(input_nex, 0)) && "Logic unit connected directly to lpm");
		sig = ivl_nexus_ptr_sig(ivl_nexus_ptr(input_nex, 0));
		if (sig) {
			nemo_des.add_connection(aff_sig, sig);
		}
	}
}
