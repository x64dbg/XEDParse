#pragma once

enum OPERAND_TYPE
{
    OPERAND_INVALID,    // Bad operand
    OPERAND_REG,        // Register
    OPERAND_IMM,        // Immediate
    // OPERAND_IMM1,    // Immediate #2 (See ENTER instruction)
    OPERAND_MEM,        // Memory
    OPERAND_SEGSEL,     // Segment selector (Immediate)
};

struct InstOperand
{
    OPERAND_TYPE    Type;
    REG             Segment;
    MEMSIZE         Size;
    int             BitSize;
    int             XedEOSZ;

    union
    {
        struct
        {
            REG             Reg;
            xed_reg_enum_t  XedReg;
        } Reg;

        struct
        {
            bool Signed;
            bool RelBranch;

            union
            {
                xed_uint64_t    imm;
                xed_int64_t     simm;
            };
        } Imm;

        struct
        {
            bool            Disp;
            xed_uint64_t    DispVal;
            MEMSIZE         DispWidth;

            bool            Scale;
            xed_uint64_t    ScaleVal;

            bool            Base;
            REG             BaseVal;

            bool            Index;
            REG             IndexVal;
        } Mem;

        struct
        {
            USHORT      Selector;
            xed_int_t   Offset;
        } Sel;
    };
};

OPSIZE PromoteImmediateWidth(bool Signed, ULONGLONG Value, OPSIZE Width);
bool AnalyzeOperand(XEDPARSE* Parse, const char* Value, InstOperand* Operand);
