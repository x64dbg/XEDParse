#include "Translator.h"

const char* MnemonicToXed(const char* Mnemonic)
{
    for(int i = 0; i < ARRAYSIZE(XedMnemonicTable); i++)
    {
        if(!_stricmp(Mnemonic, XedMnemonicTable[i].Name))
            return XedMnemonicTable[i].XedName;
    }

    return Mnemonic;
}

char* InstMnemonicToXed(XEDPARSE* Parse, Inst* Instruction)
{
    char* mnemonic = Instruction->Mnemonic;

    /*
    TODO:

    BEXTR_XOP
    PEXTRW_SSE4
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

            if(!_stricmp(mnemonic, "cmpsd"))
            {
                if(IsXmmRegister(operand->Reg.Reg))
                    strcpy(mnemonic, "cmpsd_xmm");
            }

            if(!_stricmp(mnemonic, "movsd"))
            {
                if(IsXmmRegister(operand->Reg.Reg))
                    strcpy(mnemonic, "movsd_xmm");
            }
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

    if(!_stricmp(Instruction->Mnemonic, "pushf") || !_stricmp(Instruction->Mnemonic, "popf"))
    {
        if(Parse->x64)
            strcat(Instruction->Mnemonic, "q");
        else
            strcat(Instruction->Mnemonic, "d");
    }

    // Hidden/non-explicit operands (Ex: (MOVS/CMPS)(B/W/D/Q) case)
    // Exclude instructions with XXXXX_XMM
    if(!strstr(mnemonic, "xmm"))
    {
        InstMnemonicExplicitFix(Instruction, "movs", "mov");
        InstMnemonicExplicitFix(Instruction, "cmps", "cmp");
        InstMnemonicExplicitFix(Instruction, "scas", "sca");
        InstMnemonicExplicitFix(Instruction, "stos", "sto");
        InstMnemonicExplicitFix(Instruction, "lods", "lod");
        InstMnemonicExplicitFix(Instruction, "outs", "out");
    }

    // Convert the name to XED format
    strcpy(mnemonic, MnemonicToXed(mnemonic));

    return _strupr(mnemonic);
}

void InstMnemonicExplicitFix(Inst* Instruction, const char* Base, const char* Normal)
{
    char* mnemonic  = Instruction->Mnemonic;
    size_t len      = strlen(Base);

    if(_strnicmp(mnemonic, Base, len))
        return;

    switch(mnemonic[len])
    {
    case 'b':
    case 'w':
    case 'd':
    case 'q':
    case 'B':
    case 'W':
    case 'D':
    case 'Q':
        Instruction->OperandCount = 0;
        break;
    }

    // Case
    if(mnemonic[len] == '\0')
    {
        // Default to the "normal" instruction
        strcpy(mnemonic, Normal);

        // See if there's any explicit operands to convert
        if(Instruction->OperandCount > 0)
        {
            InstOperand* operands = Instruction->Operands;

            if(operands[0].Type == OPERAND_MEM && operands[1].Type == OPERAND_MEM)
            {
                if((operands[0].Mem.BaseVal == REG_RDI && operands[1].Mem.BaseVal == REG_RSI) ||
                        (operands[0].Mem.BaseVal == REG_EDI && operands[1].Mem.BaseVal == REG_ESI) ||
                        (operands[0].Mem.BaseVal == REG_RSI && operands[1].Mem.BaseVal == REG_RDI) ||
                        (operands[0].Mem.BaseVal == REG_ESI && operands[1].Mem.BaseVal == REG_EDI))
                {
                    // Apply the base
                    strcpy(mnemonic, Base);

                    switch(operands[0].Size)
                    {
                    case SIZE_BYTE:
                        strcat(mnemonic, "b");
                        break;
                    case SIZE_WORD:
                        strcat(mnemonic, "w");
                        break;
                    case SIZE_DWORD:
                        strcat(mnemonic, "d");
                        break;
                    case SIZE_QWORD:
                        strcat(mnemonic, "q");
                        break;
                    }

                    Instruction->OperandCount = 0;
                }
            }
        }
    }
}
