module sv_bram #(
	parameter p_addr_bits = 14,
	parameter p_read_latency = 2
	)(
	input clk, en, rst, wren,
	input logic [p_addr_bits-1:0] addr,
	input logic [31:0] d_in,
	output logic [31:0] d_out);



reg [31:0] data [0:2**p_addr_bits];

initial begin
	foreach(data[j]) begin
		data[j] = 0;
	end
end

task automatic read;
	input [p_addr_bits-1:0] in_addr;
	//repeat(p_read_latency) @posedge(clk)
	d_out = data[in_addr];
endtask

task automatic write;
	input [p_addr_bits-1:0] in_addr;
	data[in_addr] = d_in;
endtask

always @(posedge clk) begin
	if (rst) begin
		d_out = 0;
	end
	else begin
		if(en && !wren) begin
			fork
				read(addr);
			join_none
		end
		if(en && wren) begin
			fork
				write(addr);
			join_none
		end
	end
end


endmodule