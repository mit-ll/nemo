#ifndef __NEMO_DESIGN_HEADER__
#define __NEMO_DESIGN_HEADER__

#include <vector>
#include <map>
#include <string>
#include <ivl_target.h>

#include "nemo_signal.h"
#include "nemo_signal.pb.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

using namespace std;
using namespace nemo_pb;
using namespace google::protobuf::io;

#define SIGS_PB_FILE "nemo_design.pb.bin"
#define DEBUG_PRINTS false

typedef pair<unsigned int, unsigned int> connection_t;

class Nemo_Design{
	public:
		Nemo_Design();
		Nemo_Design(ivl_scope_t* rs, unsigned int num_rs);
		Nemo_Design(ivl_scope_t* rs, unsigned int num_rs, string& pb_fn);
		void 					save_pb();
		bool 					loaded_from_pb();
		unsigned int 			get_id(ivl_signal_t ivl_sig);
		Nemo_Signal 			get_nemo_sig(unsigned int sig_id);
		vector<Nemo_Signal>& 	get_nemo_sigs();
		vector<ivl_signal_t>& 	get_ivl_sigs();
		const string& 			get_sig_name(ivl_signal_t sig);
		const string& 			get_sig_name(unsigned int sig_id);
		vector<connection_t>& 	get_connections();
		void 					add_connection(ivl_signal_t aff_sig, unsigned int sig_id);
		void 					add_connection(ivl_signal_t aff_sig, ivl_signal_t sig);
		unsigned int 			add_duplicate_from_ivl(ivl_signal_t new_sig);
		void 					debug_print_all_nemo_sigs();
	private:
		ivl_scope_t*                    root_scopes;	  // root ivl scopes
		unsigned int                    num_root_scopes;  // number of root ivl scopes in the design
		unsigned int                    num_sigs;		  // number of unique signals in the design
		bool 							ivl_data_missing; // true if data was sourced from PB file (i.e. ivl info not available)
		string                          pb_filename;	  // protobuf filename
		vector<Nemo_Signal>		        nemo_sigs;	      // all nemo signals in the design
		vector<ivl_signal_t>            ivl_sigs;   	  // all ivl signals in the design
		vector<connection_t>            connections;      // connections between nemo signals
		map<ivl_signal_t, unsigned int> ivl2nemo; 		  // map between ivl_signal and nemo signal ID
		void load_design_signals();
		void load_from_ivl(ivl_scope_t scope);
		void load_from_pb();
		bool fexists(string& filename);
		void add_connection_to_nemo_signal(unsigned int aff_sig_id, unsigned int sig_id);
};

#endif