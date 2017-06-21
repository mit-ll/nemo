#ifndef __NEMO_SIGNAL_HEADER__
#define __NEMO_SIGNAL_HEADER__

#include <string>
#include <ivl_target.h>

#include "nemo_signal.pb.h"
#include <google/protobuf/io/coded_stream.h>

using namespace std;
using namespace nemo_pb;
using namespace google::protobuf::io;

class Nemo_Signal{
	public:
		Nemo_Signal();
		Nemo_Signal(ivl_signal_t s, unsigned int s_id);
		bool          write_pb_to_file(CodedOutputStream* strm);
		bool 		  read_pb_from_file(CodedInputStream* strm);
		unsigned int  get_id() const;
		unsigned int  get_dimensions() const;
		unsigned long get_lsb() const;
		unsigned long get_msb() const;
		bool          is_ff() const;
		bool          is_input() const;
		int 		  signal_local() const;
		const string& scope_name() const;
		const string& base_name() const;
		const string& full_name() const;
		void		  add_connection(unsigned int sig_id);
		void 		  debug_print_nemo_sig();
	private:
		Nemo_Signal_PB	sig; // nemo signal in the design
		void init_from_ivl(ivl_signal_t s, unsigned int s_id);
};

#endif