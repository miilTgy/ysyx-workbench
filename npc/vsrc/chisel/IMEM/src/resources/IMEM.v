module ysyx_25010001_MEM #(
    ADDR_WIDTH = 64,
    INST_WIDTH = 32
) (
    input   [ADDR_WIDTH-1:0]    im_raddr,
    output  [INST_WIDTH-1:0]    inst_data
    
);
    import "DPI-C" function int paddr_read(input longint unsigned paddr);
    assign inst_data = paddr_read(im_raddr);
endmodule
