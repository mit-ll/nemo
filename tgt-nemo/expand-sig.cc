#include <cassert>
#include <cstdlib>
#include <cstdio>

#include "nemo.h"

void expand_sig(ivl_signal_t aff_sig, Dot_File& df, set<ivl_signal_t>& critical_sigs, set<ivl_signal_t>& explored_sigs, bool expand_search) {
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

	// If the module (IVL scope) is a std cell, no signals
	// will be connected in the std cell template verilog file
	// so expand all inputs to all outputs.
	if (ivl_scope_is_cell(ivl_signal_scope(aff_sig))){
		expand_std_cell_sigs(aff_sig, df);
	} 

	// Iterate through the pointers in a given nexus
	for (unsigned int i = 0; i < ivl_nexus_ptrs(nex); i++) {
		nex_ptr = ivl_nexus_ptr(nex, i);
		assert(nex_ptr && "Error: expand_sig() -- nexus pointer is invalid.\n");
		if ((sig = ivl_nexus_ptr_sig(nex_ptr))) {
			if (DEBUG_PRINTS){ printf("	input %d is a SIGNAL device (%s.%s).\n", i, ivl_scope_name(ivl_signal_scope(sig)), ivl_signal_basename(sig)); }
			// If the signal is different signal than itself --> add connection
			if (aff_sig != sig) {
				// Only expand the output signal of a child module to 
				// the output of a parent module.
				if (are_both_signals_outputs(aff_sig, sig)) {
					ivl_scope_t aff_sig_scope = ivl_signal_scope(aff_sig);
					ivl_scope_t sig_scope     = ivl_signal_scope(sig);

					// Only connect the output signal of a child module to 
					// the output of a parent module. Do not connect two 
					// outputs together.
					if (ivl_scope_parent(sig_scope) == aff_sig_scope) {
						connect_signals(aff_sig, sig, critical_sigs, explored_sigs, df, expand_search);
						printf("HERE 1\n");
					}
				} else if (is_sig1_output_and_sig2_not(aff_sig, sig) || !is_sig_output(aff_sig)){
					connect_signals(aff_sig, sig, critical_sigs, explored_sigs, df, expand_search);
					printf("HERE 2\n");
				} else {
					assert(false && "Error: expand_sig() -- invalid signal types.\n");
				}
			}
		}
		else if ((logic = ivl_nexus_ptr_log(nex_ptr))) { 
			// Pin-0 is the output of every logic device.
			// 
			// Output nexus of the logic device should be the same nexus as the aff_sig
			// otherwise the aff_sig is driving an input of the LPM
			if (ivl_logic_pin(logic, 0) == nex) {
				if (DEBUG_PRINTS){ printf("	input %d is a LOGIC device.\n", i); }
				expand_log(logic, aff_sig, df, critical_sigs, explored_sigs, expand_search);
			}
		}
		else if ((lpm = ivl_nexus_ptr_lpm(nex_ptr))) {
			// Output nexus of LPM should be the same nexus of the aff_sig
			// otherwise the aff_sig is driving an input of the LPM
			if (ivl_lpm_q(lpm) == nex) {
				if (DEBUG_PRINTS){ printf("	input %d is a LPM device (type: %d).\n", i, ivl_lpm_type(lpm)); }
				expand_lpm(lpm, aff_sig, df, critical_sigs, explored_sigs, expand_search);
     		}
		} else if ((con = ivl_nexus_ptr_con(nex_ptr))) {
			// Ignore constants because they are always attached to IVL 
			// generated signals so they are connected to the graph when
			// a signal nexus pointer is discovered.
			continue;
		}
		else if ((swt = ivl_nexus_ptr_switch(nex_ptr))) {
			assert(false && "Switches are unsupported nexus pointers.\n");
		}
		else if ((bra = ivl_nexus_ptr_branch(nex_ptr))) {
			assert(false && "Branches are unsupported nexus pointers.\n");
		}
		else {
			assert(false && "? are unsupported nexus pointers.\n");	
		}
	}
}