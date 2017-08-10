import copy

ifile = open("/Volumes/ttrippel/A2/io_gppr_soi12s0_t18_mv10_mv18_avt_pl.v", "rb")
ofile = open("io_cell_defs_45nm.vm", "wb")

started   = False
mod_paren = False
prev_line = None
for line in ifile:
	if mod_paren:
		ofile.write(line)
		if ");" in line:
			ofile.write("\n")
			mod_paren = False
	else:
		if "module" in line and "endmodule" not in line and "`else" in prev_line and not started:
			ofile.write("`celldefine\n")
			ofile.write(line)
			started = True
			if ");" not in line:
				mod_paren = True
		elif "endmodule" in line and started:
			ofile.write(line)
			ofile.write("`endcelldefine\n\n")
			started = False
		elif "inout" in line and started:
			ofile.write(line)
		elif "input" in line and started:
			ofile.write(line)
		elif "output" in line and started:
			ofile.write(line)
	prev_line = copy.deepcopy(line)

ifile.close()
ofile.close()