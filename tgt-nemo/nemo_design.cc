#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <fstream> 
#include <cstdio>
#include "nemo_design.h"

Nemo_Design::Nemo_Design(): root_scopes(NULL), num_root_scopes(0), num_sigs(0), 
	ivl_data_missing(false), pb_filename(string(SIGS_PB_FILE)){}

Nemo_Design::Nemo_Design(ivl_scope_t* rs, unsigned int num_rs)
	: root_scopes(rs), num_root_scopes(num_rs), num_sigs(0), ivl_data_missing(false), pb_filename(string(SIGS_PB_FILE)){
		load_design_signals();
}

Nemo_Design::Nemo_Design(ivl_scope_t* rs, unsigned int num_rs, string& pb_fn)
	: root_scopes(rs), num_root_scopes(num_rs), num_sigs(0), ivl_data_missing(false), pb_filename(pb_fn){
		load_design_signals();
}

void Nemo_Design::save_pb(){
	int fd = open(pb_filename.c_str(), O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
	ZeroCopyOutputStream* raw_output   = new FileOutputStream(fd);
	CodedOutputStream* 	  coded_output = new CodedOutputStream(raw_output);
	
	printf("Writing signal protobufs to %s ...\n", pb_filename.c_str());
	// Write Number of Signals to PB File
	coded_output->WriteLittleEndian32(num_sigs);
	// Write Signal Objects to PB File
	for(vector<Nemo_Signal>::iterator it = nemo_sigs.begin(); it != nemo_sigs.end(); ++it){
		if (!it->write_pb_to_file(coded_output)){
			fprintf(stderr, "ERROR: failed to write to protobuf file %s.\n", pb_filename.c_str());
		 	exit(-4);
		}
	}
	printf("Done writing protobuf to file.\n\n");
	
	delete coded_output;
	delete raw_output;
	close(fd);
}

unsigned int Nemo_Design::get_id(ivl_signal_t ivl_sig){
	return ivl2nemo[ivl_sig];
}

bool Nemo_Design::loaded_from_pb(){
	return ivl_data_missing;
}

Nemo_Signal Nemo_Design::get_nemo_sig(unsigned int sig_id){
	return nemo_sigs[sig_id];
}

vector<Nemo_Signal>& Nemo_Design::get_nemo_sigs(){
	return nemo_sigs;	
}

vector<ivl_signal_t>& Nemo_Design::get_ivl_sigs(){
	return ivl_sigs;	
}

const string& Nemo_Design::get_sig_name(ivl_signal_t sig){
	return nemo_sigs[ivl2nemo[sig]].full_name();
}

const string& Nemo_Design::get_sig_name(unsigned int sig_id){
	return nemo_sigs[sig_id].full_name();
}

vector<connection_t>& Nemo_Design::get_connections(){
	return connections;
}

void Nemo_Design::add_connection(ivl_signal_t aff_sig, unsigned int sig_id){
	unsigned int aff_sig_id = get_id(aff_sig);
	add_connection_to_nemo_signal(aff_sig_id, sig_id);
	connections.push_back(connection_t(aff_sig_id, sig_id));
}

void Nemo_Design::add_connection(ivl_signal_t aff_sig, ivl_signal_t sig){
	unsigned int aff_sig_id = get_id(aff_sig);
	unsigned int sig_id     = get_id(sig);
	add_connection_to_nemo_signal(aff_sig_id, sig_id);
	connections.push_back(connection_t(aff_sig_id, sig_id));
}

void Nemo_Design::add_connection_to_nemo_signal(unsigned int aff_sig_id, unsigned int sig_id){
	nemo_sigs[aff_sig_id].add_connection(sig_id);
}

void Nemo_Design::debug_print_all_nemo_sigs(){
	for(vector<Nemo_Signal>::iterator it = nemo_sigs.begin(); it != nemo_sigs.end(); ++it){
		it->debug_print_nemo_sig();
	} 
}

unsigned int Nemo_Design::add_duplicate_from_ivl(ivl_signal_t new_sig){
	nemo_sigs.push_back(Nemo_Signal(new_sig, num_sigs++));
	return (num_sigs - 1);
}

void Nemo_Design::load_design_signals(){
	if (!fexists(pb_filename)){
		// Protobuf file does not exist: find all signals in the iverilog design
		printf("Cannot find signals protobuf file (%s), parsing iverilog signals instead ...\n", pb_filename.c_str());
		for (unsigned int i = 0; i < num_root_scopes; i++) {
			load_from_ivl(root_scopes[i]);
		}
		printf("Done parsing iverilog signals.\n\n");
	} else{
		// Load signals from protobuf file
		printf("Found signal protobufs file (%s)!\n", pb_filename.c_str());
		load_from_pb();
	}
}

void Nemo_Design::load_from_ivl(ivl_scope_t scope){
	//@TODO: Look more into dealing with scopes that are tasks or functions
	if (ivl_scope_type(scope) != IVL_SCT_MODULE && ivl_scope_type(scope) != IVL_SCT_BEGIN && ivl_scope_type(scope) != IVL_SCT_TASK ) {
		fprintf(stderr, "ERROR: cannot parse scope type (%d)\n", ivl_scope_type(scope));
		fprintf(stderr, "File: %s Line: %d\n", ivl_scope_file(scope), ivl_scope_lineno(scope));
		return;
	}

	// Rescurse into any submodules
	for (unsigned int i = 0; i < ivl_scope_childs(scope); i++) {
		load_from_ivl(ivl_scope_child(scope, i));
	}

	// Enumerate all signals in each scope
	unsigned     num_scope_sigs = ivl_scope_sigs(scope);
	ivl_signal_t current_ivl_signal;
	for (unsigned idx = 0; idx < num_scope_sigs; idx++) {
		current_ivl_signal = ivl_scope_sig(scope, idx);
		ivl_sigs.push_back(current_ivl_signal);
		nemo_sigs.push_back(Nemo_Signal(current_ivl_signal, num_sigs));
		ivl2nemo[current_ivl_signal] = num_sigs++;
	}
}

void Nemo_Design::load_from_pb(){
	int fd = open(pb_filename.c_str(), O_RDONLY);
	ZeroCopyInputStream* raw_input   = new FileInputStream(fd);
	CodedInputStream*    coded_input = new CodedInputStream(raw_input);
	
	ivl_data_missing = true;
	
	// Read num_sigs from PB file
	coded_input->ReadLittleEndian32(&num_sigs);
	if (DEBUG_PRINTS){
		printf("Number of Signals to Load: %d\n", num_sigs);
	}

	// Read signal objects from PB file
	printf("Loading signal protobufs from %s ...\n", pb_filename.c_str());
	for(unsigned int i=0; i<num_sigs; i++){
		nemo_sigs.push_back(Nemo_Signal());
		if (DEBUG_PRINTS){
			printf("\n---- Before Signal Loaded: ----\n");
			nemo_sigs.back().debug_print_nemo_sig();
		}
		if (!nemo_sigs.back().read_pb_from_file(coded_input)){
			fprintf(stderr, "ERROR: failed to parse protobuf file %s.\n", pb_filename.c_str());
		 	exit(-4);
		}
		if (DEBUG_PRINTS){
			printf("---- After Signal Loaded: ----\n");
			nemo_sigs.back().debug_print_nemo_sig();
		}
	}
	printf("Done loading signal protobufs.\n\n");

	delete coded_input;
	delete raw_input;
	close(fd);
}

bool Nemo_Design::fexists(string& filename){
	ifstream ifile(filename.c_str());
	return ifile.good();
}
