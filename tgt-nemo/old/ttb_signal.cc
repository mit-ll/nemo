#include "ttb_signal.h"

#include <cassert>

TTB_Signal::TTB_Signal() : sig(0), msb(0), lsb(0){}

TTB_Signal::TTB_Signal(ivl_signal_t s) {
	set_from_sig_t(s);
}

TTB_Signal::TTB_Signal(ivl_signal_t s, ulong m, ulong l)
	: msb(m), lsb(l) {
		set_from_sig_t(s);
}

TTB_Signal& TTB_Signal::operator=(const ivl_signal_t s) {
	set_from_sig_t(s);
	return *this;
}

void TTB_Signal::set_lsb(ulong l) {
	lsb = l;
}

void TTB_Signal::set_msb(ulong m) {
	msb = m;
}

void TTB_Signal::set_isinput() {
	sig_type = SIG_INPUT;
}

void TTB_Signal::set_isff() {
	sig_type = SIG_FF;
}

ulong TTB_Signal::get_lsb() const {
	return lsb;
}

ulong TTB_Signal::get_msb() const {
	return msb;
}

ivl_signal_t TTB_Signal::get_sig() const {
	return sig;
}

bool TTB_Signal::is_ff() const {
	return sig_type == SIG_FF;
}

bool TTB_Signal::is_input() const {
	return sig_type == SIG_INPUT;
}

const char* TTB_Signal::basename() const {
	assert(sig);
	return ivl_signal_basename(sig);
}

const std::string& TTB_Signal::name() const {
	return fullname;
}

void TTB_Signal::set_from_sig_t(const ivl_signal_t s) {
	if (!(sig = s)) return;
	unsigned num_dimens = ivl_signal_packed_dimensions(s);
	if (num_dimens == 0) {
		msb = lsb = 0;
	} else if (num_dimens == 1) {
		msb = ivl_signal_packed_msb(s, 0);
		lsb = ivl_signal_packed_lsb(s, 0);
	} else {
		assert(false && "Unsupported number of dimensions");
	}
	ivl_scope_t scope = ivl_signal_scope(s);

	// Yea.... This is bad
	fullname = std::string(ivl_scope_name(scope)) + std::string(".") + std::string(ivl_signal_basename(s));

	if (IVL_SIP_INPUT == ivl_signal_port(sig)) {
		sig_type = SIG_INPUT;
	} else {
		sig_type = SIG_NONE;
	}
}
