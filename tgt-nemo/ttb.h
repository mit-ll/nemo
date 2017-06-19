#ifndef __TTB_HEADER__
#define __TTB_HEADER__
/*
 * TTB Backend
 *
 * This is a backend for Icarus verilog which generates a dot file with a
 * dependancy graph showing which signals are dependant on which signals
 *
 * Currently the dot file format is not supported. Instead, I output our own
 * format.
 */
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include <ivl_target.h>
#include "ttb_signal.h"

typedef std::map<std::string, TTB_Signal*> sig_map_t;

// Cannot be pointers since these signals could be slices
typedef std::pair<TTB_Signal, TTB_Signal> connection_t;

extern void propagate_sig(TTB_Signal& aff_sig, std::vector<connection_t>& connections);
extern void propagate_lpm(const ivl_lpm_t lpm, TTB_Signal& aff_sig, std::vector<connection_t>& connections);
extern void propagate_log(const ivl_net_logic_t logic, TTB_Signal& aff_sig, std::vector<connection_t>& connections);
extern int  process_statement(ivl_statement_t stmt, sig_map_t& ffs, std::vector<connection_t>& connections);
extern void print_connection(const TTB_Signal& aff_sig, const TTB_Signal& sig, std::vector<connection_t>& connections);

#endif
