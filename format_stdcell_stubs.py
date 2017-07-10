ifile = open("/Volumes/ttrippel/A2/cp65npksdst.vm", "rb")
ofile = open("std_cell_defs.vm", "wb")

for line in ifile:
	if "module" in line and "endmodule" not in line:
		ofile.write("`celldefine\n")
		ofile.write(line)
	elif "endmodule" in line:
		ofile.write(line)
		ofile.write("`endcelldefine\n")
	else:
		ofile.write(line)

ifile.close()
ofile.close()