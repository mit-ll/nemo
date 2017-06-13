#ifndef __TTB_DOT_FILE__
#define __TTB_DOT_FILE__

#include <cstdio>
#include <vector>

#include "ttb.h"
#include "ttb_signal.h"

class Dot_File {
	public:
		Dot_File();
		Dot_File(const char* p);
		void set_path(const char* p);
		void print_graph(std::vector<TTB_Signal> sigs, std::vector<connection_t> connections);
	private:
		const char* path;
		FILE* 		file_ptr;
		FILE* get_file_ptr();
		void  open_file();
		void  close_file();
};

#endif