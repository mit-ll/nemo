#include <ivl_target.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <regex>

#include "nemo.h"

void find_signal_dependencies(ivl_signal_t base_sig, Dot_File& df, set<ivl_signal_t>& explored_signals){
	ivl_net_const_t   con;           // temp IVL constant object
	ivl_signal_t      aff_sig;		 // temp IVL signal object
	set<ivl_signal_t> critical_sigs; // set of critical signals being propagated
	unsigned          depth_counter = SEARCH_DEPTH; // graph depth searched for dependencies

	// Add base signal to critical signal set
	if (!is_sig_explored(explored_signals, base_sig)){
		critical_sigs.insert(base_sig);
	}

	// Print node to graphviz dot file
	if ((con = is_const_local_sig(base_sig))) { 
		// signal was generated by the IVL compiler as
		// the output of a constant object, add constant node
		df.add_const_node(con);
	} else {
		// signal was found in netlist 
		df.add_node(base_sig);
	}

	// Determine the dependencies for critical base signals
	while (!critical_sigs.empty()){
		if (DEBUG_PRINTS){ print_signal_queues(critical_sigs, explored_signals); }

		//@TODO: Support more than 1 dimension vector
		//       Though it looks like it should be ok for OR1200
		aff_sig = *(critical_sigs.begin()); // get critical signal at beginning of queue/set
		assert(ivl_signal_packed_dimensions(aff_sig) <= 1 && "ERROR: cannot support multi-dimensional vectors.\n");
		
		if ((depth_counter-- > 0) && !is_sig_explored(explored_signals, aff_sig) && !ENUMERATE_ENTIRE_CIRCUIT){
			propagate_sig(aff_sig, df, critical_sigs, explored_signals, true);
		} else if (!is_sig_explored(explored_signals, aff_sig)) {
			propagate_sig(aff_sig, df, critical_sigs, explored_signals, false);
		}

		explored_signals.insert(aff_sig); // add signal to set of expanded signals
		critical_sigs.erase(aff_sig);     // remove from currently exploring set
	}
}

// Finds all security critical signals
void find_critical_sigs(ivl_scope_t* root_scopes, unsigned num_root_scopes, Dot_File& df){
	unsigned          num_critical_signals_found = 0;
	set<ivl_signal_t> explored_signals;

	for (unsigned i = 0; i < num_root_scopes; i++) {
		find_critical_scope_sigs(root_scopes[i], &num_critical_signals_found, df, explored_signals);
	}

	printf("\nNumber of Critical Signals Found: %d\n", num_critical_signals_found);
}

// Recurse through IVL scope objects (in this case only modules)
// to find all security critical signals
void find_critical_scope_sigs(ivl_scope_t scope, unsigned* num_sigs_found, Dot_File& df, set<ivl_signal_t>& explored_signals){
	//@TODO: Look more into dealing with scopes that are not modules
	if (ivl_scope_type(scope) != IVL_SCT_MODULE) {
		fprintf(stderr, "ERROR: cannot parse scope type (%d)\n", ivl_scope_type(scope));
		fprintf(stderr, "File: %s Line: %d\n", ivl_scope_file(scope), ivl_scope_lineno(scope));
		return;
	}

	// Rescurse into any submodules
	for (unsigned i = 0; i < ivl_scope_childs(scope); i++) {
		find_critical_scope_sigs(ivl_scope_child(scope, i), num_sigs_found, df, explored_signals);
	}

	// Enumerate all signals in each scope
	unsigned 	 num_scope_sigs = ivl_scope_sigs(scope);
	ivl_signal_t current_ivl_signal;

	// Propagate all signals
	for (unsigned idx = 0; idx < num_scope_sigs; idx++) {
		current_ivl_signal = ivl_scope_sig(scope, idx);
		// Only deal with non IVL generated (local) signals
		if (!is_ivl_generated_signal(current_ivl_signal)){
			// Check if signal is critical
			if (is_critical_sig(current_ivl_signal)){
				// Check if signal is arrayed
				if (ivl_signal_packed_dimensions(current_ivl_signal) > 1) {
					print_full_signal_name(current_ivl_signal);
					assert(false && "ERROR: Unsupported number of dimensions");
				}
				(*num_sigs_found)++;
				// Find critical signal dependencies
				print_signal_info(current_ivl_signal);
				find_signal_dependencies(current_ivl_signal, df, explored_signals);
			}
		}
	}
}

