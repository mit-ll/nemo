#include <cstdio>
#include <cstdlib>
#include <vector>

#include <ivl_target.h>
#include "nemo_design.h"
#include "ttb_dot_file.h"

Dot_File::Dot_File(): path(NULL), file_ptr(NULL){}

Dot_File::Dot_File(const char* p): file_ptr(NULL){
	set_path(p);
}

void Dot_File::set_path(const char* p){
	path = p;
}

const char* Dot_File::get_path(){
	return path;
}

FILE* Dot_File::get_file_ptr(){
	return file_ptr;
}

void Dot_File::open_file(){
	file_ptr = fopen(path, "w");
	if (!file_ptr) {
		printf("ERROR: Could not open file %s\n", path ? path : "stdout");
		exit(-4);
	}
}

void Dot_File::close_file(){
	fclose(file_ptr);
	file_ptr = NULL;
}

void Dot_File::print_graph(Nemo_Design& nemo_des){
	// Open output file
	open_file();

	// Print out nodes in the graph
	fprintf(file_ptr, "digraph G {\n");
	for (vector<Nemo_Signal>::iterator it = nemo_des.get_nemo_sigs().begin(); it != nemo_des.get_nemo_sigs().end(); ++it ) {
		// Make local signals just points instead of full nodes
		if (it->signal_local()) {
			// Graph node is a circuit ...
			fprintf(file_ptr, "\t\"%s\" [shape=point, label=\"%s[%lu:%lu]\"];\n", it->full_name().c_str(), it->full_name().c_str(), it->get_msb(), it->get_lsb());
		} else if (it->is_ff()) {
			// Graph node is a circuit flip-flop
			fprintf(file_ptr, "\t\"%s\" [shape=square, label=\"%s[%lu:%lu]\"]; /* Flip Flop */\n", it->full_name().c_str(), it->full_name().c_str(), it->get_msb(), it->get_lsb());
		} else if (it->is_input()) {
			// Graph node is a circuit input
			fprintf(file_ptr, "\t\"%s\" [shape=none, label=\"%s[%lu:%lu]\"]; /* Input */\n", it->full_name().c_str(), it->full_name().c_str(), it->get_msb(), it->get_lsb());
		} else {
			// Graph node is circuit output, or anything else 
			fprintf(file_ptr, "\t\"%s\" [shape=ellipse, label=\"%s[%lu:%lu]\"];\n", it->full_name().c_str(), it->full_name().c_str(), it->get_msb(), it->get_lsb());
		}
	}

	//  Print out connections in the graph
	for (vector<connection_t>::iterator it = nemo_des.get_connections().begin(); it != nemo_des.get_connections().end(); ++it){
		fprintf(file_ptr, "\t\"%s\" -> \"%s\"[label=\"[%lu:%lu]->[%lu:%lu]\"];\n", nemo_des.get_sig_name(it->second).c_str(),
		nemo_des.get_sig_name(it->first).c_str(), nemo_des.get_nemo_sig(it->second).get_msb(), nemo_des.get_nemo_sig(it->second).get_lsb(), 
		nemo_des.get_nemo_sig(it->first).get_msb(), nemo_des.get_nemo_sig(it->first).get_lsb());
	}

	fprintf(file_ptr, "}\n");

	// Close output file
	close_file();
}