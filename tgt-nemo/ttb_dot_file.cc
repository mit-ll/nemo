#include <cstdio>
#include <cstdlib>
#include <vector>

#include <ivl_target.h>

#include "ttb.h"
#include "ttb_signal.h"
#include "ttb_dot_file.h"

Dot_File::Dot_File(): path(NULL), file_ptr(NULL){}

Dot_File::Dot_File(const char* p): file_ptr(NULL){
	set_path(p);
}

void Dot_File::set_path(const char* p){
	path = p;
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

void Dot_File::print_graph(std::vector<TTB_Signal> sigs, std::vector<connection_t> connections){
	// Open output file
	open_file();

	// Print out nodes in the graph
	fprintf(file_ptr, "digraph G {\n");
	for (std::vector<TTB_Signal>::iterator it = sigs.begin(); it != sigs.end(); ++it ) {
		// Make local signals just points instead of full nodes
		if (ivl_signal_local(it->get_sig())) {
			// Graph node is a circuit ...
			fprintf(file_ptr, "\t\"%s\" [shape=point, label=\"%s[%lu:%lu]\"];\n", it->name().c_str(), it->name().c_str(), it->get_msb(), it->get_lsb());
		} else if (it->is_ff()) {
			// Graph node is a circuit flip-flop
			fprintf(file_ptr, "\t\"%s\" [shape=square, label=\"%s[%lu:%lu]\"]; /* Flip Flop */\n", it->name().c_str(), it->name().c_str(), it->get_msb(), it->get_lsb());
		} else if (it->is_input()) {
			// Graph node is a circuit input
			fprintf(file_ptr, "\t\"%s\" [shape=none, label=\"%s[%lu:%lu]\"]; /* Input */\n", it->name().c_str(), it->name().c_str(), it->get_msb(), it->get_lsb());
		} else {
			// Graph node is circuit output, or anything else 
			fprintf(file_ptr, "\t\"%s\" [shape=ellipse, label=\"%s[%lu:%lu]\"];\n", it->name().c_str(), it->name().c_str(), it->get_msb(), it->get_lsb());
		}
	}

	//  Print out connections in the graph
	for (std::vector<connection_t>::iterator it = connections.begin(); it != connections.end(); ++it) {
		fprintf(file_ptr, "\t\"%s\" -> \"%s\"[label=\"[%lu:%lu]->[%lu:%lu]\"];\n", it->second.name().c_str(),
		it->first.name().c_str(), it->second.get_msb(), it->second.get_lsb(), it->first.get_msb(), it->first.get_lsb());
	}

	fprintf(file_ptr, "}\n");

	// Close output file
	close_file();
}