#include <cassert>
#include <cstdlib>
#include <cstdio>

#include "ivl_target.h"
#include "nemo.h"

void propagate_sig(ivl_signal_t aff_sig, Dot_File& df, vector<ivl_signal_t>& critical_sigs, bool expand_search) {
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
			// If the signal is different signal --> add connection
			if (aff_sig != sig) {
				// Only propagate the output signal of a child module to 
				// the output of a parent module. Propagate all non-output 
				// signals to other signals.
				if ((ivl_signal_port(aff_sig) == IVL_SIP_OUTPUT && ivl_signal_port(sig) != IVL_SIP_OUTPUT) ||
				   (ivl_signal_port(aff_sig) != IVL_SIP_OUTPUT)){
					// Do not propagate local IVL compiler generated signals
					if (!is_ivl_generated_signal(sig)){
						if (DEBUG_PRINTS){ printf("	input %d is a SIGNAL device (%s).\n", i, ivl_signal_basename(sig)); }
						df.add_connection(aff_sig, sig);
						if (expand_search){
							critical_sigs.push_back(sig);
						}
					}
				} else if (ivl_signal_port(aff_sig) == IVL_SIP_OUTPUT && ivl_signal_port(sig) == IVL_SIP_OUTPUT) {
					ivl_scope_t aff_sig_scope = ivl_signal_scope(aff_sig);
					ivl_scope_t sig_scope     = ivl_signal_scope(sig);
					// If the aff_sig scope is the parent of the sig scope,
					// add a connection between the signals.
					if (ivl_scope_parent(sig_scope) == aff_sig_scope){
						// Do not propagate local IVL compiler generated signals
						if (!is_ivl_generated_signal(sig)) {
							if (DEBUG_PRINTS){ printf("	input %d is a SIGNAL device (%s).\n", i, ivl_signal_basename(sig)); }
							df.add_connection(aff_sig, sig);
							if (expand_search){
								critical_sigs.push_back(sig);
							}
						}
					}
				}
			}
		}
		else if ((logic = ivl_nexus_ptr_log(nex_ptr))){ 
			// Pin-0 is the output of every logic device.
			// 
			// Output nexus of the logic device should be the same nexus as the aff_sig
			// otherwise the aff_sig is driving an input of the LPM
			if (ivl_logic_pin(logic, 0) == nex) {
				if (DEBUG_PRINTS){ printf("	input %d is a LOGIC device.\n", i); }
				propagate_log(logic, aff_sig, df, critical_sigs, expand_search);
			}
		}
		else if ((lpm = ivl_nexus_ptr_lpm(nex_ptr))){
			// Output nexus of LPM should be the same nexus of the aff_sig
			// otherwise the aff_sig is driving an input of the LPM
			if (ivl_lpm_q(lpm) == nex) {
				if (DEBUG_PRINTS){ printf("	input %d is a LPM device (type: %d).\n", i, ivl_lpm_type(lpm)); }
				propagate_lpm(lpm, aff_sig, df, critical_sigs, expand_search);
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