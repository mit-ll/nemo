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
	int fd = open(pb_filename.c_str(), O_CREAT | O_WRONLY);
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
	printf("Done writing protobuf to file.\n");
	
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

vector<Nemo_Signal>& Nemo_Design::get_sigs(){
	return nemo_sigs;	
}

void Nemo_Design::load_design_signals(){
	if (!fexists(pb_filename)){
		// Protobuf file does not exist: find all signals in the iverilog design
		printf("Cannot find signals protobuf file (%s), parsing iverilog signals instead ...\n", pb_filename.c_str());
		for (unsigned int i = 0; i < num_root_scopes; i++) {
			load_from_ivl(root_scopes[i]);
		}
		printf("Done parsing iverilog signals.\n");
	} else{
		// Load signals from protobuf file
		printf("Found signal protobufs file (%s)!\n", pb_filename.c_str());
		load_from_pb();
	}
}

void Nemo_Design::load_from_ivl(ivl_scope_t scope){
	//@TODO: Look more into dealing with scopes that are tasks or functions
	if (ivl_scope_type(scope) != IVL_SCT_MODULE && ivl_scope_type(scope) != IVL_SCT_BEGIN && ivl_scope_type(scope) != IVL_SCT_TASK ) {
		fprintf(stderr, "ERROR: Cannot parse scope type (%d)\n", ivl_scope_type(scope));
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

	// Read signal objects from PB file
	printf("Loading signal protobufs from %s ...\n", pb_filename.c_str());
	for(unsigned int i=0; i<num_sigs; i++){
		nemo_sigs.push_back(Nemo_Signal());
		if (!nemo_sigs.back().read_pb_from_file(coded_input)){
			fprintf(stderr, "ERROR: failed to parse protobuf file %s.\n", pb_filename.c_str());
		 	exit(-4);
		}
	}
	printf("Done loading signal protobufs.\n");

	delete coded_input;
	delete raw_input;
	close(fd);
}

bool Nemo_Design::fexists(string& filename){
	ifstream ifile(filename.c_str());
	printf("%d\n", (int)ifile);
	return (bool)ifile;
}
