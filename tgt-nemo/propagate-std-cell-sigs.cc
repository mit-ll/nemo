#include <cassert>
#include <cstdlib>
#include <cstdio>

#include "ivl_target.h"
#include "nemo.h"

void propagate_std_cell_sigs(ivl_scope_t std_cell_scope, Dot_File& df){
	ivl_signal_t curr_input_sig  = NULL;
	ivl_signal_t curr_output_sig = NULL;
	unsigned     num_scope_sigs  = ivl_scope_sigs(std_cell_scope);
	unsigned 	 num_scope_ports = ivl_scope_ports(std_cell_scope);

	// Ensure number of signals is equal to the number of ports,
	// i.e. the std cell module definition is just a template of
	// with only input and output signals defined
	if (num_scope_sigs != num_scope_ports) {
		printf("Number of signals (%d) does not match number of ports (%d)\n", num_scope_sigs, num_scope_ports);
		assert(num_scope_sigs == num_scope_ports);
	}

	// Iterate over module output signals and connect all
	// input signals to each output signal.
	for (unsigned i = 0; i < num_scope_sigs; i++) {
		curr_output_sig = ivl_scope_sig(std_cell_scope, i);
		
		// Add all connections
		if (ivl_signal_port(curr_output_sig) == IVL_SIP_OUTPUT || ivl_signal_port(curr_output_sig) == IVL_SIP_INOUT) {
			if (DEBUG_PRINTS){ printf("	output %d is signal %s\n", i, ivl_signal_basename(curr_output_sig)); }
			
			for (unsigned j = 0; j < num_scope_sigs; j++) {
				curr_input_sig = ivl_scope_sig(std_cell_scope, j);
				if (ivl_signal_port(curr_input_sig) == IVL_SIP_INPUT || ivl_signal_port(curr_input_sig) == IVL_SIP_INOUT) {
					if (DEBUG_PRINTS){ printf("		input %d is signal %s\n", j, ivl_signal_basename(curr_input_sig)); }
					df.add_connection(curr_output_sig, curr_input_sig);
				}
			}
		}
	}
}