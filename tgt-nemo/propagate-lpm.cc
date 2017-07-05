#include <cassert>

#include "ivl_target.h"
#include "nemo.h"

static unsigned get_nexus_width(ivl_nexus_t nex) {
	ivl_signal_t    sig = NULL;
	ivl_net_const_t con = NULL;
	ivl_nexus_ptr_t ptr = NULL;

	for (unsigned idx = 0; idx < ivl_nexus_ptrs(nex); idx += 1) {
		ptr = ivl_nexus_ptr(nex, idx);
		sig = ivl_nexus_ptr_sig(ptr);
		con = ivl_nexus_ptr_con(ptr);

		if (sig) return ivl_signal_width(sig);
		if (con) return ivl_const_width(con);
	}

	assert(false && "Unable to find nexus width");

	return 0;
}

void propagate_lpm(const ivl_lpm_t lpm, ivl_signal_t aff_sig, Dot_File& df) {
	// LPM Type
	const ivl_lpm_type_t lpm_type = ivl_lpm_type(lpm);
	
	// Device Pointers
	ivl_signal_t    sig;
	ivl_net_const_t con;
	ivl_net_logic_t prev_logic;
	ivl_lpm_t       prev_lpm;

	// Nexus Pointers
	ivl_nexus_t     input_pin_nexus;
	ivl_nexus_ptr_t nexus_ptr;

	switch (lpm_type) {
		/* part select: vector to part (part select in rval) */
		case IVL_LPM_PART_VP:
			//@TODO: Support non-constant base (and figure out what that means)...
			if (ivl_lpm_data(lpm, 1)) {
				fprintf(stderr, "ERROR: LPM_PART_VP/PV with non constant base not supported\n");
				fprintf(stderr, "File: %s Line: %d\n", ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
				break;
			}
			if (DEBUG_PRINTS){ printf("		LPM (%s) is of type (%d) IVL_LPM_PART_PV\n", ivl_lpm_basename(lpm), lpm_type); }
			input_pin_nexus = ivl_lpm_data(lpm, 0); // extract input (0) pin nexus
			// Iterate over all nexus pointers for LPM input pin nexus
			if (DEBUG_PRINTS){ printf("			num nexus pointers at input = %d\n", ivl_nexus_ptrs(input_pin_nexus)); }
			for (unsigned int i = 0; i < ivl_nexus_ptrs(input_pin_nexus); i += 1){	
				nexus_ptr = ivl_nexus_ptr(input_pin_nexus, i);
				if ((prev_logic = ivl_nexus_ptr_log(nexus_ptr))){
					// Nexus pointer points to a logic device
					// if (DEBUG_PRINTS){ printf("				input %d is a LOGIC device.\n", i); }
					// propagate_log(prev_logic, aff_sig, df);
				} else if ((prev_lpm = ivl_nexus_ptr_lpm(nexus_ptr))){
					// if (prev_lpm != lpm){
						// Nexus pointer points to a DIFFERENT lpm device
						// if (DEBUG_PRINTS){ printf("				input %d is a LPM device.\n", i); }
						// propagate_lpm(prev_lpm, aff_sig, df);
					// }
				} else if ((sig = ivl_nexus_ptr_sig(nexus_ptr))){
					// Nexus pointer points to a signal
					if (DEBUG_PRINTS){ printf("				input %d is a SIGNAL device (%s).\n", i, ivl_signal_basename(sig)); }
					df.add_connection(aff_sig, sig, (ivl_lpm_base(lpm) + ivl_lpm_width(lpm) - 1), ivl_lpm_base(lpm));
				}
				else {
					assert(false && "ERROR: unsupported nexus pointer type input to LPM.\n");
				}
			}

			break;
		/* part select: part select to vector (part select in lval) */
		case IVL_LPM_PART_PV:
			//@TODO: Support non-constant base (and figure out what that means)...
			if (ivl_lpm_data(lpm, 1)) {
				fprintf(stderr, "ERROR: LPM_PART_VP/PV with non constant base not supported\n");
				fprintf(stderr, "File: %s Line: %d\n", ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
				break;
			}
			if (DEBUG_PRINTS){ printf("		LPM (%s) is of type (%d) IVL_LPM_PART_PV\n", ivl_lpm_basename(lpm), lpm_type); }
			input_pin_nexus = ivl_lpm_data(lpm, 0); // extract input (0) pin nexus

			// Iterate over all nexus pointers for LPM input pin nexus
			if (DEBUG_PRINTS){ printf("			num nexus pointers at input = %d\n", ivl_nexus_ptrs(input_pin_nexus)); }
			for (unsigned int i = 0; i < ivl_nexus_ptrs(input_pin_nexus); i += 1){	
				nexus_ptr = ivl_nexus_ptr(input_pin_nexus, i);
				if ((prev_logic = ivl_nexus_ptr_log(nexus_ptr))){
					// Nexus pointer points to a logic device
					// if (DEBUG_PRINTS){ printf("				input %d is a LOGIC device.\n", i); }
					// propagate_log(prev_logic, aff_sig, df);
				} else if ((prev_lpm = ivl_nexus_ptr_lpm(nexus_ptr))){
					// if (prev_lpm != lpm){
						// Nexus pointer points to a DIFFERENT lpm device
						// if (DEBUG_PRINTS){ printf("				input %d is a LPM device.\n", i); }
						// propagate_lpm(prev_lpm, aff_sig, df);
					// }
				} else if ((sig = ivl_nexus_ptr_sig(nexus_ptr))){
					// Nexus pointer points to a signal
					if (DEBUG_PRINTS){ printf("				input %d is a SIGNAL device (%s).\n", i, ivl_signal_basename(sig)); }
					df.add_connection(aff_sig, (ivl_lpm_base(lpm) + ivl_lpm_width(lpm) - 1), ivl_lpm_base(lpm), sig);
				} else {
					assert(false && "ERROR: unsupported nexus pointer type input to LPM.\n");
				}
			}
			break;

		case IVL_LPM_CONCAT:
		case IVL_LPM_CONCATZ: {
			if (DEBUG_PRINTS){ 
				printf("		LPM (%s) is of type (%d) IVL_LPM_CONCAT/IVL_LPM_CONCATZ\n", ivl_lpm_basename(lpm), lpm_type);
				printf("		num inputs = %d\n", ivl_lpm_size(lpm));
			}
			unsigned long lsb      = ivl_signal_packed_lsb(aff_sig, 0);
			unsigned long curr_lsb = lsb;
			unsigned long msb      = ivl_signal_packed_msb(aff_sig, 0);

			// Iterate over inputs to concatenation LPM
			for (unsigned int idx = 0; idx < ivl_lpm_size(lpm); idx += 1) {
				input_pin_nexus = ivl_lpm_data(lpm, idx);

				// Iterate over all nexus pointers for LPM input
				// Should only be 1 in this case for OR1200
				if (DEBUG_PRINTS){ printf("			num nexus pointers at input %d = %d\n", idx, ivl_nexus_ptrs(input_pin_nexus)); }
				for (unsigned int i = 0; i < ivl_nexus_ptrs(input_pin_nexus); i += 1){
					nexus_ptr = ivl_nexus_ptr(input_pin_nexus, i);
					if ((prev_logic = ivl_nexus_ptr_log(nexus_ptr))){
						// Nexus pointer points to a logic device
						// if (DEBUG_PRINTS){ printf("				input %d is a LOGIC device.\n", i); }
						// propagate_log(prev_logic, aff_sig, df);
						continue;
					} else if ((prev_lpm = ivl_nexus_ptr_lpm(nexus_ptr))){
						// if (prev_lpm != lpm){
							// Nexus pointer points to a DIFFERENT lpm device
							// if (DEBUG_PRINTS){ printf("				input %d is a LPM device.\n", i); }
							// propagate_lpm(prev_lpm, aff_sig, df);
						// }
						continue;
					} else if ((sig = ivl_nexus_ptr_sig(nexus_ptr))) {
						// Nexus pointer points to a signal
						if (DEBUG_PRINTS){ printf("				input %d is a SIGNAL device (%s).\n", i, ivl_signal_basename(sig)); }
						df.add_connection(aff_sig, msb, lsb, sig);
					} else if ((con = ivl_nexus_ptr_con(nexus_ptr))){
						// Nexus pointer points to a constant
						// if (DEBUG_PRINTS){ printf("				input %d is a CONSTANT device.\n", i); }
						// df.add_const_node(con);
						// df.add_const_connection(aff_sig, msb, lsb, con);
					} else {
						assert(false && "ERROR: unsupported nexus pointer type input to LPM.\n");
					}
				}
				curr_lsb += get_nexus_width(input_pin_nexus);
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
