#ifndef __NEMO_DOT_FILE__
#define __NEMO_DOT_FILE__

class Dot_File {
	public:
		Dot_File();
		Dot_File(const char* p);
		void        set_path(const char* p);
		const char* get_path();
		void        init_graph();
		void        add_node(ivl_signal_t sig);
		void        add_connection(ivl_signal_t aff_sig, ivl_signal_t sig);
		void        save_graph();
	private:
		const char* path;
		FILE* 		file_ptr;
		FILE*       get_file_ptr();
		void        open_file();
		void        close_file();
		unsigned long get_msb(ivl_signal_t sig);
		unsigned long get_lsb(ivl_signal_t sig);
};

#endif