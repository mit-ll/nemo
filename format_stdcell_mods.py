import copy

ifile = open("/home/g89_designs/libraries/MITLL90_STDLIB_8T/2018.5/MITLL90_STDLIB_8T.v", "rb")
ofile = open("MITLL90_STDLIB_8T.vm", "wb")

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
		# if "module" in line and "endmodule" not in line and "`else" in prev_line and not started:
		if "module" in line and "endmodule" not in line and not started:
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
