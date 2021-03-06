// -----------------------------------------------------------------------------
// File:        nemo.cc
// Author:      Timothy Trippel
// Affiliation: MIT Lincoln Laboratory
// -----------------------------------------------------------------------------

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
// #include <regex>

#include "nemo.h"

void find_all_signal_dependencies(vector<ivl_signal_t>& critical_sigs, Dot_File& df, unsigned search_depth){
	set<ivl_signal_t> expanded_signals;

	for (unsigned i = 0; i < critical_sigs.size(); i++) {
		find_signal_dependencies(critical_sigs[i], df, expanded_signals, search_depth);
	}
}

void find_signal_dependencies(ivl_signal_t critical_sig, Dot_File& df, set<ivl_signal_t>& expanded_signals, unsigned search_depth){
	ivl_net_const_t   con;            // temp IVL constant object
	ivl_signal_t      aff_sig;		  // temp IVL signal object
	set<ivl_signal_t> sigs_to_expand; // set of signals to be expanded
	int               depth_counter = search_depth; // graph depth searched for dependencies

	// Add base signal to signal-to-be-expanded set
	if (!is_sig_expanded(expanded_signals, critical_sig)){
		sigs_to_expand.insert(critical_sig);
	}

	// Print node to graphviz dot file
	if ((con = is_const_local_sig(critical_sig))) { 
		// signal was generated by the IVL compiler as
		// the output of a constant object, add constant node
		df.add_const_node(con);
	} else {
		// signal was found in netlist 
		df.add_node(critical_sig);
	}

	// Determine the dependencies for critical base signals
	while (!sigs_to_expand.empty()){
		if (DEBUG_PRINTS){ print_signal_queues(sigs_to_expand, expanded_signals); }

		//@TODO: Support more than 1 dimension vector
		//       Though it looks like it should be ok for OR1200
		aff_sig = *(sigs_to_expand.begin()); // get critical signal at beginning of queue/set
		assert(ivl_signal_packed_dimensions(aff_sig) <= 1 && "ERROR: cannot support multi-dimensional vectors.\n");
		
		if (!ENUMERATE_ENTIRE_CIRCUIT && !is_sig_expanded(expanded_signals, aff_sig)){
			// Do NOT enumerate the entire circuit
			if (depth_counter > 0){
				depth_counter -= expand_sig(aff_sig, df, sigs_to_expand, expanded_signals, true);
			} else {
				depth_counter -= expand_sig(aff_sig, df, sigs_to_expand, expanded_signals, false);
			}
		} else if (!is_sig_expanded(expanded_signals, aff_sig)) {
			// Enumerate the entire circuit, i.e. every signal is critical
			expand_sig(aff_sig, df, sigs_to_expand, expanded_signals, false);
		}

		expanded_signals.insert(aff_sig); // add signal to set of expanded signals
		sigs_to_expand.erase(aff_sig);    // remove from currently expanding set
	}
}

// Finds all security critical signals
void find_critical_sigs(ivl_scope_t* root_scopes, unsigned num_root_scopes, vector<ivl_signal_t>& critical_sigs, const char* regex_str){
	unsigned num_critical_signals_found = 0;

	for (unsigned i = 0; i < num_root_scopes; i++) {
		if (!ENUMERATE_ENTIRE_CIRCUIT){
			printf("Critical signals in root scope %s:\n", ivl_scope_basename(root_scopes[i]));
		}
		find_critical_scope_sigs(root_scopes[i], &num_critical_signals_found, critical_sigs, regex_str);
	}

	printf("\nNumber of critical signals found: %d\n\n", num_critical_signals_found);
}

// Recurse through IVL scope objects (in this case only modules)
// to find all security critical signals
void find_critical_scope_sigs(ivl_scope_t scope, unsigned* num_sigs_found, vector<ivl_signal_t>& critical_sigs, const char* regex_str){
	//@TODO: Look more into dealing with scopes that are not modules
	if (ivl_scope_type(scope) != IVL_SCT_MODULE) {
		fprintf(stderr, "ERROR: cannot parse scope type (%d)\n", ivl_scope_type(scope));
		fprintf(stderr, "File: %s Line: %d\n", ivl_scope_file(scope), ivl_scope_lineno(scope));
		return;
	}

	// Rescurse into any submodules
	for (unsigned i = 0; i < ivl_scope_childs(scope); i++) {
		find_critical_scope_sigs(ivl_scope_child(scope, i), num_sigs_found, critical_sigs, regex_str);
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
			if (is_critical_sig(current_ivl_signal, regex_str)){
				// Check if signal is arrayed
				if (ivl_signal_packed_dimensions(current_ivl_signal) > 1) {
					print_full_signal_name(current_ivl_signal);
					assert(false && "ERROR: Unsupported number of dimensions");
				}
				(*num_sigs_found)++;
				if (!ENUMERATE_ENTIRE_CIRCUIT){
					print_signal_info(current_ivl_signal);
				}
				// Find critical signal dependencies
				critical_sigs.push_back(current_ivl_signal);
			}
		}
	}
}

