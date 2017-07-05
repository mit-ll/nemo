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

	//First get its nexus
	//@TODO: Deal with larger vectors (more than one nexus)
	//@TODO: Figure out how more than one nexus works...
	const int count = ivl_signal_array_count(aff_sig);
	assert(count >= 0);
	if (count > 1) {
		fprintf(stderr, "Error: Skipping Arrayed Signal: %s\n", ivl_signal_basename(aff_sig));
		fprintf(stderr, "File: %s Line: %d\n", ivl_signal_file(aff_sig), ivl_signal_lineno(aff_sig));
		return;
	}
	assert(nex && "ERROR: invalid nexus for signal\n");
	
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
			// Also if two signals in a module are hooked up to the same thing
			// Also if a signal is an input signal (i.e. nothing connects to it)

			// If the signal is different signal --> add connection
			//@TODO: it is difficult to know what signal is an output vs inputs
			if (aff_sig != sig){
				if (DEBUG_PRINTS){ printf("	input %d is a SIGNAL device (%s).\n", i, ivl_signal_basename(sig)); }
				df.add_connection(aff_sig, sig);
			}
		}
		// else if ((logic = ivl_nexus_ptr_log(nex_ptr))){ 
		// 	// Pin 0 is the output of every logic device
		// 	// if aff_sig nexus is different the the logic output nexus
		// 	// the aff_sig is driving an input of the logic device
		// 	if (ivl_logic_pin(logic, 0) == nex) {
		// 		if (DEBUG_PRINTS){ printf("	input %d is a LOGIC device.\n", i); }
		// 		propagate_log(logic, aff_sig, df);
		// 	}
		// }
		else if ((lpm = ivl_nexus_ptr_lpm(nex_ptr))){
			// Output nexus of LPM should be the nexus of the aff_sig
			// otherwise the aff_sig is driving an input of the LPM
			if (ivl_lpm_q(lpm) == nex && ivl_signal_local(aff_sig)) {
				if (DEBUG_PRINTS){ printf("	input %d is a LPM device (type: %d).\n", i, ivl_lpm_type(lpm)); }
				propagate_lpm(lpm, aff_sig, df);
     		}
		} else if ((con = ivl_nexus_ptr_con(nex_ptr))){
			if (ivl_signal_local(aff_sig)){
				if (DEBUG_PRINTS){ printf("	input %d is a CONSTANT.\n", i); }
				df.add_const_node(con);
				df.add_const_connection(aff_sig, con);
			}
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