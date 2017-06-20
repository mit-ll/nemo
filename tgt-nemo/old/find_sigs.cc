#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>

#include "find_sigs.h"

Sig_Finder::Sig_Finder(): root_scopes(NULL), num_root_scopes(0), signal_counter(0){
	critical_sigs_regex = std::regex("[\(\ (to_)]sr\[0\]\|supv", std::regex_constants::grep);
}

Sig_Finder::Sig_Finder(ivl_scope_t* rs, unsigned int num_rs)
	: root_scopes(rs), num_root_scopes(num_rs), signal_counter(0){
	critical_sigs_regex = std::regex("[\(\ (to_)]sr\[0\]\|supv", std::regex_constants::grep);
}

void Sig_Finder::enumerate_design_sigs(Nemo_Signals& sigs, std::vector<ivl_signal_t>& ivl_sigs){
	// Check if protobuf file exists
	std::fstream pb_signals_input(SIGS_PB_FILE, std::ios::in | std::ios::binary);
	if (!pb_signals_input){
		// Protobuf file does not exist: find all signals in the iverilog design
		printf("Cannot find signals protobuf file (%s), parsing iverilog signals instead...\n", SIGS_PB_FILE);
		for (unsigned int i = 0; i < num_root_scopes; i++) {
			enumerate_sigs(root_scopes[i], sigs, ivl_sigs);
		}
		printf("Done parsing iverilog signals, writing protobuf to file...\n");
		
		// Write out protobuf data structures to a file
		std::fstream pb_signals_output(SIGS_PB_FILE, std::ios::out | std::ios::trunc | std::ios::binary);
		if (!sigs.SerializeToOstream(&pb_signals_output)) {
			fprintf(stderr, "ERROR: failed to parse protobuf file %s\n", SIGS_PB_FILE);
			exit(-4);
		}
		printf("Done writing protobuf to file.\n");
	} else{
		// Load signals from protobuf file
		printf("Found signals protobuf file (%s), loading protobuf...\n", SIGS_PB_FILE);
		if (!sigs.ParseFromIstream(&pb_signals_input)){
			fprintf(stderr, "ERROR: failed to parse protobuf file %s\n", SIGS_PB_FILE);
			exit(-4);
		}
		printf("Done loading protobufs.\n");
	}
}

// void Sig_Finder::find_critical_sigs(Nemo_Signals& sigs){
// 	return;
// }

void Sig_Finder::init_nemo_sig(ivl_signal_t ivl_sig, Nemo_Signal* sig){
	// Check that ivl_sig is not NULL
	if (!(ivl_sig)) return;
	
	// Set Signal ID number (index into signal array)
	sig->set_id(signal_counter);

	// Set MSB and LSB dimensions
	unsigned num_dimens = ivl_signal_packed_dimensions(ivl_sig);
	if (num_dimens == 0) {
		sig->set_msb(0);
		sig->set_lsb(0);
	} else if (num_dimens == 1) {
		sig->set_msb(ivl_signal_packed_msb(ivl_sig, 0));
		sig->set_lsb(ivl_signal_packed_lsb(ivl_sig, 0));
	} else {
		assert(false && "Unsupported number of dimensions");
	}

	// Retrieve Signal Scope
	ivl_scope_t scope = ivl_signal_scope(ivl_sig);

	// Set signal fullname ... Yea.... This is bad
	sig->set_fullname(std::string(ivl_scope_name(scope)) + std::string(".") + std::string(ivl_signal_basename(ivl_sig)));

	// Set Signal Type
	if (IVL_SIP_INPUT == ivl_signal_port(ivl_sig)) {
		sig->set_sig_type(Nemo_Signal::SIG_INPUT);
	} else {
		sig->set_sig_type(Nemo_Signal::SIG_NONE);
	}
}

// Enumerate all the signals in scope
void Sig_Finder::enumerate_sigs(ivl_scope_t scope, Nemo_Signals& sigs,  std::vector<ivl_signal_t>& ivl_sigs){
	//@TODO: Look more into dealing with scopes that are tasks or functions
	if ( ivl_scope_type(scope) != IVL_SCT_MODULE && ivl_scope_type(scope) != IVL_SCT_BEGIN && ivl_scope_type(scope) != IVL_SCT_TASK ) {
		fprintf(stderr, "ERROR: Cannot parse scope type (%d)\n", ivl_scope_type(scope));
		fprintf(stderr, "File: %s Line: %d\n", ivl_scope_file(scope), ivl_scope_lineno(scope));
		return;
	}

	// Rescurse into any submodules
	for (unsigned int i = 0; i < ivl_scope_childs(scope); i++) {
		enumerate_sigs(ivl_scope_child(scope, i), sigs, ivl_sigs);
	}

	unsigned num_sigs = ivl_scope_sigs(scope);
	std::string sig_name;
	ivl_signal_t current_ivl_signal;
	for (unsigned idx = 0; idx < num_sigs; idx++) {
		current_ivl_signal = ivl_scope_sig(scope, idx);
		ivl_sigs.push_back(current_ivl_signal);
		init_nemo_sig(current_ivl_signal, sigs.add_signals());
		sig_name = sigs.signals(signal_counter).fullname();
		(*sigs.mutable_name2sig())[sig_name] = signal_counter++;

		// if (std::regex_search(sigs.back().name(), critical_sigs_regex)){
		// 	printf("Found Critical Sig: %s\n", sigs.back().name().c_str());
		// }
	}
}