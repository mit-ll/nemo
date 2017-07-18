#### Nemo ####

Nemo is a custom Verilog compiler backend target module to be used along side the open source frontend Verilog compiler tool Icarus Verilog (http://iverilog.icarus.com/). Nemo allows an IC designer to create signal dependency graphs for any signal(s) in the design after it has been synthesized and mapped. 

Nemo is initially invoked by the Icarus Verilog (IVL) frontend compiler and interacts with the data structures created by IVL via the IVL API defined by in the "ivl_target.h" header file. In order to use Nemo, one must first build/install IVL before building/installing Nemo. 

Nemo takes as input a Verilog netlist (the output from a Verilog synthesis tool) and a Verilog netlist describing the standard cell modules used for synthesis. Nemo identifies critical signals in the input netlist based on a regular expression (defined in the "nemo.h" file) match and generates a signal dependency graph (to a graph depth also defined in the "nemo.h" file) to identify which signals in a design have the potential to influence a critical signal. Nemo outputs the critical signal dependency graph in DOT graph description language file.

Example Nemo input files, a Verilog netlist and a standard cell Verilog netlist, are available in the "netlists/" directory. Example Nemo output files, a DOT file and a PDF visual representation of the DOT file, are available in the "graphs/" directory.

#### Cloning Git Repositories ####

1. Clone IVL  Repo --> git clone git://github.com/steveicarus/iverilog.git
2. Clone Nemo Repo --> git clone

#### Building/Installing IVL ####

Detailed instructions on this process can be found here: http://iverilog.wikia.com/wiki/Installation_Guide, but a summary is provided below.

1. cd iverilog
2. sh autoconf.sh
3. ./configure --prefix=<full path of iverilog directory>
4. make install
5. cd ..

#### Building/Installing Nemo ####



#### Checkout ####

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