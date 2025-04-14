module DMEM #(
    ADDR_WIDTH = 64,
    DATA_WIDTH = 64,
    MASK_WIDTH = 8
) (
    input                       men,
    input                       mwen,
    input   [MASK_WIDTH-1:0]    wmask,
    input   [ADDR_WIDTH-1:0]    waddr,
    input   [ADDR_WIDTH-1:0]    raddr,
    input   [DATA_WIDTH-1:0]    wdata,
    output  [DATA_WIDTH-1:0]    rdata
);
    import "DPI-C" function int unsigned pmem_read_low(input longint unsigned raddr);
    import "DPI-C" function int unsigned pmem_read_high(input longint unsigned raddr);
    import "DPI-C" function void pmem_write(input longint unsigned waddr, input int unsigned wdata_low, input int unsigned wdata_high, input byte unsigned wmask);

    reg [64-1:0] _rdata;
    always @(*) begin
        if (men) begin // 有读写请求时
            _rdata = {pmem_read_high(raddr + 4), pmem_read_low(raddr)};
            if (mwen) begin // 有写请求时
                pmem_write(waddr, wdata[31:0], wdata[63:32], wmask);
            end
        end
        else begin
            _rdata = 0;
        end
    end
    assign rdata = _rdata;
endmodule
