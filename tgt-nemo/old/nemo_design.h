#ifndef __NEMO_DESIGN_HEADER__
#define __NEMO_DESIGN_HEADER__

#include <vector>
#include <map>
#include <string>
#include <ivl_target.h>

#include "nemo_signal.h"
#include "nemo_signal.pb.h"
#include "nemo_connection.h"
#include "nemo_connection.pb.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>

using namespace std;
using namespace nemo_pb;
using namespace google::protobuf::io;

#define SIGS_PB_FILE 		 ".sigs.pb"
#define SPLICED_SIGS_PB_FILE ".spliced_sigs.pb"
#define CONS_PB_FILE 		 ".connections.pb"
#define DEBUG_PRINTS 		 false

typedef pair<unsigned int, unsigned int> connection_t;

class Nemo_Design{
	public:
		Nemo_Design();
		Nemo_Design(ivl_scope_t* rs, unsigned int num_rs, const char* pb_f_bn);
		~Nemo_Design();

		static bool fexists(const char* filename);
		void 		serialize_nemo_signal_pbs();
		void 		serialize_spliced_nemo_signal_pbs();
		void 		serialize_nemo_connection_pbs();
		void 		load_design_signals();
		void 		load_design_connections();
		void 		delete_nemo_sigs();
		void 		delete_spliced_nemo_sigs();
		void 		delete_connections();
		bool 		were_signals_loaded_from_pb();
		bool 		were_connections_loaded_from_pb();

		unsigned int 			  get_id(ivl_signal_t ivl_sig);
		Nemo_Signal* 			  get_nemo_sig(unsigned int sig_id);
		Nemo_Signal* 			  get_nemo_sig(ivl_signal_t ivl_sig);
		vector<Nemo_Signal*>* 	  get_nemo_sigs();
		vector<ivl_signal_t>& 	  get_ivl_sigs();
		const string& 			  get_sig_name(ivl_signal_t sig);
		const string& 			  get_sig_name(unsigned int sig_id);
		vector<Nemo_Connection*>* get_connections();

		void 					  add_connection(ivl_signal_t aff_sig, unsigned int sig_id);
		void 					  add_connection(ivl_signal_t aff_sig, ivl_signal_t sig);
		Nemo_Signal* 			  add_spliced_signal(ivl_signal_t new_sig);
		
		void 					  debug_print_all_nemo_sigs();
		void 					  debug_print_all_nemo_sig_names();
		void 					  debug_print_all_ivl_sig_names();
	private:
		ivl_scope_t*                    root_scopes;	    // root ivl scopes
		unsigned int                    num_root_scopes;    // number of root ivl scopes in the design
		unsigned int                    num_sigs;		    // number of signals in the design
		unsigned int                    num_spliced_sigs;   // number of spliced signals in the design
		unsigned int                    num_conns;		    // number of connections in the design
		bool 							signals_loaded_from_pb;     // true if signal data was sourced from PB file (i.e. ivl info not available)
		bool 							connections_loaded_from_pb; // true if connection data was sourced from PB file (i.e. ivl info not available)
		string							sigs_pb_file;         // signals protobuf filename
		string							spliced_sigs_pb_file; // spliced signals protobuf filename
		string	                    	cons_pb_file;         // connections protobuf filename
		vector<Nemo_Signal*>*		    nemo_sigs;	          // all nemo signals in the design
		vector<Nemo_Signal*>*		    spliced_nemo_sigs;    // all spliced nemo signals in the design
		vector<ivl_signal_t>            ivl_sigs;   	      // all ivl signals in the design
		vector<Nemo_Connection*>*       connections;          // connections between nemo signals
		map<ivl_signal_t, unsigned int> ivl2nemo; 		      // map between ivl_signal and nemo signal ID
		
		// Signal Loading Functions
		void parse_signals_from_pb(const char* fn);
		void parse_spliced_signals_from_pb(const char* fn);
		void parse_signals_from_ivl(ivl_scope_t scope);
		
		// Connection Loading Functions
		void parse_connections_from_pb(const char* fn);
		void parse_connections_from_ivl();
		void propagate_sig(ivl_signal_t aff_sig);
		void propagate_lpm(const ivl_lpm_t lpm, ivl_signal_t aff_sig);
		void propagate_log(const ivl_net_logic_t logic, ivl_signal_t aff_sig);
		unsigned int get_nexus_width(ivl_nexus_t nex);
		void add_lpm_connection(ivl_nexus_t nex, const ivl_signal_t aff_sig, ivl_lpm_t lpm);
		// void add_connection_to_nemo_signal(unsigned int aff_sig_id, unsigned int sig_id);
};

#endif