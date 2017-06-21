#include <cassert>
#include <cstdlib>
#include <cstdio>

#include "ivl_target.h"
#include "ttb.h"

/* Signals are easy... ish */
void propagate_sig(ivl_signal_t aff_sig, Nemo_Design& nemo_des) {
	ivl_net_const_t con;
	ivl_net_logic_t logic;
	ivl_lpm_t       lpm;
	ivl_signal_t    sig;
	ivl_switch_t    swt;
	ivl_branch_t    bra;
	ivl_nexus_ptr_t nex_ptr;

	//First get its nexus
	//@TODO: Deal with larger vectors (more than one nexus)
	//@TODO: Figure out how more than one nexus works...
	const int base  = ivl_signal_array_base(aff_sig);
	const int count = ivl_signal_array_count(aff_sig);
	assert(count >= 0);
	if (count > 1) {
		fprintf(stderr, "Error: Skipping Arrayed Signal: %s\n", nemo_des.get_sig_name(aff_sig).c_str());
		fprintf(stderr, "File: %s Line: %d\n", ivl_signal_file(aff_sig), ivl_signal_lineno(aff_sig));
		return;
	}

	// Iterate through a signal's nexuses
	for (int idx = base; idx < base + count; idx++){
		const ivl_nexus_t nex = ivl_signal_nex(aff_sig, idx);
		assert(nex);

		// Iterate through the pointers in a given nexus
		for (unsigned int i = 0; i < ivl_nexus_ptrs(nex); i++){
			nex_ptr = ivl_nexus_ptr(nex, i);
			assert(nex_ptr);

			if ((sig = ivl_nexus_ptr_sig(nex_ptr))){
				// Means that the signals are the same. Usually happens on module hookups
				// Also if two signals in a module are hooked up to the same thing
				
				// If the signal is different signal --> add connection
				if (aff_sig != sig){
					nemo_des.add_connection(aff_sig, sig);
				}
			}
			else if ((logic = ivl_nexus_ptr_log(nex_ptr))){
				// Pin 0 is the output of every logic device
				if (ivl_logic_pin(logic, 0) == nex) {
					propagate_log(logic, aff_sig, nemo_des);
				}
			}
			else if ((lpm = ivl_nexus_ptr_lpm(nex_ptr))){
				assert(false && "LPMs are unsupported nexus pointers.\n");
			}
			else if ((swt = ivl_nexus_ptr_switch(nex_ptr))){
				assert(false && "Switches are unsupported nexus pointers.\n");
			}
			else if ((con = ivl_nexus_ptr_con(nex_ptr))){
				assert(false && "Constants are unsupported nexus pointers.\n");
			}
			else if ((bra = ivl_nexus_ptr_branch(nex_ptr))){
				assert(false && "Branches are unsupported nexus pointers.\n");
			}
			else{
				assert(false && "? are unsupported nexus pointers.\n");	
			}
		}
	}
}