TGT-TTB
=======

Overview
--------
This backend creates a dot file which shows the dependecies of different 
signals. The nodes of the dot files are signals and the edges are connections.
If one node points to another it means all or a part of the signal pointing
affects all or a part of the signal being pointed to.

How it Works
------------
Icarus Verilog output gives two main parts to look for connections in. First
is the nexus output. All signals which are not assigned in an always block are
connected with different objects using what are called nexuses. You can think
of a nexus as a wire just connecting signals with logic parts, such as AND
gates or muxes.

These nexuses can either connect to signals, lpms, or logic units. The signals
are what we are interested in. For each signal, we work our way backwards
through the lpms and logic devices until we hit another signal. Using this
knowledge we can determine which signals affect which other signals.

For signals inside of always blocks, we are left with the parse tree output
instead of the nexus connection. From there we must parse out the assignment
statements in order to determine which signals affect which other signals.

Where to Look
-------------
For parsing through always blocks, look at statement.cc
For propagating through nexuses, look at propagate_{lpm,log,sig}.cc
For functions you don't understand look at iVerilog's ivl_target.h and
ivl_target.txt file
