//-----------------------------------------------------
// Design Name : test
// File Name   : test.v
// Function    : Exercise Nemo Capabilities
// Coder       : Timothy Trippel
//-----------------------------------------------------
module test(
	o1,
	o2,
	i1,
	i2,
);
//----------Output Ports--------------
	output [7:0] o1;
	output [7:0] o2;

//------------Input Ports--------------
	input[3:0] i1;
	input[1:0] i2;

	wire[1:0] internal;
	wire[1:0] i1_part;
//-------------Code Starts Here-------
	assign i1_part  = i1[3:2];
	assign internal = i1_part & i2;
	assign o1 = {internal, 6'b110011};

	test_cell t1(.output_cell(o2), .input_cell({internal, 6'b100111}));	

endmodule