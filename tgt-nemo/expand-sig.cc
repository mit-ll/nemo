#include <cassert>
#include <cstdlib>
#include <cstdio>

#include "nemo.h"

void expand_sig(ivl_signal_t aff_sig, Dot_File& df, set<ivl_signal_t>& sigs_to_expand, set<ivl_signal_t>& explored_sigs, bool expand_search) {
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
		expand_std_cell_sigs(aff_sig, df, sigs_to_expand, explored_sigs, expand_search);
		return;
	} 

	// Iterate through the pointers in a given nexus
	for (unsigned int i = 0; i < ivl_nexus_ptrs(nex); i++) {
		nex_ptr = ivl_nexus_ptr(nex, i);
		assert(nex_ptr && "Error: expand_sig() -- nexus pointer is invalid.\n");
		if ((sig = ivl_nexus_ptr_sig(nex_ptr))) {
			// If the signal is different signal than itself --> add connection
			if (aff_sig != sig) {
				// if (DEBUG_PRINTS){ printf("		input is a SIGNAL device (%s.%s).\n", ivl_scope_name(ivl_signal_scope(sig)), ivl_signal_basename(sig)); }
				// aff_sig direction is OUTPUT; sig direction is OUTPUT
				if (ivl_signal_port(aff_sig) == IVL_SIP_OUTPUT && ivl_signal_port(sig) == IVL_SIP_OUTPUT){
					// Only connect the output signal of a child module to 
					// the output of a parent module. Otherwise, do not connect 
					// two outputs together.
					ivl_scope_t aff_sig_scope = ivl_signal_scope(aff_sig);
					ivl_scope_t sig_scope     = ivl_signal_scope(sig);

					if (ivl_scope_parent(sig_scope) == aff_sig_scope) {
						connect_signals(aff_sig, sig, sigs_to_expand, explored_sigs, df, expand_search);
					} else {
						printf("Warning <expand_sig()>: Ignoring connection of signal directions (output->output).\n");
					}
				}
				// aff_sig direction is OUTPUT; sig direction is INPUT
				else if (ivl_signal_port(aff_sig) == IVL_SIP_OUTPUT && ivl_signal_port(sig) == IVL_SIP_INPUT){
					// Only connect an input and output signal if they are
					// both contained in the same module
					if (ivl_signal_scope(aff_sig) == ivl_signal_scope(sig)){
						connect_signals(aff_sig, sig, sigs_to_expand, explored_sigs, df, expand_search);	
					} else {
						printf("Warning <expand_sig()>: Ignoring connection of signal directions (input->output).\n");
					}
				}
				// aff_sig direction is OUTPUT; sig direction is NONE
				else if (ivl_signal_port(aff_sig) == IVL_SIP_OUTPUT && ivl_signal_port(sig) == IVL_SIP_NONE){
					// Only connect a regular signal to an output if
					// they are in the same module.
					if (ivl_signal_scope(aff_sig) == ivl_signal_scope(sig)){
						connect_signals(aff_sig, sig, sigs_to_expand, explored_sigs, df, expand_search);
					} else {
						printf("Warning <expand_sig()>: Ignoring connection of signal directions (none->output).\n");
					}
				}
				// aff_sig direction is INPUT; sig direction is OUTPUT
				else if (ivl_signal_port(aff_sig) == IVL_SIP_INPUT && ivl_signal_port(sig) == IVL_SIP_OUTPUT){
					// Only connect an output and input signal if they are
					// contained in different modules
					if (ivl_signal_scope(aff_sig) != ivl_signal_scope(sig)){
						connect_signals(aff_sig, sig, sigs_to_expand, explored_sigs, df, expand_search);	
					} else {
						printf("Warning <expand_sig()>: Ignoring connection of signal directions (output->input).\n");
					}
				}
				// aff_sig direction is INPUT; sig direction is INPUT
				else if (ivl_signal_port(aff_sig) == IVL_SIP_INPUT && ivl_signal_port(sig) == IVL_SIP_INPUT){
					// Only connect an input signal of a parent module to 
					// the input signal of a child module. Otherwise, do not 
					// connect two inputs together.
					ivl_scope_t aff_sig_scope = ivl_signal_scope(aff_sig);
					ivl_scope_t sig_scope     = ivl_signal_scope(sig);

					if (ivl_scope_parent(aff_sig_scope) == sig_scope) {
						connect_signals(aff_sig, sig, sigs_to_expand, explored_sigs, df, expand_search);
					} else {
						printf("Warning <expand_sig()>: Ignoring connection of signal directions (input->input).\n");
					}
				}
				// aff_sig direction is INPUT; sig direction is NONE
				else if (ivl_signal_port(aff_sig) == IVL_SIP_INPUT && ivl_signal_port(sig) == IVL_SIP_NONE){
					// Only connect regular signal of a parent module to 
					// the input signal of a child module.
					ivl_scope_t aff_sig_scope = ivl_signal_scope(aff_sig);
					ivl_scope_t sig_scope     = ivl_signal_scope(sig);

					if (ivl_scope_parent(aff_sig_scope) == sig_scope) {
						connect_signals(aff_sig, sig, sigs_to_expand, explored_sigs, df, expand_search);
					} else {
						printf("Warning <expand_sig()>: Ignoring connection of signal directions (none->input).\n");
					}
				}
				// aff_sig direction is NONE; sig direction is OUTPUT
				else if (ivl_signal_port(aff_sig) == IVL_SIP_NONE && ivl_signal_port(sig) == IVL_SIP_OUTPUT){
					// Only connect output signal of a child module to 
					// a regular signal in a parent module.
					ivl_scope_t aff_sig_scope = ivl_signal_scope(aff_sig);
					ivl_scope_t sig_scope     = ivl_signal_scope(sig);

					if (ivl_scope_parent(sig_scope) == aff_sig_scope){
						connect_signals(aff_sig, sig, sigs_to_expand, explored_sigs, df, expand_search);	
					} else {
						printf("Warning <expand_sig()>: Ignoring connection of signal directions (output->none).\n");
					}
				}
				// aff_sig direction is NONE; sig direction is INPUT
				else if (ivl_signal_port(aff_sig) == IVL_SIP_NONE && ivl_signal_port(sig) == IVL_SIP_INPUT){
					// Only connect input signal of a module to 
					// a regular signal if they are contained in
					// the same module.
					if (ivl_signal_scope(aff_sig) == ivl_signal_scope(sig)){
						connect_signals(aff_sig, sig, sigs_to_expand, explored_sigs, df, expand_search);	
					} else {
						printf("Warning <expand_sig()>: Ignoring connection of signal directions (input->none).\n");
					}
				}
				// aff_sig direction is NONE; sig direction is NONE
				else if (ivl_signal_port(aff_sig) == IVL_SIP_NONE && ivl_signal_port(sig) == IVL_SIP_NONE){
					// Only connect two regular signals if they are 
					// in the same module
					if (ivl_signal_scope(aff_sig) == ivl_signal_scope(sig)){
						connect_signals(aff_sig, sig, sigs_to_expand, explored_sigs, df, expand_search);	
					} else {
						printf("Warning <expand_sig()>: Ignoring connection of signal directions (none->none).\n");
					}
				}
				// Raise error if none of these conditions hold
				else {
					assert(false && "Error <expand_sig()> -- invalid (inout) signal directions encountered.\n");
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
				expand_log(logic, aff_sig, df, sigs_to_expand, explored_sigs, expand_search);
			}
		}
		else if ((lpm = ivl_nexus_ptr_lpm(nex_ptr))) {
			// Output nexus of LPM should be the same nexus of the aff_sig
			// otherwise the aff_sig is driving an input of the LPM
			if (ivl_lpm_q(lpm) == nex) {
				if (DEBUG_PRINTS){ printf("	input %d is a LPM device (type: %d).\n", i, ivl_lpm_type(lpm)); }
				expand_lpm(lpm, aff_sig, df, sigs_to_expand, explored_sigs, expand_search);
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