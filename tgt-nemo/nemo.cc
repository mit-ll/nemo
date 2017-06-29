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

bool is_critical_sig(ivl_signal_t sig){
	regex  critical_regex(CRITICAL_SIG_REGEX);
	smatch matches;
	string signal_base_name = string(ivl_signal_basename(sig));
	
	regex_search(signal_base_name, matches, critical_regex);
	if (matches.size() > 0){
		return true;
	}
	return false;
}

void print_full_signal_name(ivl_signal_t sig){
	string tmp_scopename = string(ivl_scope_name(ivl_signal_scope(sig))); 
	string tmp_basename  = string(ivl_signal_basename(sig));
	string tmp_fullname  = string(tmp_scopename + tmp_basename);
	printf("%s\n", tmp_fullname.c_str());
}

void debug_print_critical_sigs(vector<ivl_signal_t>& critical_sigs){
	printf("Number of Critical Signals: %lu\n\n", critical_sigs.size());

	for (unsigned i = 0; i < critical_sigs.size(); i++){
		print_full_signal_name(critical_sigs[i]);
	}
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
	printf("Execution Time: %f (s)\n\n", duration);

	return 0;
}
