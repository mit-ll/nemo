# -----------------------------------------------------------------------------
# File:        Makefile
# Author:      Timothy Trippel
# Affiliation: MIT Lincoln Laboratory
# Description:

# This is Makefile to build a custom backend target module for the Icarus 
# Verilog compiler frontend.
# -----------------------------------------------------------------------------

BASE_IVERILOG_DIR=../iverilog
IVL=${BASE_IVERILOG_DIR}/bin/iverilog
IVL_TGT_DIR=${BASE_IVERILOG_DIR}/lib/ivl

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644

IDIR=${BASE_IVERILOG_DIR}
CC=g++
CFLAGS=-I$(IDIR) -fpic -shared -std=gnu++0x -Wall -Wextra -Wshadow -g -O2

TGT=nemo
SRCS=$(wildcard *.cc)
OBJS=$(SRCS:.cc=.o)

TOP_MODULE=test

NETLIST=../netlists/test.v ../netlists/test_std_cell_defs.vm

all: $(TGT) confs $(IVL) install

$(TGT): $(OBJS)
	$(CC) -flat_namespace $(CFLAGS) -o $@.tgt $^ 
 
%.o: %.cc
	$(CC) $(CFLAGS) -c -o $@ $<

confs: ./generate_tgt_confs
	./generate_tgt_confs $(TGT)

$(IVL):
	pushd $(BASE_IVERILOG_DIR)
	sh autoconf.sh
	./configure --prefix=$(pwd)
	make install
	popd

.PHONY: install uninstall test testsave clean cleandot cleanall

install:
	$(INSTALL_PROGRAM) ./$(TGT).tgt  "$(IVL_TGT_DIR)/$(TGT).tgt"
	$(INSTALL_DATA)    $(TGT).conf   "$(IVL_TGT_DIR)/$(TGT).conf"
	$(INSTALL_DATA)    $(TGT)-s.conf "$(IVL_TGT_DIR)/$(TGT)-s.conf"

uninstall:
	rm -f $(IVL_TGT_DIR)/$(TGT).tgt
	rm -f $(IVL_TGT_DIR)/$(TGT).conf
	rm -f $(IVL_TGT_DIR)/$(TGT)-s.conf

test:
	$(IVL) -v -t nemo -s $(TOP_MODULE) -o ../graphs/$(TOP_MODULE).dot $(NETLIST)
	# dot ../graphs/$(TOP_MODULE).dot -Tpdf -o ../graphs/$(TOP_MODULE).pdf

testsave:
	$(IVL) -v -t nemo -s $(TOP_MODULE) -o ../graphs/$(TOP_MODULE).dot $(NETLIST) > output.txt
	# dot ../graphs/$(TOP_MODULE).dot -Tpdf -o ../graphs/$(TOP_MODULE).pdf

cleanall: clean uninstall

clean:
	rm -f *.o
	rm -f *.tgt
	rm -f *.conf

cleandot:
	rm -f ../graphs/*.dot
	rm -f ../graphs/*.pdf
