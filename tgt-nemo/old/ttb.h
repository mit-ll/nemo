#ifndef __TTB_HEADER__
#define __TTB_HEADER__
/*
 * TTB Backend
 *
 * This is a backend for Icarus verilog which generates a dot file with a
 * dependancy graph showing which signals are dependant on which signals
 *
 * Currently the dot file format is not supported. Instead, I output our own
 * format.
 */
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "nemo_design.h"
#include <ivl_target.h>

using namespace std;

#endif
