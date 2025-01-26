// `include "ysyx_25010001_Mux.v";
module ysyx_25010001_IDU #(
    PC_WIDTH     = 64,
    INST_WIDTH   = 32,
    DATA_WIDTH   = 64,
    ALU_OP_WIDTH = 4
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
    output  [ALU_OP_WIDTH-1 : 0]    aluop,
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
    wire [IMM_I_WIDTH -1 : 0]  immI_stage1  ; assign immI_stage1   = inst[31 : 20];

    /* Define Encoding Type */
    wire typeI;
    ysyx_25010001_MuxKeyWithDefault #(4, 5, 1) muxI (
        typeI, opcode_stage1[OP_WIDTH-1:2], 1'b0, {
        5'b00000, 1'b1,
        5'b00100, 1'b1,
        5'b00110, 1'b1,
        5'b11001, 1'b1
    });

    /* Immediate Decoder */
    localparam IMM_I_WIDTH = 12;

    wire [DATA_WIDTH     - 1 : 0]   imm_stage1;

    wire [DATA_WIDTH     - 1 : 0]   immI_stage1_sext;
        ysyx_25010001_Sext immI_sext(immI_stage1, immI_stage1_sext);
    

    ysyx_25010001_MuxKeyWithDefault #(1, 1, DATA_WIDTH) mux_imm (
        imm_stage1, 1'b1, {(DATA_WIDTH){1'b0}}, {
        typeI, immI_stage1_sext
    });

    /* Control signal Generator */
    

// DECODER OUTPUT
    /* Register outputs */
    assign src1 = src1_stage1;
    assign src2 = src2_stage1;
    assign rd   = rd_stage1  ;
    /* Immediate outputs */
    assign imm  = imm_stage1 ;
    /* Control outputs */
    assign alud2slct = typeI;

    /* DBG signals */
    assign opcode_dbg = opcode_stage1;
    assign type_I_dbg = typeI;

endmodule
