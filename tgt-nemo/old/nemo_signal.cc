#include <cstdio>
#include "nemo_signal.h"

Nemo_Signal::Nemo_Signal(){
	sig = new Nemo_Signal_PB();
}

Nemo_Signal::Nemo_Signal(ivl_signal_t s, unsigned int s_id){
	sig = new Nemo_Signal_PB();
	init_from_ivl(s, s_id);
}

Nemo_Signal::~Nemo_Signal(){
	if (sig){
		delete sig;
	}
}

bool Nemo_Signal::write_pb_to_file(CodedOutputStream* strm){
	strm->WriteLittleEndian32(sig->ByteSize());
	if (strm->HadError()){
		return false;
	}
	if (!sig->SerializeToCodedStream(strm)){
		return false;
	}
	return true;
}

unsigned int Nemo_Signal::get_id() const{
	return sig->id();
}

unsigned long Nemo_Signal::get_lsb() const{
	return sig->lsb();
}

unsigned long Nemo_Signal::get_msb() const{
	return sig->msb();
}

void Nemo_Signal::set_lsb(unsigned long lsb){
	sig->set_lsb(lsb);
}

void Nemo_Signal::set_msb(unsigned long msb){
	sig->set_msb(msb);
}

bool Nemo_Signal::is_ff() const{
	return (sig->type() == Nemo_Signal_PB::SIG_FF);
}

bool Nemo_Signal::is_input() const{
	return (sig->type() == Nemo_Signal_PB::SIG_INPUT);
}

int Nemo_Signal::is_signal_local() const{
	return sig->local();
}

unsigned int Nemo_Signal::get_dimensions() const{
	return sig->dimensions();
}

const string& Nemo_Signal::scope_name() const{
	return sig->scope_name();
}

const string& Nemo_Signal::base_name() const{
	return sig->base_name();
}

const string& Nemo_Signal::full_name() const{
	return sig->full_name();
}

void Nemo_Signal::add_connection(unsigned int sig_id){
	sig->add_connections(sig_id);
}

bool Nemo_Signal::read_pb_from_file(CodedInputStream* strm){
	unsigned int size;
	int 		 read_limit;
	
	// Read size of protobuf
	if (!strm->ReadLittleEndian32(&size)){
		return false;
	}

	// Set read byte limit
	read_limit = strm->PushLimit(size);
	
	// Parse the protobuf message
	if(!sig->ParseFromCodedStream(strm)){
		return false;
	}
	if (!strm->ConsumedEntireMessage()){
		return false;
	}
	strm->PopLimit(read_limit);
	return true;
}

void Nemo_Signal::debug_print_nemo_sig(){
	printf("Signal ID   = %d\n", sig->has_id() ? sig->id() : -1);
	printf("Scope Name  = %s\n", sig->has_scope_name() ? sig->scope_name().c_str() : "none");
	printf("Base Name   = %s\n", sig->has_base_name() ? sig->base_name().c_str() : "none");
	printf("Full Name   = %s\n", sig->has_full_name() ? sig->full_name().c_str() : "none");
	printf("Type        = %d\n", sig->has_type() ? (int)sig->type() : -1);
	printf("Local       = %d\n", sig->has_local() ? sig->local() : -1);
	printf("Dimensions  = %d\n", sig->has_dimensions() ? sig->dimensions() : -1);
	printf("MSB         = %llu\n", sig->has_msb() ? sig->msb() : -1);
	printf("LSB         = %llu\n", sig->has_lsb() ? sig->lsb() : -1);
	printf("Connections = %d\n", sig->connections_size());
}

void Nemo_Signal::init_from_ivl(ivl_signal_t s, unsigned int s_id){
	// Check that ivl_sig is not NULL
	if (!(s)) return;

	// Set Signal ID number (index into signal array)
	sig->set_id(s_id);

	// Set Signal Dimenions
	sig->set_dimensions(ivl_signal_packed_dimensions(s));

	// Set MSB and LSB dimensions
	if (sig->dimensions() == 0) {
		sig->set_msb(0);
		sig->set_lsb(0);
	} else if (sig->dimensions() == 1) {
		sig->set_msb(ivl_signal_packed_msb(s, 0));
		sig->set_lsb(ivl_signal_packed_lsb(s, 0));
	} else {
		assert(false && "Unsupported number of dimensions");
	}

	// Retrieve Signal Scope
	ivl_scope_t scope = ivl_signal_scope(s);

	// Set signal names
	sig->set_scope_name(string(ivl_scope_name(ivl_signal_scope(s))));
	sig->set_base_name(string(ivl_signal_basename(s)));
	sig->set_full_name(string(ivl_scope_name(scope)) + string(".") + string(ivl_signal_basename(s)));

	// Set signal local value --> see <ivl_target.h> ivl_signal_local()
	sig->set_local(ivl_signal_local(s));

	// Set Signal Type
	if (IVL_SIP_INPUT == ivl_signal_port(s)) {
		sig->set_type(Nemo_Signal_PB::SIG_INPUT);
	} else {
		sig->set_type(Nemo_Signal_PB::SIG_NONE);
	}
}
