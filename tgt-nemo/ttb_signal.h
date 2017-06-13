#ifndef __TTB_SIGNAL__
#define __TTB_SIGNAL__

#include <string>

#include "ivl_target.h"

typedef unsigned long ulong;

//@TODO: Deal with arrayed signal

class TTB_Signal {
	public:
		TTB_Signal();
		TTB_Signal(ivl_signal_t s);
		TTB_Signal(ivl_signal_t s, ulong m, ulong l);

		TTB_Signal& operator=(ivl_signal_t s);

		void set_lsb(ulong l);
		void set_msb(ulong m);
		void set_isff();
		void set_isinput();

		ulong              get_lsb() const;
		ulong              get_msb() const;
		ivl_signal_t       get_sig() const;
		bool               is_ff() const;
		bool               is_input() const;
		const char*        basename() const;
		const std::string& name() const;

	private:
		enum sig_type_e {SIG_NONE, SIG_FF, SIG_INPUT};

		ivl_signal_t  sig;
		std::string   fullname;
		sig_type_e    sig_type;
		unsigned long msb;
		unsigned long lsb;

		void set_from_sig_t(ivl_signal_t s);
};
#endif