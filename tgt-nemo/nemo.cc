#include <ivl_target.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include <string>
#include <regex>

#include "nemo.h"

void find_critical_sigs(vector<ivl_signal_t>& critical_sigs, ivl_scope_t* root_scopes, unsigned num_root_scopes){
	for (unsigned i = 0; i < num_root_scopes; i++) {
		find_sigs(root_scopes[i], critical_sigs);
	}
}

void find_sigs(ivl_scope_t scope, vector<ivl_signal_t>& critical_sigs){
	//@TODO: Look more into dealing with scopes that are not modules
	if (ivl_scope_type(scope) != IVL_SCT_MODULE) {
		fprintf(stderr, "ERROR: cannot parse scope type (%d)\n", ivl_scope_type(scope));
		fprintf(stderr, "File: %s Line: %d\n", ivl_scope_file(scope), ivl_scope_lineno(scope));
		return;
	}

	// Rescurse into any submodules
	for (unsigned i = 0; i < ivl_scope_childs(scope); i++) {
		find_sigs(ivl_scope_child(scope, i), critical_sigs);
	}

	// Enumerate all signals in each scope
	unsigned num_scope_sigs = ivl_scope_sigs(scope);
	ivl_signal_t current_ivl_signal;
	for (unsigned idx = 0; idx < num_scope_sigs; idx++) {
		current_ivl_signal = ivl_scope_sig(scope, idx);
		// Only deal with non local signals, or non-local const signals
		if (!ivl_signal_local(current_ivl_signal) || is_non_const_local_sig(current_ivl_signal)){
			// Check if signal is critical
			if (is_critical_sig(current_ivl_signal)){
				// Check if signal is arrayed
				if (ivl_signal_packed_dimensions(current_ivl_signal) > 1) {
					print_full_signal_name(current_ivl_signal);
					assert(false && "ERROR: Unsupported number of dimensions");
				}
				critical_sigs.push_back(current_ivl_signal);
			}
		}
	}
}

bool is_critical_sig(ivl_signal_t sig){
	regex  critical_regex(CRITICAL_SIG_REGEX);
	smatch matches;
	string signal_base_name = string(ivl_signal_basename(sig));
	
	regex_search(signal_base_name, matches, critical_regex);
	if (matches.size() > 0 || ENUMERATE_ENTIRE_CIRCUIT){
		return true;
	}
	return false;
}

bool is_non_const_local_sig(ivl_signal_t sig){
	ivl_nexus_t 	nexus     = ivl_signal_nex(sig, 0);
	ivl_nexus_ptr_t nexus_ptr = NULL;
	ivl_net_const_t con       = NULL;

	//@TODO: Deal with larger vectors (more than one nexus)
	//@TODO: Figure out how more than one nexus works...
	const int count = ivl_signal_array_count(sig);
	assert(count >= 0 && "ERROR: invalid nexus count\n");
	if (count > 1){
		fprintf(stderr, "ERROR: cannot process arrayed signal: %s\n", ivl_signal_basename(sig));
		fprintf(stderr, "File: %s Line: %d\n", ivl_signal_file(sig), ivl_signal_lineno(sig));
		exit(-1);
	}
	assert(nexus && "ERROR: invalid nexus for signal\n");

	// If the signal is not a local signal, return false
	if (!ivl_signal_local(sig)){
		return false;
	}

	// Check if local signal is connected to a constant
	for(unsigned i = 0; i < ivl_nexus_ptrs(nexus); i++){
		nexus_ptr = ivl_nexus_ptr(nexus, i);
		if ((con = ivl_nexus_ptr_con(nexus_ptr))){
			return true;
		}
	}

	return false;
}

void print_full_signal_name(ivl_signal_t sig){
	string tmp_scopename = string(ivl_scope_name(ivl_signal_scope(sig))); 
	string tmp_basename  = string(ivl_signal_basename(sig));
	string tmp_fullname  = string(tmp_scopename + string(".") + tmp_basename);
	printf("%s - Width: %d\n", tmp_fullname.c_str(), ivl_signal_width(sig));
}

void debug_print_critical_sigs(vector<ivl_signal_t>& critical_sigs){
	printf("Number of Critical Signals: %lu\n\n", critical_sigs.size());
	for (unsigned i = 0; i < critical_sigs.size(); i++){
		print_full_signal_name(critical_sigs[i]);
	}
	printf("\n");
}

// *** "Main"/Entry Point *** of iverilog target
int target_design(ivl_design_t des) {
	ivl_scope_t* 		 roots = 0;     // root scopes of the design
	unsigned 		     num_roots;     // number of root scopes of the design
	vector<ivl_signal_t> critical_sigs; // all critical signals in the design
	Dot_File 			 df;  			// output graph dot file

	// Variables to calculate runtime of this target module
	double  duration;	
	clock_t start = std::clock(); // Start timer

	// Get root scopes of design
	ivl_design_roots(des, &roots, &num_roots);
	printf("\nNumber of Root Scopes: %d\n", num_roots);
	
	// Find all critical signals in the design
	find_critical_sigs(critical_sigs, roots, num_roots);
	debug_print_critical_sigs(critical_sigs);

	// Write all nodes to the graph dot file
	df = Dot_File(ivl_design_flag(des, "-o"));
	df.init_graph();
	for (vector<ivl_signal_t>::iterator it = critical_sigs.begin(); it != critical_sigs.end(); ++it){
		df.add_node(*it);
	}

	// Determine the dependencies for critical connections
	for (vector<ivl_signal_t>::iterator it = critical_sigs.begin(); it != critical_sigs.end(); ++it){
		//@TODO: Support more than 1 dimension vector
		//       Though it looks like it should be ok for OR1200
		assert(ivl_signal_packed_dimensions(*it) <= 1 && "ERROR: cannot support multi-dimensional vectors.\n");
		propagate_sig(*it, df);
	}

	// Close graph dot file
	df.save_graph();
	// Stop timer
	duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
	printf("\nExecution Time: %f (s)\n\n", duration);

	return 0;
}
