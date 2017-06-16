BASE_IVERILOG_DIR=/Users/ti27457/Repos/nemo/iverilog
EXEC_DIR=/Users/ti27457/Repos/nemo/iverilog/lib/ivl

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644

IDIR=../iverilog
CC=g++
CFLAGS=-I$(IDIR) -fpic -shared 
# -Wall -Wextra -Wshadow -g -O2
LIBS=-lpthread

TGT=nemo
DOT=up_counter
# DOT=orpsoc_top.generic_low.nl
SRCS=$(wildcard *.cc)
OBJS=$(SRCS:.cc=.o)
# DEPS=ttb.h ttb_signal.h ttb_dot_file.h ivl_target.h

all: $(TGT) confs install

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
	../iverilog/bin/iverilog -t nemo -o $(DOT).dot ../$(DOT).v

clean:
	rm -f *.o
	rm -f *.tgt
	rm -f *.conf
	rm -f $(DOT).dot

cleanall: clean uninstall