// Returns true if the signal has already been explored,
// i.e. the signal has already been passed to propa
bool is_sig_explored(set<ivl_signal_t>& explored_signals, ivl_signal_t sig){
	return (explored_signals.find(sig) != explored_signals.end());
}

// Returns true if the signal name found in the netlist 
// matches the regex for security critical signals
bool is_critical_sig(ivl_signal_t sig){
	regex  critical_regex(CRITICAL_SIG_REGEX, regex::grep);
	smatch matches;
	string signal_base_name = string(ivl_signal_basename(sig));
	
	regex_search(signal_base_name, matches, critical_regex);
	if (matches.size() > 0 || ENUMERATE_ENTIRE_CIRCUIT){
		return true;
	}
	return false;
}

// Returns true if the signal was generated by the IVL compiler.
// Returns false if the signal is found in the verilog netlist.
// NOTE: signals that are generated by the IVL compiler as outputs
// from constant net objects do not return true.
bool is_ivl_generated_signal(ivl_signal_t sig){
	//@TODO: Deal with larger vectors (more than one nexus)
	//@TODO: Figure out how more than one nexus works...
	const ivl_nexus_t nexus = ivl_signal_nex(sig, 0);
	const int         count = ivl_signal_array_count(sig);
	assert(count >= 0 && "ERROR: invalid nexus count\n");
	if (count > 1){
		fprintf(stderr, "ERROR: cannot process arrayed signal: %s\n", ivl_signal_basename(sig));
		fprintf(stderr, "File: %s Line: %d\n", ivl_signal_file(sig), ivl_signal_lineno(sig));
		exit(-1);
	}
	assert(nexus && "ERROR: invalid nexus for signal\n");

	// If option to include local signals is true, return false
	if (INCLUDE_LOCAL_SIGNALS){
		return false;
	}

	// Check if the signal was generated by IVL compiler front-end
	if (!ivl_signal_local(sig)){
		// signal was not generated by IVL compiler, return false
		return false;
	} else {
		// signal was generated by IVL compiler, check if it is 
		// the output of a constant net object
		if (is_const_local_sig(sig)){
			// signal is the output of a constant net object
			return false;
		} else{
			// signal is NOT the output of a constant net object
			return true;
		}
	}
}

// Returns true if the signal was generated by the IVL
// compiler as an output from a constant net object
ivl_net_const_t is_const_local_sig(ivl_signal_t sig){
	ivl_nexus_t 	nexus     = ivl_signal_nex(sig, 0);
	ivl_nexus_ptr_t nexus_ptr = NULL;
	ivl_net_const_t con       = NULL;

	// Check if local signal is connected to a constant
	for(unsigned i = 0; i < ivl_nexus_ptrs(nexus); i++){
		nexus_ptr = ivl_nexus_ptr(nexus, i);
		if ((con = ivl_nexus_ptr_con(nexus_ptr))){
			return con;
		}
	}

	return con;
}

bool is_sig_output(ivl_signal_t sig){
	if (ivl_signal_port(sig) == IVL_SIP_OUTPUT || ivl_signal_port(sig) == IVL_SIP_INOUT) {
		return true;
	} else {
		return false;
	}
}

bool are_both_signals_outputs(ivl_signal_t sig1, ivl_signal_t sig2) {
	if (is_sig_output(sig1) && is_sig_output(sig2)) {
		return true;
	} else {
		return false;
	}
}

bool is_sig1_output_and_sig2_not(ivl_signal_t sig1, ivl_signal_t sig2) {
	if (is_sig_output(sig1) && !is_sig_output(sig2)) {
		return true;
	} else {
		return false;
	}
}

