module vpcie_header (
        output reg [7:0] op,
        output reg [7:0] bar,
        output reg [7:0] width,
        output reg [63:0] addr,
        output reg [15:0] size,
        output reg [31:0] word_data,
        output reg new_msg
    );
endmodule // vpcie_header

module vpcie_status (
		output reg running,
		output reg connected,
        input wire creditToken
	);

always @ (posedge creditToken) begin
    $vpcieCreditToken();
end

endmodule // vpcie_status