// Returns true if the signal has already been explored,
// i.e. the signal has already been passed to expand_sig().
bool is_sig_expanded(set<ivl_signal_t>& explored_signals, ivl_signal_t sig){
	return (explored_signals.find(sig) != explored_signals.end());
}

// Returns true if the signal name found in the netlist 
// matches the regex for security critical signals
bool is_critical_sig(ivl_signal_t sig, const char* regex_str){
	if (ENUMERATE_ENTIRE_CIRCUIT) {
		return true;
	}

	if (strstr(ivl_signal_basename(sig), regex_str)){
		return true;
	}
	return false;	
	// regex  critical_regex(regex_str, regex::grep);
	// smatch matches;
	// string signal_base_name = string(ivl_signal_basename(sig));
	
	// regex_search(signal_base_name, matches, critical_regex);
	// if (matches.size() > 0){
	// 	return true;
	// }
	// return false;
}

// Returns true if the signal is a CLK signal.
// Returns false if the signal is not.
bool is_clk_sig(ivl_signal_t sig){
	if (IGNORE_CLK_SIGNALS && (strcmp(ivl_signal_basename(sig), SEQUENTIAL_CLK_PIN_NAME) == 0)){
		return true;
	} else {
		return false;
	}
}

// Returns true if the child module is contained
// within the parent module. Else, returns false.
bool is_child_of_parent_module(ivl_scope_t child_scope, ivl_scope_t parent_scope){
	ivl_scope_t child_parent_scope = ivl_scope_parent(child_scope);
	if (child_parent_scope == NULL){
		return false;
	} else if (child_parent_scope == parent_scope){
		return true;
	} else {
		return is_child_of_parent_module(child_parent_scope, parent_scope);
	}
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

void connect_signals(ivl_signal_t aff_sig, ivl_signal_t sig, set<ivl_signal_t>& sigs_to_expand, set<ivl_signal_t>& explored_sigs, Dot_File& df, bool expand_search) {
	pair<set<ivl_signal_t>::iterator, bool> insert_ret;
	
	// Do not connect local IVL compiler generated signals
	if (!is_ivl_generated_signal(sig)) {
		if (DEBUG_PRINTS){ printf("	input is a SIGNAL device (%s.%s).", ivl_scope_name(ivl_signal_scope(sig)), ivl_signal_basename(sig)); }
		df.add_connection(aff_sig, sig);

		// Only continue to expand signals that have not been expanded. 
		// Do not expand IVL generated signals either.
		if (expand_search && !ivl_signal_local(sig) && !is_sig_expanded(explored_sigs, sig)){
			insert_ret = sigs_to_expand.insert(sig);
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
	printf("	%-25s (%d)\n", tmp_fullname.c_str(), ivl_signal_width(sig));
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
	// Nemo Configurations
	const char* critical_sig_regex = DEFAULT_CRITICAL_SIG_REGEX; // critical-signal prefix
	unsigned    search_depth       = DEFAULT_SEARCH_DEPTH;       // critical-signal search depth
	
	// Verilog Structural Info
	ivl_scope_t* roots     = 0; // root scopes of the design
	unsigned     num_roots = 0; // number of root scopes of the design
	
	// Data Structures
	Dot_File 		     df;  		    // output graph dot file
	vector<ivl_signal_t> critical_sigs; // critical signals found in a design

	// Variables to calculate runtime of this target module
	double  duration;
	clock_t start = clock(); // Start timer

	// Get critical signal prefix input
	critical_sig_regex = ivl_design_flag(des, "nemo_sig_prefix");
	if (strstr("", critical_sig_regex)) {
		critical_sig_regex = DEFAULT_CRITICAL_SIG_REGEX;
	}
	printf("\nCritical Signal Prefix: %s\n", critical_sig_regex);

	// Get critical signal search depth input
	sscanf(ivl_design_flag(des, "nemo_search_depth"), "%u", &search_depth);
	printf("Signal Search Depth:    %u\n", search_depth);

	// Initialize graphviz dot file
	df = Dot_File(ivl_design_flag(des, "-o"));
	df.init_graph();

	// Get root scopes of design
	ivl_design_roots(des, &roots, &num_roots);
	printf("Number of root scopes:  %d\n\n", num_roots);

	// Find all critical signals and dependencies in the design
	find_critical_sigs(roots, num_roots, critical_sigs, critical_sig_regex);

	// Find signal dependencies of critical sigs
	find_all_signal_dependencies(critical_sigs, df, search_depth);

	// Close graphviz dot file
	df.save_graph();

	// Stop timer
	duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
	printf("Execution Time: %f (s)\n\n", duration);

	return 0;
}
