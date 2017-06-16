#include <cassert>
#include <cstdlib>
#include <cstdio>

#include "ivl_target.h"

#include "ttb.h"
#include "ttb_signal.h"

/* Signals are easy... ish */
void propagate_sig(TTB_Signal& aff_sig, std::vector<connection_t>& connections) {
	ivl_signal_t    	sig;
	ivl_net_logic_t 	log;
	ivl_lpm_t       	lpm;
	ivl_nexus_ptr_t 	nex_ptr;

	//First get its nexus
	//@TODO: Deal with larger vectors (more than one nexus)
	//@TODO: Figure out how more than one nexus works...
	const int base  = ivl_signal_array_base(aff_sig.get_sig());
	const int count = ivl_signal_array_count(aff_sig.get_sig());
	assert(count >= 0);
	if (count > 1) {
		fprintf(stderr, "Error: Skipping Arrayed Signal: %s\n", aff_sig.name().c_str());
		fprintf(stderr, "File: %s Line: %d\n", ivl_signal_file(aff_sig.get_sig()), ivl_signal_lineno(aff_sig.get_sig()));
		return;
	}

	for (int idx = base; idx < base + count; idx++) {
		const ivl_nexus_t nex = ivl_signal_nex(aff_sig.get_sig(), idx);
		assert(nex);

		for (unsigned i = 0; i < ivl_nexus_ptrs(nex); i++) {
			nex_ptr = ivl_nexus_ptr(nex, i);
			assert(nex_ptr);

			if ((sig = ivl_nexus_ptr_sig(nex_ptr))) {
				// Signal connected to another signal --> Means that the signals are the same. Usually happens on module hookups
				// Also if two signals in a module are hooked up to the same thing
				//@TODO: Not sure how to handle this yet
			}
			if ((log = ivl_nexus_ptr_log(nex_ptr))) {
				if (ivl_logic_pin(log, 0) == nex) {
					propagate_log(log, aff_sig, connections);
				}
			} else if ((lpm = ivl_nexus_ptr_lpm(nex_ptr))) {
				if (ivl_lpm_q(lpm) == nex) {
					propagate_lpm(lpm, aff_sig, connections);
				}
			}
		}
	}
}
