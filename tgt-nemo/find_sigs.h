#ifndef __FIND_SIGS_HEADER__
#define __FIND_SIGS_HEADER__

#include <string>
#include <regex>
// #include <pthread.h>
#include <ivl_target.h>
#include "nemo_signals.pb.h"

using namespace nemo_pb;

#define NUM_SIG_FINDER_THREADS 8
#define SIGS_PB_FILE 		   "nemo_signals.pb.bin"

class Sig_Finder {
	public:
		Sig_Finder();
		Sig_Finder(ivl_scope_t* rs, unsigned int num_rs);
		void enumerate_design_sigs(Nemo_Signals& sigs, std::vector<ivl_signal_t>& ivl_sigs);
		// void find_critical_sigs(Nemo_Signals& sigs);
	
	private:
		ivl_scope_t* root_scopes;
		unsigned int num_root_scopes;
		unsigned int signal_counter;
		// pthread_t threads[NUM_SIG_FINDER_THREADS];
		std::regex critical_sigs_regex;

		void init_nemo_sig(ivl_signal_t iverilog_sig, Nemo_Signal* sig);
		void enumerate_sigs(ivl_scope_t scope, Nemo_Signals& sigs, std::vector<ivl_signal_t>& ivl_sigs);
};

#endif