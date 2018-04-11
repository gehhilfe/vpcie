module vpcie_header (
        output reg [7:0] op
    );
endmodule // vpcie_header

module vpcie_status (
		output reg running,
		output reg connected
	);
endmodule // vpcie_status