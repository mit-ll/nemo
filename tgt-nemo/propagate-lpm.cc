#include <cassert>

#include "ivl_target.h"
#include "nemo.h"

void propagate_lpm(const ivl_lpm_t lpm, ivl_signal_t aff_sig, Dot_File& df) {
	// LPM Type
	const ivl_lpm_type_t lpm_type = ivl_lpm_type(lpm);
	
	// Device Pointers
	ivl_signal_t    sig;
	ivl_net_logic_t prev_logic;
	ivl_lpm_t       prev_lpm;

	// Nexus Pointers
	ivl_nexus_t     input_pin_nexus;
	ivl_nexus_ptr_t nexus_ptr;

	switch (lpm_type) {
		/* part select: vector to part (part select in rval) */
		/* part select: part select to vector (part select in lval) */
		case IVL_LPM_PART_VP:
		case IVL_LPM_PART_PV: {
			//@TODO: Support non-constant base (and figure out what that means)...
			if (ivl_lpm_data(lpm, 1)) {
				fprintf(stderr, "ERROR: LPM_PART_VP/PV with non constant base not supported\n");
				fprintf(stderr, "File: %s Line: %d\n", ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
				break;
			}
			input_pin_nexus = ivl_lpm_data(lpm, 0); // extract input (0) pin nexus

			// Iterate over all nexus pointers for LPM input pin nexus
			for (unsigned int i = 0; i < ivl_nexus_ptrs(input_pin_nexus); i++){
				nexus_ptr = ivl_nexus_ptr(input_pin_nexus, i);
				if ((prev_logic = ivl_nexus_ptr_log(nexus_ptr))){
					// Nexus pointer points to a logic device
					propagate_log(prev_logic, aff_sig, df);
				}
				if ((prev_lpm = ivl_nexus_ptr_lpm(nexus_ptr))){
					// Nexus pointer points to a lpm device
					propagate_lpm(prev_lpm, aff_sig, df);
				}
				if ((sig = ivl_nexus_ptr_sig(nexus_ptr))) {
					// Nexus pointer points to a signal
					df.add_connection(aff_sig, sig);
				}
			}
			}
			break;

		case IVL_LPM_CONCAT:
		case IVL_LPM_CONCATZ: {
			// Iterate over inputs to concatenation
			for (unsigned int idx = 0; idx < ivl_lpm_size(lpm); idx++) {
				input_pin_nexus = ivl_lpm_data(lpm, idx);
				// Iterate over all nexus pointers for LPM input pin nexus
				// Should only be 1 in this case for OR1200
				for (unsigned int i = 0; i < ivl_nexus_ptrs(input_pin_nexus); i++){
					printf("Number of concatenation inputs at nexus: %d\n", ivl_nexus_ptrs(input_pin_nexus));
					nexus_ptr = ivl_nexus_ptr(input_pin_nexus, i);
					if ((prev_logic = ivl_nexus_ptr_log(nexus_ptr))){
						// Nexus pointer points to a logic device
						propagate_log(prev_logic, aff_sig, df);
					}
					if ((prev_lpm = ivl_nexus_ptr_lpm(nexus_ptr))){
						// Nexus pointer points to a lpm device
						propagate_lpm(prev_lpm, aff_sig, df);
					}
					if ((sig = ivl_nexus_ptr_sig(nexus_ptr))) {
						// Nexus pointer points to a signal
						df.add_connection(aff_sig, sig);
					}
				}
			}
			}
			break;

		default:
			fprintf(stderr, "ERROR: Unsupported LPM type %d\n", lpm_type);
			fprintf(stderr, "File: %s Line: %d\n", ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
			return;
			break;
	}
}
