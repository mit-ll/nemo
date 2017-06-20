#include "ivl_target.h"

#include "ttb.h"
#include "ttb_signal.h"

#include <cassert>

// Logic devices are easy!
void propagate_log(const ivl_net_logic_t logic, TTB_Signal& aff_sig, std::vector<connection_t>& connections) {
  unsigned npins = ivl_logic_pins(logic);
  ivl_nexus_t input;
  TTB_Signal sig;
  
  // Propagate over all input pins
  // pin 0 is output
  for (unsigned i = 1; i < npins; i++) {
    input = ivl_logic_pin(logic, i);
    assert(!ivl_nexus_ptr_log(ivl_nexus_ptr(input, 0)) && "Logic unit connected directly to logic unit");
    assert(!ivl_nexus_ptr_lpm(ivl_nexus_ptr(input, 0)) && "Logic unit connected directly to lpm");
    sig = ivl_nexus_ptr_sig(ivl_nexus_ptr(input, 0));
    if (sig.get_sig()) {
      print_connection(aff_sig, sig, connections);
    }
  }
}
