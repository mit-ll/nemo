BASE_IVERILOG_DIR=/Users/ti27457/Repos/iverilog
EXEC_DIR=/Users/ti27457/Repos/iverilog/lib/ivl

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644

IDIR=../../iverilog
CC=g++
CFLAGS=-I$(IDIR) -fpic -std=gnu++0x -Wall -Wextra -Wshadow -g -O2

TGT=nemo
SRCS=$(wildcard *.cc)
OBJS=$(SRCS:.cc=.o)

# TOP_MODULE=test
TOP_MODULE=orpsoc_top

# NETLIST=../netlists/test.v ../netlists/test_std_cell_defs.vm
NETLIST=../netlists/orpsoc_top.high_200MHz.nl.v ../netlists/std_cell_defs.vm

all: $(TGT) confs install

$(TGT): $(OBJS)
	$(CC) -undefined suppress -flat_namespace $(CFLAGS) -o $@.tgt $^ 
 
%.o: %.cc
	$(CC) $(CFLAGS) -c -o $@ $<

confs: ./generate_tgt_confs
	./generate_tgt_confs $(TGT)

.PHONY: install uninstall run clean 

install:
	$(INSTALL_PROGRAM) ./$(TGT).tgt  "$(EXEC_DIR)/$(TGT).tgt"
	$(INSTALL_DATA)    $(TGT).conf   "$(EXEC_DIR)/$(TGT).conf"
	$(INSTALL_DATA)    $(TGT)-s.conf "$(EXEC_DIR)/$(TGT)-s.conf"

uninstall:
	rm -f $(EXEC_DIR)/$(TGT).tgt
	rm -f $(EXEC_DIR)/$(TGT).conf
	rm -f $(EXEC_DIR)/$(TGT)-s.conf

run:
	$(BASE_IVERILOG_DIR)/bin/iverilog -v -t nemo -s $(TOP_MODULE) -o ../graphs/$(TOP_MODULE).dot $(NETLIST)
	dot ../graphs/$(TOP_MODULE).dot -Tpdf -o ../graphs/$(TOP_MODULE).pdf

runsave:
	$(BASE_IVERILOG_DIR)/bin/iverilog -v -t nemo -s $(TOP_MODULE) -o ../graphs/$(TOP_MODULE).dot $(NETLIST) > output.txt
	dot ../graphs/$(TOP_MODULE).dot -Tpdf -o ../graphs/$(TOP_MODULE).pdf

clean:
	rm -f *.o
	rm -f *.tgt
	rm -f *.conf

cleandot:
	rm -f ../graphs/*.dot
	rm -f ../graphs/*.pdf

cleanall: clean uninstall
