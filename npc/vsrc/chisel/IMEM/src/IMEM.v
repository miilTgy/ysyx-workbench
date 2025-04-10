module IMEM #(
    ADDR_WIDTH = 64,
    INST_WIDTH = 32
) (
    input   [ADDR_WIDTH-1:0]    pc,
    output  [INST_WIDTH-1:0]    inst_data
    
);
    import "DPI-C" function int pimem_read(input longint unsigned paddr);
    assign inst_data = pimem_read(pc);
endmodule
