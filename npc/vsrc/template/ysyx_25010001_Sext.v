module ysyx_25010001_Sext #(
    IWIDTH = 12,
    OWIDTH = 64
) (
    input  [IWIDTH - 1 : 0] data_i,
    output [OWIDTH - 1 : 0] data_o
);
    assign data_o = {{(OWIDTH - IWIDTH){data_i[IWIDTH - 1 : IWIDTH - 1]}}, data_i};
endmodule
