#ifndef __FIND_SIGS_HEADER__
#define __FIND_SIGS_HEADER__

#include <vector>
#include <string>
#include <regex>
#include <pthread.h>
#include <ivl_target.h>
#include "ttb_signal.h"

#define NUM_SIG_FINDER_THREADS 8

class Sig_Finder {
	public:
		Sig_Finder();
		Sig_Finder(ivl_scope_t* rs, unsigned int num_rs);
		void enumerate_design_sigs(std::vector<TTB_Signal>& sigs);
		void find_critical_sigs(std::vector<TTB_Signal>& sigs);
	
	private:
		ivl_scope_t* root_scopes;
		unsigned int num_root_scopes;
		pthread_t threads[NUM_SIG_FINDER_THREADS];
		std::vector<TTB_Signal> sig_subsets[NUM_SIG_FINDER_THREADS];
		std::regex critical_sigs_regex;
		
		void enumerate_sigs(ivl_scope_t scope, std::vector<TTB_Signal>& sigs);
};

#endif