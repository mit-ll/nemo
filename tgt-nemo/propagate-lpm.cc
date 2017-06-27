#include <cassert>

#include "ivl_target.h"
#include "ttb.h"

/**
 * Finds the width (in bits) of a nexus
 */
static unsigned get_nexus_width(ivl_nexus_t nex) {
	ivl_signal_t sig = 0;

	for (unsigned idx = 0 ; idx < ivl_nexus_ptrs(nex) ; idx += 1) {
		ivl_nexus_ptr_t ptr = ivl_nexus_ptr(nex,idx);
		sig = ivl_nexus_ptr_sig(ptr);
		if (sig) return ivl_signal_width(sig);
	}

	assert(false && "Unable to find nexus width");

	return 0;
} 

static void add_lpm_connection(ivl_nexus_t nex, const ivl_signal_t aff_sig, ivl_lpm_t lpm, Nemo_Design& nemo_des) {
	ivl_signal_t  sig;
	unsigned int  nsig_id;
	unsigned long lsb;
	unsigned long msb;

	// Iterate over nexus pointers
	for (unsigned int i = 0; i < ivl_nexus_ptrs(nex); i++) {
		if ((sig = ivl_nexus_ptr_sig(ivl_nexus_ptr(nex, i)))){
			nsig_id = nemo_des.add_duplicate_from_ivl(sig);
			if (lpm) {
				lsb = ivl_lpm_base(lpm);
				msb = lsb + ivl_lpm_width(lpm) - 1;
				nemo_des.get_nemo_sig(nsig_id).set_lsb(lsb);
				nemo_des.get_nemo_sig(nsig_id).set_msb(msb);
			}
			nemo_des.add_connection(aff_sig, nsig_id);
		}
	}
}

void propagate_lpm(const ivl_lpm_t lpm, ivl_signal_t aff_sig, Nemo_Design& nemo_des) {
	const ivl_lpm_type_t lpm_type   = ivl_lpm_type(lpm);
	const unsigned int   aff_sig_id = nemo_des.get_id(aff_sig);
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
			input_nexus = ivl_lpm_data(lpm, 0);
			add_lpm_connection(input_nexus, aff_sig, lpm, nemo_des);
			break;

		/* part select: part select to vector (part select in lval) */
		case IVL_LPM_PART_PV:
			//@TODO: Support non-constant base (and figure out what that means)...
			if (ivl_lpm_data(lpm, 1)) {
				fprintf(stderr, "ERROR: LPM_PART_VP with non constant base not supported\n");
				fprintf(stderr, "File: %s Line: %d\n", ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
				break;
			}
			nemo_des.get_nemo_sig(aff_sig_id).set_lsb(ivl_lpm_base(lpm));
			nemo_des.get_nemo_sig(aff_sig_id).set_msb(ivl_lpm_base(lpm) + ivl_lpm_width(lpm));
			input_nexus = ivl_lpm_data(lpm, 0);
			add_lpm_connection(input_nexus, aff_sig, lpm, nemo_des);
			break;

		case IVL_LPM_CONCAT:
		case IVL_LPM_CONCATZ: {
			/* Transparent concat */
			unsigned long lsb      = nemo_des.get_nemo_sig(aff_sig_id).get_lsb();
			unsigned long curr_lsb = lsb;
			unsigned long msb      = nemo_des.get_nemo_sig(aff_sig_id).get_msb();
			for (unsigned int idx = 0; idx < ivl_lpm_size(lpm); idx++) {
				input_nexus = ivl_lpm_data(lpm, idx);
				nemo_des.get_nemo_sig(aff_sig_id).set_lsb(curr_lsb);
				nemo_des.get_nemo_sig(aff_sig_id).set_msb(curr_lsb + get_nexus_width(input_nexus) - 1);
				curr_lsb = curr_lsb + get_nexus_width(input_nexus);
				add_lpm_connection(input_nexus, aff_sig, 0, nemo_des);
			}
			nemo_des.get_nemo_sig(aff_sig_id).set_msb(msb);
			nemo_des.get_nemo_sig(aff_sig_id).set_lsb(lsb);
			}
			break;

		default:
			fprintf(stderr, "ERROR: Unsupported LPM type %d\n", lpm_type);
			fprintf(stderr, "File: %s Line: %d\n", ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
			return;
			break;
	}
}
