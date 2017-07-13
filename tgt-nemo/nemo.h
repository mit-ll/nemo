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

#include <set>
#include <ivl_target.h>
#include "nemo_dot_file.h"

#define DEBUG_PRINTS 			 false
#define INCLUDE_LOCAL_SIGNALS 	 false
#define ENUMERATE_ENTIRE_CIRCUIT false
#define SEARCH_DEPTH 			 6
// #define CRITICAL_SIG_REGEX "[\\(\\ (to_)]sr\\[0\\]\\|supv"
#define CRITICAL_SIG_REGEX "supv"

using namespace std;

void find_signal_dependencies(ivl_signal_t base_sig, Dot_File& df, set<ivl_signal_t>& explored_signals);
void find_critical_sigs(ivl_scope_t* root_scopes, unsigned num_root_scopes, Dot_File& df);
void find_critical_scope_sigs(ivl_scope_t scope, unsigned* num_sigs_found, Dot_File& df, set<ivl_signal_t>& explored_signals);

bool is_sig_explored(set<ivl_signal_t>& explored_signals, ivl_signal_t sig);
bool is_critical_sig(ivl_signal_t sig);
bool is_ivl_generated_signal(ivl_signal_t sig);
ivl_net_const_t is_const_local_sig(ivl_signal_t sig);
bool is_sig_output(ivl_signal_t sig);
bool are_both_signals_outputs(ivl_signal_t sig1, ivl_signal_t sig2);
bool is_sig1_output_and_sig2_not(ivl_signal_t sig1, ivl_signal_t sig2);

void print_signal_queues(set<ivl_signal_t>& critical_sigs, set<ivl_signal_t>& explored_signals);
void print_signal_attrs(ivl_signal_t sig);
void print_full_signal_name(ivl_signal_t sig);
void print_signal_info(ivl_signal_t sig);

void connect_signals(
	ivl_signal_t       aff_sig, 
	ivl_signal_t       sig, 
	set<ivl_signal_t>& critical_sigs,
	set<ivl_signal_t>& explored_sigs, 
	Dot_File&          df, 
	bool               expand_search);

void propagate_std_cell_sigs(
	ivl_signal_t aff_sig, 
	Dot_File&    df);

void propagate_sig(
	ivl_signal_t 	   aff_sig, 
	Dot_File& 		   df, 
	set<ivl_signal_t>& critical_sigs,
	set<ivl_signal_t>& explored_sigs,
	bool 			   expand_search);

void propagate_log(
	const ivl_net_logic_t logic, 
	ivl_signal_t 	   aff_sig, 
	Dot_File& 		   df, 
	set<ivl_signal_t>& critical_sigs,
	set<ivl_signal_t>& explored_sigs,
	bool 		       expand_search);

void propagate_lpm(
	const ivl_lpm_t	    lpm, 
	ivl_signal_t 	    aff_sig, 
	Dot_File& 		    df, 
	set<ivl_signal_t>& critical_sigs,
	set<ivl_signal_t>& explored_sigs,
	bool 			   expand_search);

#endif
