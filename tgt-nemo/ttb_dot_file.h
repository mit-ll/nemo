#ifndef __TTB_DOT_FILE__
#define __TTB_DOT_FILE__

#include <cstdio>
#include <vector>

#include "ttb.h"
#include "nemo_design.h"

using namespace std;

class Dot_File {
	public:
		Dot_File();
		Dot_File(const char* p);
		void        set_path(const char* p);
		const char* get_path();
		void        print_graph(Nemo_Design& nemo_des, vector<connection_t>& connections);
	private:
		const char* path;
		FILE* 		file_ptr;
		FILE*       get_file_ptr();
		void        open_file();
		void        close_file();
};

#endif