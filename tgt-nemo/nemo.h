#ifndef __NEMO_HEADER__
#define __NEMO_HEADER__
/*
 * TTB Backend
 *
 * This is a backend for Icarus verilog which generates a dot file with a
 * dependancy graph showing which signals are dependant on which signals
 *
 * Currently the dot file format is not supported. Instead, I output our own
 * format.
 */

#include <vector>
#include <ivl_target.h>
#include "nemo_dot_file.h"

#define DEBUG_PRINTS 			 false
#define INCLUDE_LOCAL_SIGNALS 	 false
#define ENUMERATE_ENTIRE_CIRCUIT false
// #define VISITED_ATTR_STRING "v"
#define SEARCH_DEPTH 3
#define CRITICAL_SIG_REGEX "o1"
// #define CRITICAL_SIG_REGEX "[\(\ (to_)]sr\[0\]\|supv"

using namespace std;

void find_critical_sigs(vector<ivl_signal_t>& critical_sigs, ivl_scope_t* root_scopes, unsigned num_root_scopes);
void find_critical_scope_sigs(ivl_scope_t scope, vector<ivl_signal_t>& critical_sigs);
bool is_critical_sig(ivl_signal_t sig);
bool is_ivl_generated_signal(ivl_signal_t sig);
ivl_net_const_t is_const_local_sig(ivl_signal_t sig);
void print_signal_attrs(ivl_signal_t sig);
void print_full_signal_name(ivl_signal_t sig);
void print_signals(vector<ivl_signal_t>& critical_sigs);

void propagate_sig(
	ivl_signal_t 		  aff_sig, 
	Dot_File& 			  df, 
	vector<ivl_signal_t>& critical_sigs,
	bool 				  expand_search);

void propagate_log(
	const ivl_net_logic_t logic, 
	ivl_signal_t 		  aff_sig, 
	Dot_File& 			  df, 
	vector<ivl_signal_t>& critical_sigs,
	bool 				  expand_search);

void propagate_lpm(
	const ivl_lpm_t 	  lpm, 
	ivl_signal_t 		  aff_sig, 
	Dot_File& 			  df, 
	vector<ivl_signal_t>& critical_sigs,
	bool 				  expand_search);

#endif
