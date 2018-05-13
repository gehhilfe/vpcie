module vpcie_header (
        input clk_i,
        output reg [7:0] op,
        output reg [7:0] bar,
        output reg [7:0] width,
        output reg [63:0] addr,
        output reg [15:0] size,
        output reg [31:0] word_data,
        output reg new_msg
    );

reg [7:0] data_out;
reg [7:0] recv_payload [0:4096];
integer i;

always @(posedge new_msg) begin
    for (i = 0; i < 4098; i = i + 1) begin
        $getPayloadData(i);
        recv_payload[i] = data_out;
    end
end

always @(posedge clk_i) begin
    if(new_msg) new_msg <= 0;
end 

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
