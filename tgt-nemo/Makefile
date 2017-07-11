BASE_IVERILOG_DIR=/Users/ti27457/Repos/nemo/iverilog
EXEC_DIR=/Users/ti27457/Repos/nemo/iverilog/lib/ivl

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644

IDIR=../iverilog
CC=g++
CFLAGS=-I$(IDIR) -fpic
# -Wall -Wextra -Wshadow -g -O2
LIBS=-lprotobuf
TGT=nemo
SRCS=$(wildcard *.cc)
OBJS=$(SRCS:.cc=.o)
# DEPS=ttb.h ttb_signal.h ttb_dot_file.h ivl_target.h

# TOP_MODULE=up_counter
# TOP_MODULE=test
TOP_MODULE=orpsoc_top

# DOT=up_counter
# DOT=test
DOT=orpsoc_top.express_100MHz.debug

# NETLIST=../up_counter.nl.v ../std_cell_defs.vm
# NETLIST=../test.v
NETLIST=../orpsoc_top.express_100MHz.nl.v ../std_cell_defs.vm


all: $(TGT) confs install

$(TGT): $(OBJS)
	$(CC) -undefined suppress -flat_namespace $(CFLAGS) $(LIBS) -o $@.tgt $^ 
 
%.o: %.cc
	$(CC) $(CFLAGS) -c -o $@ $<

confs: ./generate_tgt_confs
	./generate_tgt_confs $(TGT)

.PHONY: install uninstall run clean 

install:
	$(INSTALL_PROGRAM) ./$(TGT).tgt "$(EXEC_DIR)/$(TGT).tgt"
	$(INSTALL_DATA) $(TGT).conf "$(EXEC_DIR)/$(TGT).conf"
	$(INSTALL_DATA) $(TGT)-s.conf "$(EXEC_DIR)/$(TGT)-s.conf"

uninstall:
	rm -f $(EXEC_DIR)/$(TGT).tgt
	rm -f $(EXEC_DIR)/$(TGT).conf
	rm -f $(EXEC_DIR)/$(TGT)-s.conf

run:
	../iverilog/bin/iverilog -v -gspecify -t nemo -s $(TOP_MODULE) -o $(DOT).dot $(NETLIST)
	dot $(DOT).dot -Tpdf -o $(DOT).pdf

runsave:
	../iverilog/bin/iverilog -v -gspecify -t nemo -s $(TOP_MODULE) -o $(DOT).dot $(NETLIST) > test.txt
	dot $(DOT).dot -Tpdf -o $(DOT).pdf

clean:
	rm -f *.o
	rm -f *.tgt
	rm -f *.conf

cleanall: clean uninstall
