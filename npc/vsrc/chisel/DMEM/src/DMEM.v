module DMEM #(
    ADDR_WIDTH = 64,
    DATA_WIDTH = 64
) (
    input   [ADDR_WIDTH-1:0]    addr,
    input   [DATA_WIDTH-1:0]    wdata,
    output  [DATA_WIDTH-1:0]    rdata
);
    import "DPI-C" function int paddr_read(input longint unsigned paddr);
    assign data = paddr_read(addr);
endmodule
