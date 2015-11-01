#include "Translator.h"

const char* MnemonicToXed(char* Mnemonic)
{
    // Translate known aliases for certain instructions
    for(int i = 0; i < ARRAYSIZE(XedMnemonicTable); i++)
    {
        if(_stricmp(Mnemonic, XedMnemonicTable[i].Name) != 0)
            continue;

        // Found an alias, copy the real mnemonic now
        strcpy(Mnemonic, XedMnemonicTable[i].XedName);
        break;
    }

    // Force uppercase, which is usually required by Xed
    return _strupr(Mnemonic);
}

const char* MnemonicInstToXed(XEDPARSE* Parse, Inst* Instruction)
{
    char* mnemonic = Instruction->Mnemonic;

    /*
    TODO:

    BEXTR_XOP
    PREFETCH_EXCLUSIVE
    PREFETCH_RESERVED
    */

    // Special cases depending on the operands
    for(int i = 0; i < Instruction->OperandCount; i++)
    {
        InstOperand* operand = &Instruction->Operands[i];

        if(operand->Type == OPERAND_REG)
        {
            if(!_stricmp(mnemonic, "mov"))
            {
                if(IsControlRegister(operand->Reg.Reg))
                    strcpy(mnemonic, "mov_cr");
                else if(IsDebugRegister(operand->Reg.Reg))
                    strcpy(mnemonic, "mov_dr");
            }

            MnemonicTranslateXmmAlias(mnemonic, operand->Reg.Reg);
        }
    }

    // Far addressing
    if(Instruction->Far)
    {
        if(!_stricmp(Instruction->Mnemonic, "call"))
            strcpy(Instruction->Mnemonic, "call_far");
        else if(!_stricmp(Instruction->Mnemonic, "jmp"))
            strcpy(Instruction->Mnemonic, "jmp_far");
        else if(!_stricmp(Instruction->Mnemonic, "ret"))
            strcpy(Instruction->Mnemonic, "ret_far");
    }

    // 32-bit and 64-bit save flags handling
    if(!_stricmp(Instruction->Mnemonic, "pushf") || !_stricmp(Instruction->Mnemonic, "popf"))
    {
        if(Parse->x64)
            strcat(Instruction->Mnemonic, "q");
        else
            strcat(Instruction->Mnemonic, "d");
    }

    // Implicit aliases
    MnemonicTranslateImplicitAlias(mnemonic, Instruction);

    // Done, onvert the name to Xed format
    return MnemonicToXed(mnemonic);
}

void MnemonicTranslateXmmAlias(char* Mnemonic, REG RegisterType)
{
    // Handles instructions where there is a regular GP register, or
    // where an XMM register can be used
    if(IsXmmRegister(RegisterType))
    {
        if(!_stricmp(Mnemonic, "cmpsd"))            // CMPSD
            strcpy(Mnemonic, "cmpsd_xmm");
        else if(!_stricmp(Mnemonic, "movsd"))       // MOVSD
            strcpy(Mnemonic, "movsd_xmm");
        else if(!_stricmp(Mnemonic, "pextrw"))      // PEXTRW
            strcpy(Mnemonic, "pextrw_sse4");
    }
}

void MnemonicTranslateImplicitAlias(char* Mnemonic, Inst* Instruction)
{
    // Avoid naming conflicts (SSE "_XMM", MOVSX*)
    if(strchr(Mnemonic, '_') ||
            strchr(Mnemonic, 'X') ||
            strchr(Mnemonic, 'x'))
        return;

    // Certain string instructions need to override the instruction size
    if(!_strnicmp(Mnemonic, "ins", 3) ||
            !_strnicmp(Mnemonic, "outs", 4) ||
            !_strnicmp(Mnemonic, "movs", 4) ||
            !_strnicmp(Mnemonic, "cmps", 4) ||
            !_strnicmp(Mnemonic, "stos", 4) ||
            !_strnicmp(Mnemonic, "lods", 4) ||
            !_strnicmp(Mnemonic, "scas", 4))
    {
        if(Instruction->OperandCount >= 2)
        {
            // Determine size from the MEMORY operand REGISTER
            auto operands = Instruction->Operands;

            if(operands[0].Type == OPERAND_MEM)
                Instruction->AddressSizeOverride = OpsizeToBits(RegGetSize(operands[0].Mem.BaseVal));
            else if(operands[1].Type == OPERAND_MEM)
                Instruction->AddressSizeOverride = OpsizeToBits(RegGetSize(operands[1].Mem.BaseVal));
        }

        // All operands are implicit/hidden
        Instruction->OperandCount = 0;
    }
}