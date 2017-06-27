#include <cassert>

#include <ivl_target.h>
#include "ttb.h"

// Logic devices are easy!
void propagate_log(const ivl_net_logic_t logic, ivl_signal_t aff_sig, Nemo_Design& nemo_des) {
	unsigned int    npins = ivl_logic_pins(logic);
	ivl_nexus_t     input_pin_nex;
	ivl_nexus_ptr_t nex_ptr;
	unsigned int    num_nexus_ptrs;
	ivl_signal_t    sig;

	// Propagate over all input pins
	// pin 0 is output
	for (unsigned int i = 1; i < npins; i++) {
		input_pin_nex = ivl_logic_pin(logic, i);
		for (unsigned int i=0; i<ivl_nexus_ptrs(input_pin_nex); i++){
			nex_ptr = ivl_nexus_ptr(input_pin_nex, i);
			// assert(!ivl_nexus_ptr_log(nex_ptr) && "Logic unit connected directly to logic unit");
			if (ivl_nexus_ptr_log(nex_ptr)){
				printf("Logic unit connected directly to logic unit (%s)\n", ivl_signal_basename(aff_sig));
			}
			// assert(!ivl_nexus_ptr_lpm(nex_ptr) && "Logic unit connected directly to lpm");	ivl_nexus_ptr_t = ivl_nexus_ptr(input_pin_nex, i);
			if (ivl_nexus_ptr_lpm(nex_ptr)){
				printf("Logic unit connected directly to lpm (%s)\n", ivl_signal_basename(aff_sig));
			}
			sig = ivl_nexus_ptr_sig(ivl_nexus_ptr(input_pin_nex, 0));
			if (sig) {
				nemo_des.add_connection(aff_sig, sig);
			}
		}
	}
}
