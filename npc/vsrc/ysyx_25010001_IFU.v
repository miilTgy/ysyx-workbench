module ysyx_25010001_IFU #(
        PC_WIDTH = 64,
        PC_RESET = 64'h80000000
    ) (
        input clk,
        input rst,
        output [PC_WIDTH-1:0] maddr
    );
    wire [PC_WIDTH-1:0] thispc;
    assign maddr = thispc;
    
    wire [PC_WIDTH-1:0] nextpc;
    assign nextpc = thispc + 64'h4;

    ysyx_25010001_Reg #(
        .WIDTH      (64             ),
        .RESET_VAL  (64'h80000000   )
    ) pc (
        .clk    (clk    ),
        .rst    (rst    ),
        .din    (nextpc ),
        .dout   (thispc ),
        .wen    (1'b1   )
    );
endmodule
