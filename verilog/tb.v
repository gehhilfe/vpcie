module tb();

wire [7:0] op;

vpcie_header vpcie_header_inst (
        .op(op)
);

initial begin
    $setHeader(vpcie_header_inst);
end

endmodule