void connect_signals(ivl_signal_t aff_sig, ivl_signal_t sig, set<ivl_signal_t>& critical_sigs, set<ivl_signal_t>& explored_sigs, Dot_File& df, bool expand_search) {
	pair<set<ivl_signal_t>::iterator, bool> insert_ret;
	
	// Do not connect local IVL compiler generated signals
	if (!is_ivl_generated_signal(sig) && !is_sig_explored(explored_sigs, sig)) {
		if (DEBUG_PRINTS){ printf("	input is a SIGNAL device (%s.%s).", ivl_scope_name(ivl_signal_scope(sig)), ivl_signal_basename(sig)); }
		df.add_connection(aff_sig, sig);

		// Only expand search to NON IVL generated signals,
		// excluding signals generated  by IVL from constants. 
		if (expand_search && !ivl_signal_local(sig)){
			insert_ret = critical_sigs.insert(sig);
			if (insert_ret.second && DEBUG_PRINTS){ printf(" Expanded."); }
		}
		if (DEBUG_PRINTS){ printf("\n"); }
	}
}

void print_signal_queues(set<ivl_signal_t>& critical_sigs, set<ivl_signal_t>& explored_signals){
	// Print critical signals set
	printf("Critical Signals: |");
	for (set<ivl_signal_t>::iterator it = critical_sigs.begin(); it != critical_sigs.end(); it++){
		printf("%s.%s| ", ivl_scope_name(ivl_signal_scope(*it)), ivl_signal_basename(*it));
	}
	printf("\n");

	// Print expanded signals set
	printf("Expanded Signals: |");
	for (set<ivl_signal_t>::iterator it = explored_signals.begin(); it != explored_signals.end(); it++){
		printf("%s.%s| ", ivl_scope_name(ivl_signal_scope(*it)), ivl_signal_basename(*it));
	}
	printf("\n\n");
}


// Prints the full signal name (<module name>.<signal name>) to stdin
void print_full_signal_name(ivl_signal_t sig){
	string tmp_scopename = string(ivl_scope_name(ivl_signal_scope(sig))); 
	string tmp_basename  = string(ivl_signal_basename(sig));
	string tmp_fullname  = string(tmp_scopename + string(".") + tmp_basename);
	printf("%-15s (%d)\n", tmp_fullname.c_str(), ivl_signal_width(sig));
}

// Prints all signal attributes to stdin
void print_signal_attrs(ivl_signal_t sig){
	for (unsigned idx = 0;  idx < ivl_signal_attr_cnt(sig);  idx += 1) {
		ivl_attribute_t atr = ivl_signal_attr_val(sig, idx);

		switch (atr->type) {
			case IVL_ATT_STR:
				printf("    %s = %s\n", atr->key, atr->val.str);
				break;
			case IVL_ATT_NUM:
				printf("    %s = %ld\n", atr->key, atr->val.num);
				break;
			case IVL_ATT_VOID:
				printf("    %s\n", atr->key);
				break;
			default:
				break;
		}
	}
}

// Prints all signal names and attributes to stdin
void print_signal_info(ivl_signal_t sig){
	print_full_signal_name(sig);
	print_signal_attrs(sig);
}

// *** "Main"/Entry Point *** of iverilog target
int target_design(ivl_design_t des) {
	ivl_scope_t* 		 roots = 0;     // root scopes of the design
	unsigned 		     num_roots;     // number of root scopes of the design
	Dot_File 			 df;  			 // output graph dot file

	// Variables to calculate runtime of this target module
	double  duration;	
	clock_t start = clock(); // Start timer

	// Initialize graphviz dot file
	df = Dot_File(ivl_design_flag(des, "-o"));
	df.init_graph();

	// Get root scopes of design
	ivl_design_roots(des, &roots, &num_roots);
	printf("\nNumber of Root Scopes: %d\n\n", num_roots);
	
	// Find all critical signals and dependencies in the design
	find_critical_sigs(roots, num_roots, df);

	// Close graphviz dot file
	df.save_graph();

	// Stop timer
	duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
	printf("\nExecution Time: %f (s)\n\n", duration);

	return 0;
}
