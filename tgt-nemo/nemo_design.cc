#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <fstream> 
#include <cstdio>
#include "nemo_design.h"

Nemo_Design::Nemo_Design(): root_scopes(NULL), num_root_scopes(0), num_sigs(0), num_spliced_sigs(0), 
	num_conns(0), signals_loaded_from_pb(false), connections_loaded_from_pb(false){
		nemo_sigs         = NULL;
		spliced_nemo_sigs = NULL;
		connections       = NULL;	
	}

Nemo_Design::Nemo_Design(ivl_scope_t* rs, unsigned int num_rs)
	: root_scopes(rs), num_root_scopes(num_rs), num_sigs(0), num_spliced_sigs(0),
	num_conns(0), signals_loaded_from_pb(false), connections_loaded_from_pb(false){
		nemo_sigs         = NULL;
		spliced_nemo_sigs = NULL;
		connections       = NULL;
}

Nemo_Design::~Nemo_Design(){
	delete_nemo_sigs();
	delete_spliced_nemo_sigs();
	delete_connections();
}

// Static Method
bool Nemo_Design::fexists(const char* filename){
	ifstream ifile(filename);
	return ifile.good();
}

void Nemo_Design::serialize_nemo_signal_pbs(){
	int fd = open(SIGS_PB_FILE, O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
	ZeroCopyOutputStream* raw_output   = new FileOutputStream(fd);
	CodedOutputStream* 	  coded_output = new CodedOutputStream(raw_output);
	
	printf("Writing signal protobufs to %s ...\n", SIGS_PB_FILE);
	// Write Number of Signals to PB File
	coded_output->WriteLittleEndian32(num_sigs);

	// Write Signal Objects to PB File
	for(unsigned int i = 0; i < nemo_sigs->size(); i++){
		if (!(*nemo_sigs)[i]->write_pb_to_file(coded_output)){
			fprintf(stderr, "ERROR: failed to write to protobuf file %s.\n", SIGS_PB_FILE);
		 	exit(-4);
		}
	}
	printf("Done writing protobuf to file.\n\n");
	
	delete coded_output;
	delete raw_output;
	close(fd);
}

void Nemo_Design::serialize_spliced_nemo_signal_pbs(){
	int fd = open(SPLICED_SIGS_PB_FILE, O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
	ZeroCopyOutputStream* raw_output   = new FileOutputStream(fd);
	CodedOutputStream* 	  coded_output = new CodedOutputStream(raw_output);
	
	printf("Writing spliced signal protobufs to %s ...\n", SPLICED_SIGS_PB_FILE);
	// Write Number of Signals to PB File
	coded_output->WriteLittleEndian32(num_spliced_sigs);

	// Write Signal Objects to PB File
	for(unsigned int i = 0; i < spliced_nemo_sigs->size(); i++){
		if (!(*spliced_nemo_sigs)[i]->write_pb_to_file(coded_output)){
			fprintf(stderr, "ERROR: failed to write to protobuf file %s.\n", SPLICED_SIGS_PB_FILE);
		 	exit(-4);
		}
	}
	printf("Done writing protobuf to file.\n\n");
	
	delete coded_output;
	delete raw_output;
	close(fd);
}

void Nemo_Design::serialize_nemo_connection_pbs(){
	int fd = open(CONS_PB_FILE, O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
	ZeroCopyOutputStream* raw_output   = new FileOutputStream(fd);
	CodedOutputStream* 	  coded_output = new CodedOutputStream(raw_output);
	
	printf("Writing signal protobufs to %s ...\n", CONS_PB_FILE);
	// Write Number of Signals to PB File
	coded_output->WriteLittleEndian32(num_conns);

	// Write Signal Objects to PB File
	for(unsigned int i = 0; i < connections->size(); i++){
		if (!(*connections)[i]->write_pb_to_file(coded_output)){
			fprintf(stderr, "ERROR: failed to write to protobuf file %s.\n", CONS_PB_FILE);
		 	exit(-4);
		}
	}
	printf("Done writing protobuf to file.\n\n");
	
	delete coded_output;
	delete raw_output;
	close(fd);
}

void Nemo_Design::delete_nemo_sigs(){
	if (nemo_sigs){
		for(unsigned int i=0; i<nemo_sigs->size(); i++){
			if ((*nemo_sigs)[i]){
				delete (*nemo_sigs)[i];
			}
		}
		delete nemo_sigs;
	}
}

void Nemo_Design::delete_spliced_nemo_sigs(){
	if (spliced_nemo_sigs){
		for(unsigned int i=0; i<spliced_nemo_sigs->size(); i++){
			if ((*spliced_nemo_sigs)[i]){
				delete (*spliced_nemo_sigs)[i];
			}
		}
		delete spliced_nemo_sigs;
	}
}

void Nemo_Design::delete_connections(){
	if (connections){
		for(unsigned int i=0; i<connections->size(); i++){
			if ((*connections)[i]){
				delete (*connections)[i];
			}
		}
		delete connections;
	}
}

bool Nemo_Design::were_signals_loaded_from_pb(){
	return signals_loaded_from_pb;
}

bool Nemo_Design::were_connections_loaded_from_pb(){
	return signals_loaded_from_pb;
}

unsigned int Nemo_Design::get_id(ivl_signal_t ivl_sig){
	return ivl2nemo[ivl_sig];
}

Nemo_Signal* Nemo_Design::get_nemo_sig(unsigned int sig_id){
	if (nemo_sigs != NULL){
		return (*nemo_sigs)[sig_id];	
	}
	return NULL;
}

vector<Nemo_Signal*>* Nemo_Design::get_nemo_sigs(){
	if (nemo_sigs != NULL){
		return nemo_sigs;		
	}
	return NULL;	
}

vector<ivl_signal_t>& Nemo_Design::get_ivl_sigs(){
	return ivl_sigs;	
}

const string& Nemo_Design::get_sig_name(ivl_signal_t sig){
	if (nemo_sigs != NULL){
		return (*nemo_sigs)[ivl2nemo[sig]]->full_name();
	}
	return NULL;
}

const string& Nemo_Design::get_sig_name(unsigned int sig_id){
	if (nemo_sigs != NULL){
		return (*nemo_sigs)[sig_id]->full_name();
	}
	return NULL;
}

vector<Nemo_Connection*>* Nemo_Design::get_connections(){
	if (connections != NULL){
		return connections;
	}
	return NULL;
}

void Nemo_Design::add_connection(ivl_signal_t aff_sig, unsigned int sig_id){
	unsigned int aff_sig_id = get_id(aff_sig);
	connections->push_back(new Nemo_Connection(aff_sig_id, sig_id));
}

void Nemo_Design::add_connection(ivl_signal_t aff_sig, ivl_signal_t sig){
	unsigned int aff_sig_id = get_id(aff_sig);
	unsigned int sig_id     = get_id(sig);
	connections->push_back(new Nemo_Connection(aff_sig_id, sig_id));
}

// void Nemo_Design::add_connection_to_nemo_signal(unsigned int aff_sig_id, unsigned int sig_id){
// 	if (nemo_sigs != NULL){
// 		(*nemo_sigs)[aff_sig_id]->add_connection(sig_id);
// 	}
// 	fprintf(stderr, "ERROR: cannot add connection to NULL nemo signals.\n");
// }

Nemo_Signal* Nemo_Design::add_spliced_signal(ivl_signal_t new_sig){
	spliced_nemo_sigs->push_back(new Nemo_Signal(new_sig, num_sigs++));
	num_spliced_sigs++;
	return spliced_nemo_sigs->back();
}

void Nemo_Design::debug_print_all_nemo_sigs(){
	if (nemo_sigs != NULL){
		for(unsigned int i = 0; i<nemo_sigs->size(); i++){
			(*nemo_sigs)[i]->debug_print_nemo_sig();
		}
	}
}

void Nemo_Design::load_design_signals(){
	nemo_sigs = new vector<Nemo_Signal*>();

	// Load all signals in the design
	if (!fexists(SIGS_PB_FILE)){
		// Protobuf file does not exist: find all signals in the iverilog design
		printf("Cannot find signals protobuf file (%s), parsing IVL structure instead ...\n", SIGS_PB_FILE);
		for (unsigned int i = 0; i < num_root_scopes; i++) {
			load_signals_from_ivl(root_scopes[i]);
		}
		printf("Done parsing IVL signals.\n\n");
	} else{
		// Load signals from protobuf file
		printf("Found signal protobufs file (%s)!\n", SIGS_PB_FILE);
		load_signals_from_pb();
	}

	// Load all spliced signals in the design
	if (fexists(SPLICED_SIGS_PB_FILE)){
		printf("Found signal protobufs file (%s)!\n", SPLICED_SIGS_PB_FILE);
		load_spliced_signals_from_pb();
	}
}

void Nemo_Design::load_signals_from_pb(){
	int fd = open(SIGS_PB_FILE, O_RDONLY);
	ZeroCopyInputStream* raw_input   = new FileInputStream(fd);
	CodedInputStream*    coded_input = new CodedInputStream(raw_input);
	
	signals_loaded_from_pb = true;
	
	// Read num_sigs from PB file
	coded_input->ReadLittleEndian32(&num_sigs);
	if (DEBUG_PRINTS){
		printf("Number of Signals to Load: %d\n", num_sigs);
	}

	// Read signal objects from PB file
	printf("Loading signal protobufs from %s ...\n", SIGS_PB_FILE);
	for(unsigned int i=0; i<num_sigs; i++){
		nemo_sigs->push_back(new Nemo_Signal());
		if (DEBUG_PRINTS){
			printf("\n---- Before Signal Loaded: ----\n");
			nemo_sigs->back()->debug_print_nemo_sig();
		}
		if (!nemo_sigs->back()->read_pb_from_file(coded_input)){
			fprintf(stderr, "ERROR: failed to parse protobuf file %s.\n", SIGS_PB_FILE);
		 	exit(-4);
		}
		if (DEBUG_PRINTS){
			printf("---- After Signal Loaded: ----\n");
			nemo_sigs->back()->debug_print_nemo_sig();
		}
	}
	printf("Done loading signal protobufs.\n\n");

	delete coded_input;
	delete raw_input;
	close(fd);
}

void Nemo_Design::load_spliced_signals_from_pb(){
	int fd = open(SIGS_PB_FILE, O_RDONLY);
	ZeroCopyInputStream* raw_input   = new FileInputStream(fd);
	CodedInputStream*    coded_input = new CodedInputStream(raw_input);
	
	// Read num_sigs from PB file
	coded_input->ReadLittleEndian32(&num_spliced_sigs);
	if (DEBUG_PRINTS){
		printf("Number of Spliced Signals to Load: %d\n", num_spliced_sigs);
	}

	// Read signal objects from PB file
	printf("Loading spliced signal protobufs from %s ...\n", SPLICED_SIGS_PB_FILE);
	for(unsigned int i=0; i<num_spliced_sigs; i++){
		spliced_nemo_sigs->push_back(new Nemo_Signal());
		if (DEBUG_PRINTS){
			printf("\n---- Before Signal Loaded: ----\n");
			spliced_nemo_sigs->back()->debug_print_nemo_sig();
		}
		if (!spliced_nemo_sigs->back()->read_pb_from_file(coded_input)){
			fprintf(stderr, "ERROR: failed to parse protobuf file %s.\n", SPLICED_SIGS_PB_FILE);
		 	exit(-4);
		}
		if (DEBUG_PRINTS){
			printf("---- After Signal Loaded: ----\n");
			spliced_nemo_sigs->back()->debug_print_nemo_sig();
		}
	}
	printf("Done loading signal protobufs.\n\n");

	delete coded_input;
	delete raw_input;
	close(fd);
}

void Nemo_Design::load_signals_from_ivl(ivl_scope_t scope){
	//@TODO: Look more into dealing with scopes that are tasks or functions
	if (ivl_scope_type(scope) != IVL_SCT_MODULE && ivl_scope_type(scope) != IVL_SCT_BEGIN && ivl_scope_type(scope) != IVL_SCT_TASK ) {
		fprintf(stderr, "ERROR: cannot parse scope type (%d)\n", ivl_scope_type(scope));
		fprintf(stderr, "File: %s Line: %d\n", ivl_scope_file(scope), ivl_scope_lineno(scope));
		return;
	}

	// Rescurse into any submodules
	for (unsigned int i = 0; i < ivl_scope_childs(scope); i++) {
		load_signals_from_ivl(ivl_scope_child(scope, i));
	}

	// Enumerate all signals in each scope
	unsigned     num_scope_sigs = ivl_scope_sigs(scope);
	ivl_signal_t current_ivl_signal;
	for (unsigned idx = 0; idx < num_scope_sigs; idx++) {
		current_ivl_signal = ivl_scope_sig(scope, idx);
		ivl_sigs.push_back(current_ivl_signal);
		nemo_sigs->push_back(new Nemo_Signal(current_ivl_signal, num_sigs));
		ivl2nemo[current_ivl_signal] = num_sigs++;
	}
}

void Nemo_Design::load_design_connections(){
	connections       = new vector<Nemo_Connection*>();
	spliced_nemo_sigs = new vector<Nemo_Signal*>();

	if (!fexists(CONS_PB_FILE)){
		// Protobuf file does not exist: find all connections in the iverilog design
		printf("Cannot find connections protobuf file (%s), parsing IVL structure instead ...\n", CONS_PB_FILE);
		load_connections_from_ivl();
		printf("Done parsing IVL connections.\n\n");
	} else{
		// Load connections from protobuf file
		printf("Found connection protobufs file (%s)!\n", CONS_PB_FILE);
		load_connections_from_pb();
	}
}

void Nemo_Design::load_connections_from_pb(){
	int fd = open(CONS_PB_FILE, O_RDONLY);
	ZeroCopyInputStream* raw_input   = new FileInputStream(fd);
	CodedInputStream*    coded_input = new CodedInputStream(raw_input);
	
	connections_loaded_from_pb = true;
	
	// Read num_conns from PB file
	coded_input->ReadLittleEndian32(&num_conns);
	if (DEBUG_PRINTS){
		printf("Number of Connections to Load: %d\n", num_conns);
	}

	// Read connection objects from PB file
	printf("Loading connection protobufs from %s ...\n", CONS_PB_FILE);
	for(unsigned int i=0; i<num_conns; i++){
		connections->push_back(new Nemo_Connection());
		if (!connections->back()->read_pb_from_file(coded_input)){
			fprintf(stderr, "ERROR: failed to parse protobuf file %s.\n", CONS_PB_FILE);
		 	exit(-4);
		}
	}
	printf("Done loading connection protobufs.\n\n");

	delete coded_input;
	delete raw_input;
	close(fd);
}

void Nemo_Design::load_connections_from_ivl(){
	for (vector<ivl_signal_t>::iterator it = ivl_sigs.begin(); it != ivl_sigs.end(); ++it){
		//@TODO: Support more than 1 dimension vector
		//       Though it looks like it should be ok for OR1200
		assert(ivl_signal_packed_dimensions(*it) <= 1);
		propagate_sig(*it);
	}
}
