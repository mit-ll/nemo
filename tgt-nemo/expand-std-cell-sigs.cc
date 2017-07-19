#include <cassert>
#include <cstdlib>
#include <cstdio>

#include "nemo.h"

void expand_std_cell_sigs(ivl_signal_t aff_sig, Dot_File& df, set<ivl_signal_t>& sigs_to_expand, set<ivl_signal_t>& explored_sigs, bool expand_search){
	// If aff_signal is an input to the std cell module stub, return
	if (ivl_signal_port(aff_sig) == IVL_SIP_INPUT){
		return;
	} else if (ivl_signal_port(aff_sig) == IVL_SIP_INOUT || ivl_signal_port(aff_sig) == IVL_SIP_NONE) {
		assert(false && "Error <expand_std_cell_sigs()>: invalid (inout) port directions in std cell.\n");
	}

	ivl_signal_t curr_input_sig  = NULL;
	ivl_scope_t  curr_scope      = ivl_signal_scope(aff_sig);
	unsigned     num_scope_sigs  = ivl_scope_sigs(ivl_signal_scope(aff_sig));
	unsigned 	 num_scope_ports = ivl_scope_ports(ivl_signal_scope(aff_sig));

	pair<set<ivl_signal_t>::iterator, bool> insert_ret;

	// Ensure number of signals is equal to the number of ports,
	// i.e. the std cell module definition is just a template of
	// with only input and output signals defined
	if (num_scope_sigs != num_scope_ports) {
		printf("Number of signals (%d) does not match number of ports (%d)\n", num_scope_sigs, num_scope_ports);
		assert(num_scope_sigs == num_scope_ports);
	}

	if (DEBUG_PRINTS){ printf("STD cell module found, expand all inputs to ouput (%s.%s).\n", ivl_scope_name(ivl_signal_scope(aff_sig)), ivl_signal_basename(aff_sig)); }

	// Iterate over module input signals and connect all
	// input signals to the aff_sig (output signal).
	for (unsigned i = 0; i < num_scope_sigs; i++) {
		curr_input_sig = ivl_scope_sig(curr_scope, i);
		if (ivl_signal_port(curr_input_sig) == IVL_SIP_INPUT) {
			if (DEBUG_PRINTS){ printf("		input %d is a SIGNAL device (%s.%s).", i, ivl_scope_name(ivl_signal_scope(curr_input_sig)), ivl_signal_basename(curr_input_sig)); }
			df.add_connection(aff_sig, curr_input_sig);

			// Only expand search to NON IVL generated signals,
			// excluding signals generated  by IVL from constants. 
			if (expand_search && !ivl_signal_local(curr_input_sig) && !is_sig_expanded(explored_sigs, curr_input_sig)){
				insert_ret = sigs_to_expand.insert(curr_input_sig);
				if (insert_ret.second && DEBUG_PRINTS){ printf(" Expanded."); }
			}
			if (DEBUG_PRINTS){ printf("\n"); }
		} else if (ivl_signal_port(curr_input_sig) == IVL_SIP_INOUT || ivl_signal_port(curr_input_sig) == IVL_SIP_NONE) {
			printf("Warning <expand_std_cell_sigs()>: ignoring inout or none type signals.\n");
		}
	}
}