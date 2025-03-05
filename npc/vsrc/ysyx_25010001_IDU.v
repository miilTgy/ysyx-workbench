// `include "ysyx_25010001_Mux.v";
module ysyx_25010001_IDU #(
    PC_WIDTH     = 64,
    INST_WIDTH   = 32,
    DATA_WIDTH   = 64,
    ALUOP_WIDTH = 4
) (
    input   [INST_WIDTH - 1 : 0]    inst,
    input   [PC_WIDTH   - 1 : 0]    pc,
    /* Register outputs */
    output  [REG_WIDTH  - 1 : 0]    src1,
    output  [REG_WIDTH  - 1 : 0]    src2,
    output  [REG_WIDTH  - 1 : 0]    rd,
    /* Immediate outputs */
    output  [DATA_WIDTH - 1 : 0]    imm,
    /* Control outputs */
    output                          alud2slct,
    output  [ALUOP_WIDTH-1 : 0]    aluop,
    /* DBG outputs */
    output  [OP_WIDTH   - 1 : 0]    opcode_dbg,
    output                          type_I_dbg
);
// FIRST DECODER
    /* Inst seperation */
    localparam REG_WIDTH    = 5;
    localparam OP_WIDTH     = 7;
    localparam FUNCT3_WIDTH = 3;
    localparam FUNCT7_WIDTH = 7;

    wire [REG_WIDTH - 1  : 0]  src1_stage1  ; assign src1_stage1   = inst[19 : 15];
    wire [REG_WIDTH - 1  : 0]  src2_stage1  ; assign src2_stage1   = inst[24 : 20];
    wire [REG_WIDTH - 1  : 0]  rd_stage1    ; assign rd_stage1     = inst[11 : 7];
    wire [OP_WIDTH  - 1  : 0]  opcode_stage1; assign opcode_stage1 = inst[6  : 0];
    wire [FUNCT3_WIDTH-1 : 0]  funct3_stage1; assign funct3_stage1 = inst[14 : 12];
    wire [FUNCT7_WIDTH-1 : 0]  funct7_stage1; assign funct7_stage1 = inst[31 : 25];

    wire [DATA_WIDTH -1  : 0]  immI_stage1;
    wire [DATA_WIDTH -1  : 0]  immU_stage1;
    wire [DATA_WIDTH -1  : 0]  immS_stage1;
    wire [DATA_WIDTH -1  : 0]  immB_stage1;
    wire [DATA_WIDTH -1  : 0]  immJ_stage1;

    assign immI_stage1 = {{52{inst[31]}}, inst[31:20]};
    assign immU_stage1 = {{32{inst[31]}}, inst[31:12], 12'b0};
    assign immS_stage1 = {{52{inst[31]}}, inst[31:25], inst[11:7]};
    assign immB_stage1 = {{52{inst[31]}}, inst[7], inst[30:25], inst[11:8], 1'b0};
    assign immJ_stage1 = {{44{inst[31]}}, inst[19:12], inst[20], inst[30:21], 1'b0};

    /* Define Encoding Type */
    reg typeI;
    always @(*) begin
        case (opcode_stage1[OP_WIDTH-1:2])
            5'b00000: typeI = 1'b1;
            5'b00100: typeI = 1'b1;
            5'b00110: typeI = 1'b1;
            5'b11001: typeI = 1'b1;
            default : typeI = 1'b0;
        endcase        
    end

    /* Immediate Decoder */
    reg  [DATA_WIDTH - 1 : 0]   imm_stage1;

    always @(*) begin
        case (1'b1)
            typeI  : imm_stage1 = immI_stage1;
            default: imm_stage1 = {(DATA_WIDTH){1'b0}};
        endcase
    end

    /* Control signal Generator */
    /* ALU Control */
    reg [ALUOP_WIDTH-1:0] aluop_stage1;
    always @(*) begin
        casez ({opcode_stage1, funct3_stage1, funct7_stage1})
            17'b0010011_000_??????? : aluop_stage1 = 4'b0000;
            default               : aluop_stage1 = 4'b1111;
        endcase
    end

// DECODER OUTPUT
    /* Register outputs */
    assign src1 = src1_stage1;
    assign src2 = src2_stage1;
    assign rd   = rd_stage1  ;
    /* Immediate outputs */
    assign imm  = imm_stage1 ;
    /* Control outputs */
    assign alud2slct = typeI;
    assign aluop     = aluop_stage1;

    /* DBG signals */
    assign opcode_dbg = opcode_stage1;
    assign type_I_dbg = typeI;

endmodule
