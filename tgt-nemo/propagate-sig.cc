#include <cassert>
#include <cstdlib>
#include <cstdio>

#include "ivl_target.h"
#include "nemo.h"

/* Signals are easy... ish */
void propagate_sig(ivl_signal_t aff_sig, Dot_File& df) {
	// Device Pointers
	ivl_net_const_t con;
	ivl_net_logic_t logic;
	ivl_lpm_t       lpm;
	ivl_signal_t    sig;
	ivl_switch_t    swt;
	ivl_branch_t    bra;

	// Nexus Pointers
	ivl_nexus_t     nex = ivl_signal_nex(aff_sig, 0);
	ivl_nexus_ptr_t nex_ptr;
	
	if (DEBUG_PRINTS){
		printf("Signal %s.%s, ", ivl_scope_name(ivl_signal_scope(aff_sig)), ivl_signal_basename(aff_sig));
		printf("num_ptrs: %d\n", ivl_nexus_ptrs(nex));
	}

	// Iterate through the pointers in a given nexus
	for (unsigned int i = 0; i < ivl_nexus_ptrs(nex); i += 1){
		nex_ptr = ivl_nexus_ptr(nex, i);
		assert(nex_ptr);
		if ((sig = ivl_nexus_ptr_sig(nex_ptr))){
			// Means that the signals are the same. Usually happens on module hookups
			// 
			// Also if two signals in a module are hooked up to the same thing
			// 
			// Also if a signal is an input signal (i.e. nothing connects to it)

			// If the signal is different signal --> add connection
			//@TODO: it is difficult to know what signal is an output vs inputs
			if (aff_sig != sig){
				// only propagate a signal to a signal if it is an output port
				// if (ivl_signal_port(aff_sig) == IVL_SIP_OUTPUT || ivl_signal_port(aff_sig) == IVL_SIP_INOUT){
					// Do not propagate local IVL compiler generated signals
					if (!is_ivl_generated_signal(sig)){
						if (DEBUG_PRINTS){ printf("	input %d is a SIGNAL device (%s).\n", i, ivl_signal_basename(sig)); }
						df.add_connection(aff_sig, sig);	
					}
				// }
			}
		}
		else if ((logic = ivl_nexus_ptr_log(nex_ptr))){ 
			// Pin-0 is the output of every logic device.
			// 
			// Output nexus of the logic device should be the same nexus as the aff_sig
			// otherwise the aff_sig is driving an input of the LPM
			if (ivl_logic_pin(logic, 0) == nex) {
				if (DEBUG_PRINTS){ printf("	input %d is a LOGIC device.\n", i); }
				propagate_log(logic, aff_sig, df);
			}
		}
		else if ((lpm = ivl_nexus_ptr_lpm(nex_ptr))){
			// Output nexus of LPM should be the same nexus of the aff_sig
			// otherwise the aff_sig is driving an input of the LPM
			if (ivl_lpm_q(lpm) == nex) {
				if (DEBUG_PRINTS){ printf("	input %d is a LPM device (type: %d).\n", i, ivl_lpm_type(lpm)); }
				propagate_lpm(lpm, aff_sig, df);
     		}
		} else if ((con = ivl_nexus_ptr_con(nex_ptr))){
			continue;
		}
		else if ((swt = ivl_nexus_ptr_switch(nex_ptr))){
			assert(false && "Switches are unsupported nexus pointers.\n");
		}
		else if ((bra = ivl_nexus_ptr_branch(nex_ptr))){
			assert(false && "Branches are unsupported nexus pointers.\n");
		}
		else{
			assert(false && "? are unsupported nexus pointers.\n");	
		}
	}
}