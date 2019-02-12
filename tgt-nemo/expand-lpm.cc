// -----------------------------------------------------------------------------
// File:        expand-lpm.cc
// Author:      Timothy Trippel
// Affiliation: MIT Lincoln Laboratory
// -----------------------------------------------------------------------------

#include <cassert>

#include "nemo.h"

int expand_lpm(const ivl_lpm_t lpm, ivl_signal_t aff_sig, Dot_File& df, set<ivl_signal_t>& sigs_to_expand, set<ivl_signal_t>& explored_sigs, bool expand_search) {
	// LPM Type
	const ivl_lpm_type_t lpm_type = ivl_lpm_type(lpm);
	
	// Device Pointers
	ivl_signal_t    sig;

	// Nexus Pointers
	ivl_nexus_t     input_pin_nexus;
	ivl_nexus_ptr_t nexus_ptr;

	// Depth Explored
	int depth_explored = 1;

	pair<set<ivl_signal_t>::iterator, bool> insert_ret;

	switch (lpm_type) {
		/* part select: vector to part (part select in rval) */
		case IVL_LPM_PART_VP:
			if (DEBUG_PRINTS){ printf("		LPM (%s) is of type (%d) IVL_LPM_PART_PV\n", ivl_lpm_basename(lpm), lpm_type); }
			//@TODO: Support non-constant base (and figure out what that means)...
			if (ivl_lpm_data(lpm, 1)) {
				fprintf(stderr, "ERROR: LPM_PART_VP/PV with non constant base not supported\n");
				fprintf(stderr, "File: %s Line: %d\n", ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
				depth_explored = 0;
				break;
			}
			
			// extract input pin (pin-0) nexus for LPM device
			input_pin_nexus = ivl_lpm_data(lpm, 0);

			// Iterate over all nexus pointers for LPM input pin 
			// nexus to find the input signal
			if (DEBUG_PRINTS){ printf("			num nexus pointers at input = %d\n", ivl_nexus_ptrs(input_pin_nexus)); }
			for (unsigned int i = 0; i < ivl_nexus_ptrs(input_pin_nexus); i += 1){	
				nexus_ptr = ivl_nexus_ptr(input_pin_nexus, i);
				// IVL creates local (compiler generated) signals as the output of 
				// every LPM device and constant. Therefore it is only necessary to look 
				// for signal devices as inputs to the LPM, not any other devices
				if ((sig = ivl_nexus_ptr_sig(nexus_ptr))){
					// Do not expand local IVL compiler generated signals
					// unless they are outputs of constants
					if (!is_ivl_generated_signal(sig)){
						if (DEBUG_PRINTS){ printf("				input is a SIGNAL device (%s.%s).", ivl_scope_name(ivl_signal_scope(sig)), ivl_signal_basename(sig)); }
						df.add_connection(aff_sig, sig, (ivl_lpm_base(lpm) + ivl_lpm_width(lpm) - 1), ivl_lpm_base(lpm));
						if (expand_search && !ivl_signal_local(sig) && !is_sig_expanded(explored_sigs, sig)){
							insert_ret = sigs_to_expand.insert(sig);
							if (insert_ret.second && DEBUG_PRINTS){ printf(" Expanded."); }
						}
						if (DEBUG_PRINTS){ printf("\n"); }
					}
				}
			}
			break;
		/* part select: part select to vector (part select in lval) */
		case IVL_LPM_PART_PV:
			//@TODO: Support non-constant base (and figure out what that means)...
			if (ivl_lpm_data(lpm, 1)) {
				fprintf(stderr, "ERROR: LPM_PART_VP/PV with non constant base not supported\n");
				fprintf(stderr, "File: %s Line: %d\n", ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
				depth_explored = 0;
				break;
			}
			if (DEBUG_PRINTS){ printf("		LPM (%s) is of type (%d) IVL_LPM_PART_PV\n", ivl_lpm_basename(lpm), lpm_type); }
			
			// extract input pin (pin-0) nexus for LPM device
			input_pin_nexus = ivl_lpm_data(lpm, 0); 

			// Iterate over all nexus pointers for LPM input pin 
			// nexus to find the input signal
			if (DEBUG_PRINTS){ printf("			num nexus pointers at input = %d\n", ivl_nexus_ptrs(input_pin_nexus)); }
			for (unsigned int i = 0; i < ivl_nexus_ptrs(input_pin_nexus); i += 1){	
				nexus_ptr = ivl_nexus_ptr(input_pin_nexus, i);
				// IVL creates local (compiler generated) signals as the output of 
				// every LPM device and constant. Therefore it is only necessary to look 
				// for signal devices as inputs to the LPM, not any other devices
				if ((sig = ivl_nexus_ptr_sig(nexus_ptr))){
					// Do not expand local IVL compiler generated signals
					// unless they are outputs of constants
					if (!is_ivl_generated_signal(sig)){
						// Nexus pointer points to a signal
						if (DEBUG_PRINTS){ printf("				input is a SIGNAL device (%s.%s).", ivl_scope_name(ivl_signal_scope(sig)), ivl_signal_basename(sig)); }
						df.add_connection(aff_sig, (ivl_lpm_base(lpm) + ivl_lpm_width(lpm) - 1), ivl_lpm_base(lpm), sig);
						if (expand_search && !ivl_signal_local(sig) && !is_sig_expanded(explored_sigs, sig)){
							insert_ret = sigs_to_expand.insert(sig);
							if (insert_ret.second && DEBUG_PRINTS){ printf(" Expanded."); }
						}
						if (DEBUG_PRINTS){ printf("\n"); }
					}
				}
			}
			break;
		case IVL_LPM_CONCAT:
		case IVL_LPM_CONCATZ: {
			if (DEBUG_PRINTS){ 
				printf("		LPM (%s) is of type (%d) IVL_LPM_CONCAT/IVL_LPM_CONCATZ\n", ivl_lpm_basename(lpm), lpm_type);
				printf("		num inputs = %d\n", ivl_lpm_size(lpm));
			}
			unsigned long curr_lsb      = ivl_signal_packed_lsb(aff_sig, 0);

			// Iterate over inputs to concatenation LPM
			for (unsigned int idx = 0; idx < ivl_lpm_size(lpm); idx += 1) {
				input_pin_nexus = ivl_lpm_data(lpm, idx);

				// Iterate over all nexus pointers for LPM input pin
				// nexus to find the input signals
				if (DEBUG_PRINTS){ printf("			num nexus pointers at input %d = %d\n", idx, ivl_nexus_ptrs(input_pin_nexus)); }
				for (unsigned int i = 0; i < ivl_nexus_ptrs(input_pin_nexus); i += 1){
					nexus_ptr = ivl_nexus_ptr(input_pin_nexus, i);
					if ((sig = ivl_nexus_ptr_sig(nexus_ptr))) {
						// Do not expand local IVL compiler generated signals
						// unless they are outputs of constants
						if (!is_ivl_generated_signal(sig)){
							// Nexus pointer points to a signal
							if (DEBUG_PRINTS){ printf("				input is a SIGNAL device (%s.%s).", ivl_scope_name(ivl_signal_scope(sig)), ivl_signal_basename(sig)); }
							df.add_connection(aff_sig, (curr_lsb + ivl_signal_width(sig) - 1), curr_lsb, sig);
							if (expand_search && !ivl_signal_local(sig) && !is_sig_expanded(explored_sigs, sig)){
								insert_ret = sigs_to_expand.insert(sig);
								if (insert_ret.second && DEBUG_PRINTS){ printf(" Expanded."); }
							}
							if (DEBUG_PRINTS){ printf("\n"); }
							curr_lsb += ivl_signal_width(sig);
						}
					}
				}
			}
			}
			break;
		default:
			fprintf(stderr, "ERROR: Unsupported LPM type %d\n", lpm_type);
			fprintf(stderr, "File: %s Line: %d\n", ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
			depth_explored = 0;
			break;
	}
	return depth_explored;
}
