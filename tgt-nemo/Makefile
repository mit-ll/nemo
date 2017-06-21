BASE_IVERILOG_DIR=/Users/ti27457/Repos/nemo/iverilog
EXEC_DIR=/Users/ti27457/Repos/nemo/iverilog/lib/ivl

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644

IDIR=../iverilog
CC=g++
CFLAGS=-I$(IDIR) -fpic -shared
# -Wall -Wextra -Wshadow -g -O2
LIBS=-lprotobuf

TGT=nemo
# DOT=up_counter.dot
DOT=orpsoc_top.express50MHz.nl.dot
# NETLIST=../up_counter.v
NETLIST=../orpsoc_top.express50MHz.nl.v /Volumes/ttrippel/A2/cp65npksdst.vm
SRCS=$(wildcard *.cc)
OBJS=$(SRCS:.cc=.o)
PROTO=$(wildcard protobufs/*.proto)
# DEPS=ttb.h ttb_signal.h ttb_dot_file.h ivl_target.h

all: proto $(TGT) confs install

proto:
	protoc -I=protobufs/ --cpp_out=. $(PROTO)

$(TGT): $(OBJS)
	$(CC) -undefined suppress -flat_namespace $(CFLAGS) $(LIBS) -o $@.tgt $^ 
 
%.o: %.cc
	$(CC) $(CFLAGS) $(LIBS) -c -o $@ $<

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
	../iverilog/bin/iverilog -t nemo -o $(DOT) $(NETLIST)

clean:
	rm -f *.o
	rm -f *.tgt
	rm -f *.conf
	rm -f $(DOT).dot

cleanproto:
	rm -f *.pb.cc
	rm -f *.pb.h
	rm -f *.pb.bin

cleanall: clean cleanproto uninstall
