# Nemo

Nemo is a custom Verilog compiler backend target module to be used along side the open source frontend Verilog compiler tool Icarus Verilog (http://iverilog.icarus.com/). Nemo allows an IC designer to create signal dependency graphs for any signal(s) in the design after it has been synthesized and mapped. Nemo takes as input a Verilog netlist (the output from a Verilog synthesis tool) and a Verilog description of the standard cell modules inputs and outputs. Nemo identifies critical signals in the input netlist based on a regular expression (defined in the "nemo.h" file) match and generates a signal dependency graph to identify which signals in a design have the potential to influence a critical signal. Nemo outputs the critical signal dependency graph in DOT graph description language file.



To make detecting distributed counter-registers practical, we use
connection information from the circuit in question to limit the
register combinations checked by our existing analysis flow. We
represent connection information as a dataflow graph. Creating a
dataflow graph for a circuit requires parsing the textual description
of the circuit and connecting the individual assignments to form a
graph. To maximize compatibility and reduce engineering effort, we use
Icarus Verilog to parse circuits described in Verilog. Once parsed we
walk the parse tree to build a dataflow graph for the
circuit. Finally, we use the dataflow graph to determine all possible
combinations of registers in a design. The resulting combinations feed
directly into our existing flow for detecting coalesced counter
registers.

Icarus Verilog supports adding functionality via modules called
backends. Backends interact with Icarus Verilog via the backend dll
API. Using the API means that we should be able to build our backend
(tgt-ttb) independently from Icarus Verilog, but for now we need to
compile our backend and Icarus Verilog together.

# Checkout

Icarus Verilog is a submodule in our project repository. This makes it
easy for us to track with the main Icarus Verilog repository. Using
submodules requires that after cloning the project repository, we
must checkout Icarus Verilog using,

git submodule init
git submodule update

After that, every time you pull, you must run the command below to
update the Icarus Verilog submodule.

git submodule update

Building
========
As mentioned above, we build our dll backend (tgt-ttb) with iverilog currently.
Though tgt-ttb is a dll, we still have to recompile all of Icarus Verilog
since I have not been able to decouple the backend build from the rest of 
Icarus Verilog.