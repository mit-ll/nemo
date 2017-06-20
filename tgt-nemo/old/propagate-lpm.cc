#include <cassert>

#include "ivl_target.h"

#include "ttb.h"
#include "ttb_signal.h"

/**
 * Fines the width (in bits) of a nexus
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

static void print_connections(ivl_nexus_t nex, const TTB_Signal& aff_sig, ivl_lpm_t lpm, std::vector<connection_t>& connections) {
  TTB_Signal sig;
  for (unsigned i = 0; i < ivl_nexus_ptrs(nex); i++) {
    sig = ivl_nexus_ptr_sig(ivl_nexus_ptr(nex, i));
    if (sig.get_sig()) {
      if (lpm) {
        sig.set_lsb(ivl_lpm_base(lpm));
        sig.set_msb(sig.get_lsb() + ivl_lpm_width(lpm) - 1);
      }
      print_connection(aff_sig, sig, connections);
    }
  }
}

void propagate_lpm(const ivl_lpm_t lpm, TTB_Signal& aff_sig, std::vector<connection_t>& connections) {
  const ivl_lpm_type_t lpm_type = ivl_lpm_type(lpm);
  ivl_nexus_t input;
  TTB_Signal sig;

  switch (lpm_type) {

    case IVL_LPM_ARRAY:
      //@TODO: Fix this
      fprintf(stderr, "ERROR: LPM Array Found\n");
      fprintf(stderr, "File: %s Line: %d\n", ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
      //fprintf(out, "\tFrom %s\n", ivl_signal_basename(ivl_lpm_array(lpm)));
      break;

    case IVL_LPM_MUX: 
      input = ivl_lpm_select(lpm);
      print_connections(input, aff_sig, 0, connections);
      for (unsigned idx = 0; idx < ivl_lpm_size(lpm); idx++) {
        input = ivl_lpm_data(lpm, idx);
        print_connections(input, aff_sig, 0, connections);
      }
      break;

    /* part select: vector to part (part select in rval) */
    case IVL_LPM_PART_VP:
      //@TODO: Support non-constant base (and figure out what that means)...
      if (ivl_lpm_data(lpm, 1)) {
        fprintf(stderr, "ERROR: LPM_PART_VP with non constant base not supported\n");
        fprintf(stderr, "File: %s Line: %d\n", ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
        break;
      }
      input = ivl_lpm_data(lpm, 0);
      print_connections(input, aff_sig, lpm, connections);
      break;

    /* part select: part select to vector (part select in lval) */
    case IVL_LPM_PART_PV:
      //@TODO: Support non-constant base (and figure out what that means)...
      if (ivl_lpm_data(lpm, 1)) {
        fprintf(stderr, "ERROR: LPM_PART_VP with non constant base not supported\n");
        fprintf(stderr, "File: %s Line: %d\n", ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
        break;
      }
      aff_sig.set_lsb(ivl_lpm_base(lpm));
      aff_sig.set_msb(ivl_lpm_base(lpm) + ivl_lpm_width(lpm));
      input = ivl_lpm_data(lpm, 0);
      print_connections(input, aff_sig, lpm, connections);
      break;

    case IVL_LPM_CONCAT:
    case IVL_LPM_CONCATZ: /* Transparent concat */ {
      unsigned long lsb = aff_sig.get_lsb();
      unsigned long curr_lsb = lsb;
      unsigned long msb = aff_sig.get_msb();
      for (unsigned idx = 0; idx < ivl_lpm_size(lpm); idx++) {
        input = ivl_lpm_data(lpm, idx);
        aff_sig.set_lsb(curr_lsb);
        aff_sig.set_msb(curr_lsb + get_nexus_width(input) - 1);
        curr_lsb = curr_lsb + get_nexus_width(input);
        print_connections(input, aff_sig, 0, connections);
      }
      aff_sig.set_msb(msb);
      aff_sig.set_lsb(lsb);
      }
      break;
      

    case IVL_LPM_ADD:
    case IVL_LPM_DIVIDE:
    case IVL_LPM_CMP_EEQ: /* Case EQ (===) */
    case IVL_LPM_CMP_EQ:
    case IVL_LPM_CMP_GE:
    case IVL_LPM_CMP_GT:
    case IVL_LPM_CMP_NE:
    case IVL_LPM_CMP_NEE: /* Case NE (!==) */
    case IVL_LPM_MULT:
    case IVL_LPM_POW:
    case IVL_LPM_RE_AND:
    case IVL_LPM_RE_NOR:
    case IVL_LPM_RE_OR:
    case IVL_LPM_SHIFTL:
    case IVL_LPM_SHIFTR:
    case IVL_LPM_SUB:
      for (unsigned idx = 0; idx < ivl_lpm_size(lpm); idx++) {
        input = ivl_lpm_data(lpm, idx);
        print_connections(input, aff_sig, 0, connections);
      }
      break;

    default:
      fprintf(stderr, "ERROR: Unsupported LPM type %d\n", lpm_type);
      fprintf(stderr, "File: %s Line: %d\n", ivl_lpm_file(lpm), ivl_lpm_lineno(lpm));
      return;
      break;
  }
}
