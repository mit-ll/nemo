#include <ivl_target.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <limits>
#include <utility>
#include <vector>

#include "ttb.h"
// #include "ttb_dot_file.h"

using namespace std;
using namespace nemo_pb;

// *** "Main"/Entry Point *** of iverilog target
int target_design(ivl_design_t des) {
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	ivl_scope_t* 		 roots = 0;   // root scopes of the design
	unsigned int 		 num_roots;   // number of root scopes of the design
	Nemo_Design* 	     nemo_des;	  // entire circuit graph in PB exportable form
	vector<ivl_signal_t> ivl_sigs;    // all ivl signals
	// Dot_File 			 df;	  // output dot file to represent signal graph

	// Variables to calculate runtime of this target module
	std::clock_t start;
	double duration;

	// Start timer
	start = std::clock();

	// Open dot graph output file 
	// df = Dot_File(ivl_design_flag(des, "-o"));

	// Get root scopes of design
	ivl_design_roots(des, &roots, &num_roots);
	printf("\nNumber of Root Scopes: %d\n", num_roots);
	nemo_des = new Nemo_Design(roots, num_roots, ivl_design_flag(des, "-o"));
	
	//------------------------------------------------------------------------
	// Find all the signals in the design
	nemo_des->load_design_signals();
	
	// Save Signal Protobufs
	if (!nemo_des->were_signals_loaded_from_pb()){
		nemo_des->serialize_nemo_signal_pbs();	
		nemo_des->delete_nemo_sigs();
		nemo_des->delete_spliced_nemo_sigs();
	}

	//------------------------------------------------------------------------
	// Determing connections for all signals
	nemo_des->load_design_connections();

	// Save Connection Protobufs
	if (!nemo_des->were_connections_loaded_from_pb()){
		nemo_des->serialize_nemo_connection_pbs();
		// nemo_des->serialize_spliced_nemo_signal_pbs();
	}

	//------------------------------------------------------------------------
	// Print out dot graph
	// df.print_graph(nemo_des);

	// Stop timer
	duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
	printf("Execution Time: %f (s)\n\n", duration);
	
	delete nemo_des;

	// Delete all global objects allocated by libprotobuf
	google::protobuf::ShutdownProtobufLibrary();

	return 0;
}
