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

class Nemo_Design{
	public:
		Nemo_Design();
		Nemo_Design(ivl_scope_t* rs, unsigned int num_rs);
		Nemo_Design(ivl_scope_t* rs, unsigned int num_rs, string& pb_fn);
		void          save_pb();
		unsigned int  get_id(ivl_signal_t ivl_sig);
		bool		  loaded_from_pb();
		vector<Nemo_Signal>& get_sigs(); 
	private:
		ivl_scope_t*                    root_scopes;	  // root ivl scopes
		unsigned int                    num_root_scopes;  // number of root ivl scopes in the design
		unsigned int                    num_sigs;		  // number of unique signals in the design
		bool 							ivl_data_missing; // true if data was sourced from PB file (i.e. ivl info not available)
		string                          pb_filename;	  // protobuf filename
		vector<Nemo_Signal>		        nemo_sigs;	      // all nemo signals in the design
		vector<ivl_signal_t>            ivl_sigs;   	  // all ivl signals in the design
		map<ivl_signal_t, unsigned int> ivl2nemo; 		  // map between ivl_signal and nemo signal ID
		void load_design_signals();
		void load_from_ivl(ivl_scope_t scope);
		void load_from_pb();
		bool fexists(string& filename);
};

#endif