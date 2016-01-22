#include "Translator.h"
#include <stdio.h>

OPSIZE PromoteImmediateWidth(bool Signed, ULONGLONG Value, OPSIZE Width)
{
    // Adjustment fix-up when the sign bit is set
    //
    // half     = pow(2, n_bits - 1);
    // minhalf  = -half;
    xed_uint64_t maxhalf = 1 << (OpsizeToBits(Width) - 1);
    xed_uint64_t minhalf = 1 + (~maxhalf);

    // Values that are <  -(MAX_VALUE/2) need to use the next biggest size
    // Values that are >= +(MAX_VALUE/2) need to use the next biggest size
    if((Signed && Value < minhalf) || (!Signed && Value >= maxhalf))
    {
        switch(Width)
        {
        case SIZE_BYTE:
            return SIZE_DWORD;
        case SIZE_DWORD:
            return SIZE_QWORD;
        }
    }

    return Width;
}

bool AnalyzeOperand(XEDPARSE* Parse, const char* Value, InstOperand* Operand)
{
    REG registerVal  = RegFromString(Value);
    ULONGLONG immVal = 0;

    if(registerVal != REG_INVALID)
    {
        // Register
        Operand->Type       = OPERAND_REG;
        Operand->Segment    = REG_INVALID;
        Operand->Size       = RegGetSize(registerVal);
        Operand->XedEOSZ    = OpsizeToEosz(Operand->Size);
        Operand->Reg.Reg    = registerVal;
        Operand->Reg.XedReg = RegToXed(registerVal);
    }
    else if(strchr(Value, '[') && strchr(Value, ']'))
    {
        // Memory
        Operand->Type       = OPERAND_MEM;
        Operand->Segment    = REG_INVALID;
        Operand->Size       = SIZE_UNSET;
        Operand->XedEOSZ    = EOSZ_64_32(Parse->x64);

        return HandleMemoryOperand(Parse, Value, Operand);
    }
    else if(strchr(Value, ':'))
    {
        // Segment selector
        Operand->Type       = OPERAND_SEGSEL;
        Operand->Segment    = REG_INVALID;
        Operand->Size       = SIZE_DWORD;
        Operand->XedEOSZ    = EOSZ_64_32(Parse->x64);

        return HandleSegSelectorOperand(Parse, Value, Operand);
    }
    else if(valfromstring(Value, &immVal) || (Parse->cbUnknown && Parse->cbUnknown(Value, &immVal)))
    {
        // Immediate
        Operand->Type       = OPERAND_IMM;
        Operand->Segment    = REG_INVALID;
        Operand->Size       = OpsizeFromValue(immVal);
        Operand->XedEOSZ    = EOSZ_64_32(Parse->x64);
        Operand->Imm.Signed = (Value[0] == '-');
        Operand->Imm.imm    = immVal;
    }
    else
    {
        // Unknown
        Operand->Type = OPERAND_INVALID;

        sprintf(Parse->error, "Unknown operand identifier '%s'", Value);
        return false;
    }

    return true;
}