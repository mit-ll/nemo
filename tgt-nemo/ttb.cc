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

// void print_connection(const TTB_Signal& aff_sig, const TTB_Signal& sig, std::vector<connection_t>& connections) {
// 	connections.push_back(connection_t(aff_sig, sig));
// }

// *** "Main"/Entry Point *** of iverilog target
int target_design(ivl_design_t des) {
	// Verify that the version of the library that we linked against is
	// compatible with the version of the headers we compiled against.
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	ivl_scope_t* 			  roots = 0;   // root scopes of the design
	unsigned int 			  num_roots;   // number of root scopes of the design
	Nemo_Design 			  nemo_des;	   // all signals in the design
	// std::vector<connection_t> connections; // all pair-wise signal connections
	// Dot_File 				  df;		   // output dot file to represent signal graph

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
	nemo_des = Nemo_Design(roots, num_roots);

	// Determing connections for all signals
	// for ( std::vector<ivl_signal_t>::iterator it = ivl_sigs.begin(); it != ivl_sigs.end(); ++it ) {
	// 	//@TODO: Support more than 1 dimension vector
	// 	//       Though it looks like it should be ok for OR1200
	// 	assert(ivl_signal_packed_dimensions(*it) <= 1);
	// 	propagate_sig(*it, sigs, connections);
	// }

	// // We have to put them in a pair b/c I have to pass a single pointer
	// std::pair<sig_map_t*, std::vector<connection_t>* > data(&sig_map, &connections);

	// Save Protobufs
	if (!nemo_des.loaded_from_pb()){
		nemo_des.save_pb();	
	}

	// Print out dot graph
	// df.print_graph(sigs, connections);

	// Stop timer
	duration = (std::clock() - start) / (double) CLOCKS_PER_SEC;
	printf("Execution Time: %f (s)\n", duration);
	
	// Delete all global objects allocated by libprotobuf
	google::protobuf::ShutdownProtobufLibrary();

	return 0;
}
