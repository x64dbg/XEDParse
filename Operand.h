#pragma once

enum OPERAND_TYPE
{
    OPERAND_INVALID,
    OPERAND_REG,
    OPERAND_IMM,
    // OPERAND_IMM1,
    OPERAND_MEM,
    OPERAND_SEGSEL,
};

struct InstOperand
{
    OPERAND_TYPE    Type;
    SEG             Segment;
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
            bool Disp;
            xed_uint64_t DispVal;
            MEMSIZE DispWidth;
            bool DispRipRelative;

            bool Scale;
            xed_uint64_t ScaleVal;

            bool Base;
            REG BaseVal;

            bool Index;
            REG IndexVal;
        } Mem;

        struct
        {
            USHORT Selector;
            xed_int_t Offset;
        } Sel;
    };
};

OPSIZE PromoteImmediateWidth(bool Signed, ULONGLONG Value, OPSIZE Width);
bool AnalyzeOperand(XEDPARSE* Parse, const char* Value, InstOperand* Operand);
