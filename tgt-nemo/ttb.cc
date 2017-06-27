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
#include "ttb_dot_file.h"
#include "nemo_signals.pb.h"

using namespace std;
using namespace nemo_pb;

// *** "Main"/Entry Point *** of iverilog target
int target_design(ivl_design_t des) {
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	ivl_scope_t* 		 roots = 0;   // root scopes of the design
	unsigned int 		 num_roots;   // number of root scopes of the design
	Nemo_Design 	     nemo_des;	  // entire circuit graph in PB exportable form
	vector<ivl_signal_t> ivl_sigs;    // all ivl signals
	vector<connection_t> connections; // all pair-wise nemo signal connections
	// Dot_File 			 df;		   // output dot file to represent signal graph

	// Variables to calculate runtime of this target module
	std::clock_t start;
	double duration;

	// Start timer
	start = std::clock();

	// Open dot graph output file 
	// df = Dot_File(ivl_design_flag(des, "-o"));

	// Get root scopes of design
	ivl_design_roots(des, &roots, &num_roots);

	// Find all the signals in the design
	printf("Enumerating all signals in the design ...\n");
	nemo_des = Nemo_Design(roots, num_roots);
	ivl_sigs = nemo_des.get_ivl_sigs();
	printf("Done enumerating signals.\n\n");

	// Determing connections for all signals
	printf("Elaborating all signals connections ...\n");
	for (vector<ivl_signal_t>::iterator it = ivl_sigs.begin(); it != ivl_sigs.end(); ++it){
		//@TODO: Support more than 1 dimension vector
		//       Though it looks like it should be ok for OR1200
		assert(ivl_signal_packed_dimensions(*it) <= 1);
		propagate_sig(*it, nemo_des);
	}
	printf("Done elaborating connections.\n\n");

	// Save Protobufs
	if (!nemo_des.loaded_from_pb()){
		nemo_des.save_pb();	
	}

	// Print out dot graph
	// df.print_graph(nemo_des);

	// Stop timer
	duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
	printf("Execution Time: %f (s)\n\n", duration);
	
	// Delete all global objects allocated by libprotobuf
	google::protobuf::ShutdownProtobufLibrary();

	return 0;
}
