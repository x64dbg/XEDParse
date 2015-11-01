#include "ParseTest.h"
#include "Register.h"
#include <stdio.h>

void OperandToString(char* Buffer, InstOperand* Operand)
{
    switch(Operand->Type)
    {
    case OPERAND_INVALID:
        strcpy(Buffer, "(INVALID OPERAND)");
        break;

    case OPERAND_REG:
        strcpy(Buffer, RegToString(Operand->Reg.Reg));
        break;

    case OPERAND_IMM:
        if(Operand->Imm.Signed && Operand->Imm.simm < 0)
            sprintf(Buffer, "-0x%llX", (~Operand->Imm.imm) + 1);
        else
            sprintf(Buffer, "0x%llX", Operand->Imm.imm);
        break;

    case OPERAND_MEM:
    {
        char base[64];
        char scale[64];

        memset(base, 0, sizeof(base));
        memset(scale, 0, sizeof(scale));

        if(Operand->Mem.Base)
            sprintf(base, "%s + ", RegToString(Operand->Mem.BaseVal));

        if(Operand->Mem.Index)
            sprintf(scale, "%s * %d + ", RegToString(Operand->Mem.IndexVal), Operand->Mem.ScaleVal);

        sprintf(Buffer, "%s ptr %s:[%s%s0x%llX/%d]",
                OpsizeToString(Operand->Size),
                RegToString(Operand->Segment),
                base,
                scale,
                Operand->Mem.DispVal,
                OpsizeToBits(Operand->Mem.DispWidth));
    }
    break;

    case OPERAND_SEGSEL:
        sprintf(Buffer, "0x%llX", Operand->Sel.Offset);
        break;
    }
}

void InstructionToString(char* Buffer, Inst* Instruction)
{
    // Add the prefix if there was one
    if(Instruction->Prefix != PREFIX_NONE)
    {
        strcat(Buffer, PrefixToString(Instruction->Prefix));
        strcat(Buffer, " ");
    }

    // Append mnemonic
    strcat(Buffer, Instruction->Mnemonic);

    // Append all operands
    if(Instruction->OperandCount > 0)
    {
        strcat(Buffer, " ");

        for(int i = 0; i < Instruction->OperandCount; i++)
        {
            char op[XEDPARSE_MAXBUFSIZE];
            OperandToString(op, &Instruction->Operands[i]);

            strcat(Buffer, op);
            strcat(Buffer, ", ");
        }

        // Remove the final trailing comma
        *strrchr(Buffer, ',') = '\0';
    }
}
