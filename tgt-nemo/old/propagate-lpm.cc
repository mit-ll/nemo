#include <cassert>

#include "ivl_target.h"
#include "nemo_design.h"

/**
 * Finds the width (in bits) of a nexus
 */
unsigned Nemo_Design::get_nexus_width(ivl_nexus_t nex) {
	ivl_signal_t sig = 0;

	for (unsigned idx = 0 ; idx < ivl_nexus_ptrs(nex) ; idx += 1) {
		ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex,idx);
		sig = ivl_nexus_ptr_sig(ptr);
		if (sig) return ivl_signal_width(sig);
	}

	assert(false && "Unable to find nexus width");

	return 0;
} 

void Nemo_Design::add_lpm_connection(ivl_nexus_t lpm_input_nex, const ivl_signal_t aff_sig, ivl_lpm_t lpm){
	ivl_signal_t    sig;
	ivl_lpm_t       prev_lpm;
	ivl_nexus_ptr_t curr_input_nex_ptr = NULL;

	// Iterate over nexus pointers
	for (unsigned int i = 0; i < ivl_nexus_ptrs(lpm_input_nex); i++) {
		curr_input_nex_ptr = ivl_nexus_ptr(lpm_input_nex, i);
		if ((sig = ivl_nexus_ptr_sig(curr_input_nex_ptr))){
			// new_nemo_sig = add_spliced_signal(sig);
			// if (lpm) {
			// 	lsb = ivl_lpm_base(lpm);
			// 	msb = lsb + ivl_lpm_width(lpm) - 1;
			// 	new_sig->set_lsb(lsb);
			// 	new_sig->set_msb(msb);
			// }
			add_connection(aff_sig, sig);
		}
		else if ((prev_lpm = ivl_nexus_ptr_lpm(curr_input_nex_ptr))){
			// Recurse
			propagate_lpm(prev_lpm, aff_sig);
		}
		else{
			fprintf(stderr, "ERROR: unsupported LPM input device.\n");
		}
	}
}

void Nemo_Design::propagate_lpm(const ivl_lpm_t lpm, ivl_signal_t aff_sig) {
	const ivl_lpm_type_t lpm_type   = ivl_lpm_type(lpm);
	const unsigned int   aff_sig_id = get_id(aff_sig);
	ivl_nexus_t          input_nexus;
	ivl_signal_t         sig;

	switch (lpm_type) {
		/* part select: vector to part (part select in rval) */
		case IVL_LPM_PART_VP:
			//@TODO: Support non-constant base (and figure out what that means)...
			if (ivl_lpm_data(lpm, 1)) {
				fprintf(stderr, "ERROR: LPM_PART_VP with non constant base not supported\n");
				fprintf(stderr, "File: %s Line: %d\n", ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
				break;
			}
			input_nexus = ivl_lpm_data(lpm, 0); // extract input (0) pin nexus
			add_lpm_connection(input_nexus, aff_sig, lpm);
			break;

		/* part select: part select to vector (part select in lval) */
		case IVL_LPM_PART_PV:
			//@TODO: Support non-constant base (and figure out what that means)...
			if (ivl_lpm_data(lpm, 1)) {
				fprintf(stderr, "ERROR: LPM_PART_VP with non constant base not supported\n");
				fprintf(stderr, "File: %s Line: %d\n", ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
				break;
			}
			// get_nemo_sig(aff_sig_id)->set_lsb(ivl_lpm_base(lpm));
			// get_nemo_sig(aff_sig_id)->set_msb(ivl_lpm_base(lpm) + ivl_lpm_width(lpm) - 1);
			input_nexus = ivl_lpm_data(lpm, 0);
			add_lpm_connection(input_nexus, aff_sig, lpm);
			break;

		case IVL_LPM_CONCAT:
		case IVL_LPM_CONCATZ: {
			/* Transparent concat */
			// unsigned long lsb      = get_nemo_sig(aff_sig_id)->get_lsb();
			// unsigned long curr_lsb = lsb;
			// unsigned long msb      = get_nemo_sig(aff_sig_id)->get_msb();
			for (unsigned int idx = 0; idx < ivl_lpm_size(lpm); idx++) {
				input_nexus = ivl_lpm_data(lpm, idx);
				// get_nemo_sig(aff_sig_id)->set_lsb(curr_lsb);
				// get_nemo_sig(aff_sig_id)->set_msb(curr_lsb + get_nexus_width(input_nexus) - 1);
				// curr_lsb = curr_lsb + get_nexus_width(input_nexus);
				add_lpm_connection(input_nexus, aff_sig, 0);
			}
			// get_nemo_sig(aff_sig_id)->set_msb(msb);
			// get_nemo_sig(aff_sig_id)->set_lsb(lsb);
			}
			break;

		default:
			fprintf(stderr, "ERROR: Unsupported LPM type %d\n", lpm_type);
			fprintf(stderr, "File: %s Line: %d\n", ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
			return;
			break;
	}
}
