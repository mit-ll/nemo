#include <ivl_target.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <limits>
#include <utility>
#include <vector>

#include "ttb.h"
#include "ttb_signal.h"
#include "ttb_dot_file.h"


void print_connection(const TTB_Signal& aff_sig, const TTB_Signal& sig, std::vector<connection_t>& connections) {
	connections.push_back(connection_t(aff_sig, sig));
}

// Finds all the sigs in scope
void find_sigs(ivl_scope_t scope, std::vector<TTB_Signal>& sigs) {
	//@TODO: Look more into dealing with scopes that are tasks or functions
	if ( ivl_scope_type(scope) != IVL_SCT_MODULE && ivl_scope_type(scope) != IVL_SCT_BEGIN && ivl_scope_type(scope) != IVL_SCT_TASK ) {
		fprintf(stderr, "ERROR: Cannot parse scope type (%d)\n", ivl_scope_type(scope));
		fprintf(stderr, "File: %s Line: %d\n", ivl_scope_file(scope), ivl_scope_lineno(scope));
		return;
	}

	// Rescurse into any submodules
	for (unsigned int idx = 0; idx < ivl_scope_childs(scope); idx++) {
		find_sigs(ivl_scope_child(scope, idx), sigs);
	}

	unsigned num_sigs = ivl_scope_sigs(scope);
	TTB_Signal sig;
	for (unsigned idx = 0; idx < num_sigs; idx++) {
		sig = ivl_scope_sig(scope, idx);
		sigs.push_back(TTB_Signal(sig));
	}
}

int show_process(ivl_process_t net, void* x) {
	std::pair<sig_map_t*, std::vector<connection_t>* >* data = (std::pair<sig_map_t*, std::vector<connection_t>* >*)x;
	sig_map_t* ffs = data->first;
	std::vector<connection_t>* connections = data->second;

	/* Error Checking */
	switch(ivl_process_type(net)) {
		case IVL_PR_INITIAL:
			fprintf(stderr, "WARNING: Skipping Initial Block\n");
			fprintf(stderr, "File: %s Line: %d\n", ivl_process_file(net), ivl_process_lineno(net));
			return 0;
			break;

		case IVL_PR_FINAL:
			fprintf(stderr, "WARNING: Skipping Final Block\n");
			fprintf(stderr, "File: %s Line: %d\n", ivl_process_file(net), ivl_process_lineno(net));
			return 0;
			break;

		case IVL_PR_ALWAYS:
			if (ivl_process_analog(net)) {
				fprintf(stderr, "ERROR: Analog Always Blocks Not Supported\n");
				fprintf(stderr, "File: %s Line: %d\n", ivl_process_file(net), ivl_process_lineno(net));
				return 3;
			}
			break;

		default:
			fprintf(stderr, "ERROR: Unknown Process\n");
			fprintf(stderr, "File: %s Line: %d\n", ivl_process_file(net), ivl_process_lineno(net));
			return 4;
			break;
	} /* End Error Checking */

	// At this point, should only have always blocks
	ivl_statement_t stmt = ivl_process_stmt(net);

	int ret = process_statement(stmt, *ffs, *connections);
	return ret;
}

// *** "Main"/Entry Point *** of iverilog target
int target_design(ivl_design_t des) {
	int          			  rc;
	ivl_scope_t* 			  roots = 0;
	unsigned int 			  num_roots;
	std::vector<TTB_Signal>   sigs;
	sig_map_t 				  sig_map;
	std::vector<connection_t> connections;
	Dot_File 				  df;

	// Open dot graph output file 
	df = Dot_File(ivl_design_flag(des, "-o"));

	// Get root scopes of design
	ivl_design_roots(des, &roots, &num_roots);

	// Find all the signals in the design
	for ( unsigned i = 0; i < num_roots; i++ ) {
		find_sigs( roots[i], sigs );
	}

	// Create map data struct between signal names and pointer to corresponding TTB_signal object
	for ( std::vector<TTB_Signal>::iterator it = sigs.begin(); it != sigs.end(); ++it ) {
		sig_map[ it->name() ] = &( *it );
	}

	// Determing connections for all signals
	for ( std::vector<TTB_Signal>::iterator it = sigs.begin(); it != sigs.end(); ++it ) {
		//@TODO: Support more than 1 dimension vector
		//       Though it looks like it should be ok for OR1200
		assert(ivl_signal_packed_dimensions(it->get_sig()) <= 1);
		propagate_sig(*it, connections);
	}

	// We have to put them in a pair b/c I have to pass a single pointer
	std::pair<sig_map_t*, std::vector<connection_t>* > data(&sig_map, &connections);

	// Goes through all assignments in always blocks
	// Print out all the assignments
	rc = ivl_design_process(des, show_process, &data);
	if (rc != 0) {
		fprintf(stderr, "ERROR: Could not process always blocks\n");
		return rc;
	}

	// Print out dot graph
	df.print_graph(sigs, connections);

	return 0;
}
