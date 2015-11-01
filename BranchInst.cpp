#include "Translator.h"

bool IClassIsJump(xed_iclass_enum_t IClass)
{
    return (IClass >= XED_ICLASS_JB && IClass <= XED_ICLASS_JZ) || (IClass >= XED_ICLASS_LOOP && IClass <= XED_ICLASS_LOOPNE);
}

bool IClassIsCall(xed_iclass_enum_t IClass)
{
    return (IClass == XED_ICLASS_CALL_FAR || IClass == XED_ICLASS_CALL_NEAR);
}

bool IClassIsBranch(xed_iclass_enum_t IClass)
{
    return IClassIsJump(IClass) || IClassIsCall(IClass);
}

int IClassBranchLength(xed_iclass_enum_t IClass, bool Imm8)
{
    switch(IClass)
    {
    case XED_ICLASS_JB:
    case XED_ICLASS_JBE:
    case XED_ICLASS_JL:
    case XED_ICLASS_JLE:
    case XED_ICLASS_JNB:
    case XED_ICLASS_JNBE:
    case XED_ICLASS_JNL:
    case XED_ICLASS_JNLE:
    case XED_ICLASS_JNO:
    case XED_ICLASS_JNP:
    case XED_ICLASS_JNS:
    case XED_ICLASS_JNZ:
    case XED_ICLASS_JO:
    case XED_ICLASS_JP:
    case XED_ICLASS_JS:
    case XED_ICLASS_JZ:
        return (Imm8) ? 2 : 6;

    case XED_ICLASS_LOOP:
    case XED_ICLASS_LOOPE:
    case XED_ICLASS_LOOPNE:
    case XED_ICLASS_JRCXZ:
        return 2;

    case XED_ICLASS_JMP:
        return (Imm8) ? 2 : 5;

    case XED_ICLASS_CALL_NEAR:
        return 5;

    // Both FAR CALL and FAR JMP are 7 bytes long, however
    // displacement doesn't apply
    case XED_ICLASS_JMP_FAR:
    case XED_ICLASS_CALL_FAR:
        return 0;
    }

    return 0;
}

bool TranslateBranchInst(XEDPARSE* Parse, Inst* Instruction)
{
    // Check if it needs to be fixed
    if(!IClassIsBranch(Instruction->Class))
    {
        // Instructions that are not a branch cannot have the 'short' modifier
        if(Instruction->Short)
        {
            strcpy(Parse->error, "Unsupported 'short' modifier for instruction");
            return false;
        }

        return true;
    }

    // Any regular branch instruction can only have one operand max
    if(Instruction->OperandCount > 1)
    {
        // Far jumps are the exception: JMP FAR 0xEA231000, 0x1000
        if(Instruction->OperandCount == 2 && Instruction->Far)
        {
            Instruction->OperandCount           = 2;
            Instruction->Operands[0].Type       = OPERAND_SEGSEL;
            Instruction->Operands[0].Sel.Offset = Instruction->Operands[0].Imm.imm;
            Instruction->Operands[1].Type       = OPERAND_IMM;
            Instruction->Operands[1].Size       = SIZE_WORD;
            return true;
        }

        strcpy(Parse->error, "Too many operands in branch");
        return false;
    }

    // This is only handled if the operand is memory/selector/an immediate
    InstOperand* operand = &Instruction->Operands[0];

    if(operand->Type == OPERAND_IMM)
    {
        LONGLONG delta = TranslateRelativeCip(Parse, operand->Imm.imm, true);

        // XED doesn't automatically do this:
        //
        // Modify the delta so that it accommodates for the instruction size
        //
        // IF NOT CALL
        // and
        // IF DELTA <= 127+ShortJumpLen (for Forward Jumps) && DELTA > -127 (for Backward Jumps) THEN [SHORT JUMP]
        int branchClass = IClassBranchLength(Instruction->Class, (delta <= 127 + 2) && (delta > -127));
        delta           -= branchClass;

        // Short jumps are required to be 2 bytes or less
        if(Instruction->Short && branchClass > 2)
        {
            strcpy(Parse->error, "Unable to use a short jump for displacement");
            return false;
        }

        // Branches can't have a displacement larger than 32 bits
        ULONGLONG masked = delta & 0xFFFFFFFF00000000;

        if(masked != 0 && masked != 0xFFFFFFFF00000000)
        {
            strcpy(Parse->error, "Branch displacement is too large");
            return false;
        }

        // 64bit -> 32bit variable mask
        if(!Parse->x64)
            delta &= 0xFFFFFFFF;

        operand->Size           = branchClass == 2 ? SIZE_BYTE : SIZE_DWORD;
        operand->Imm.Signed     = true;
        operand->Imm.RelBranch  = true;
        operand->Imm.simm       = delta;
    }
    else if(operand->Type == OPERAND_MEM)
    {
        // JMP/CALL FARWORD PTR[]
        if(Instruction->Class == XED_ICLASS_CALL_FAR || Instruction->Class == XED_ICLASS_JMP_FAR)
        {
            if(operand->Size != SIZE_UNSET && operand->Size != SIZE_FWORD)
            {
                strcpy(Parse->error, "Invalid memory type for far branch");
                return false;
            }

            operand->Size = SIZE_FWORD;
        }
    }
    else if(operand->Type == OPERAND_SEGSEL)
    {
        // JMP FAR 0000:00000000
        // This is considered 2 operands by XED
        Instruction->OperandCount           = 2;
        Instruction->Operands[1].Type       = OPERAND_IMM;
        Instruction->Operands[1].Size       = SIZE_WORD;
        Instruction->Operands[1].Imm.imm    = Instruction->Operands[0].Sel.Selector;
    }

    return true;
}