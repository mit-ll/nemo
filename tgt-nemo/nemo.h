// -----------------------------------------------------------------------------
// File:        nemo.h
// Author:      Timothy Trippel
// Affiliation: MIT Lincoln Laboratory
// -----------------------------------------------------------------------------

#ifndef __NEMO_HEADER__
#define __NEMO_HEADER__

#include <set>
#include <vector>
#include <ivl_target.h>
#include "nemo_dot_file.h"

// Debugging Switches
#define DEBUG_PRINTS 		          false
#define DEBUG_CONNECTION_DIRS_INFO    false
#define DEBUG_CONNECTION_DIRS_WARNING true
#define DEBUG_LOGIC_ASSIGN_CONNECTION false

// Circuit Analysis Options
#define INCLUDE_LOCAL_SIGNALS 	 false
#define ENUMERATE_ENTIRE_CIRCUIT false
#define IGNORE_CLK_SIGNALS		 true
#define SEQUENTIAL_CLK_PIN_NAME  "CLK"

// Tool Defaults
#define DEFAULT_SEARCH_DEPTH       1
#define DEFAULT_CRITICAL_SIG_REGEX "critical"

using namespace std;

void find_all_signal_dependencies(vector<ivl_signal_t>& critical_sigs, Dot_File& df, unsigned search_depth);
void find_signal_dependencies(ivl_signal_t critical_sig, Dot_File& df, set<ivl_signal_t>& expanded_signals, unsigned search_depth);
void find_critical_sigs(ivl_scope_t* root_scopes, unsigned num_root_scopes, vector<ivl_signal_t>& critical_sigs, const char* regex_str);
void find_critical_scope_sigs(ivl_scope_t scope, unsigned* num_sigs_found, vector<ivl_signal_t>& critical_sigs, const char* regex_str);

bool is_sig_expanded(set<ivl_signal_t>& explored_signals, ivl_signal_t sig);
bool is_critical_sig(ivl_signal_t sig, const char* regex_str);
bool is_clk_sig(ivl_signal_t sig);
bool is_child_of_parent_module(ivl_scope_t child_scope, ivl_scope_t parent_scope);
bool is_ivl_generated_signal(ivl_signal_t sig);
ivl_net_const_t is_const_local_sig(ivl_signal_t sig);

void print_signal_queues(set<ivl_signal_t>& critical_sigs, set<ivl_signal_t>& explored_signals);
void print_signal_attrs(ivl_signal_t sig);
void print_full_signal_name(ivl_signal_t sig);
void print_signal_info(ivl_signal_t sig);

void connect_signals(
	ivl_signal_t       aff_sig, 
	ivl_signal_t       sig, 
	set<ivl_signal_t>& sigs_to_expand,
	set<ivl_signal_t>& explored_sigs, 
	Dot_File&          df, 
	bool               expand_search);

int expand_std_cell_sigs(
	ivl_signal_t       aff_sig, 
	Dot_File&          df, 
	set<ivl_signal_t>& sigs_to_expand,
	set<ivl_signal_t>& explored_sigs,
	bool 			   expand_search);

int expand_sig(
	ivl_signal_t 	   aff_sig, 
	Dot_File& 		   df, 
	set<ivl_signal_t>& sigs_to_expand,
	set<ivl_signal_t>& explored_sigs,
	bool 			   expand_search);

int expand_log(
	const ivl_net_logic_t logic, 
	ivl_signal_t 	   aff_sig, 
	Dot_File& 		   df, 
	set<ivl_signal_t>& sigs_to_expand,
	set<ivl_signal_t>& explored_sigs,
	bool 		       expand_search);

int expand_lpm(
	const ivl_lpm_t	   lpm, 
	ivl_signal_t 	   aff_sig, 
	Dot_File& 		   df, 
	set<ivl_signal_t>& sigs_to_expand,
	set<ivl_signal_t>& explored_sigs,
	bool 			   expand_search);

#endif
