#include "Translator.h"

bool IsControlRegister(REG reg)
{
    return (reg >= REG_CR0 && reg <= REG_CR8);
}

bool IsDebugRegister(REG reg)
{
    return (reg >= REG_DR0 && reg <= REG_DR7);
}

bool IsSegmentRegister(REG reg)
{
    return (reg >= REG_CS && reg <= REG_SS);
}

bool IsXmmRegister(REG reg)
{
    return (reg >= REG_XMM0 && reg <= REG_XMM15);
}

bool IsYmmRegister(REG reg)
{
    return (reg >= REG_YMM0 && reg <= REG_YMM15);
}

REG getregister(const char* text)
{
    // Loop through each entry looking for the register
    for(int i = 0; i < ARRAYSIZE(RegisterIds); i++)
    {
        if(!_stricmp(RegisterIds[i].Name, text))
            return RegisterIds[i].RegId;
    }

    // No register found or it was not implemented
    return REG_INVALID;
}

SEG getsegment(const char* text)
{
    // Loop through each entry looking for the segment register
    for(int i = 0; i < ARRAYSIZE(SegmentIds); i++)
    {
        if(!_stricmp(SegmentIds[i].Name, text))
            return SegmentIds[i].SegId;
    }

    // No segment found or it was not implemented
    return SEG_INVALID;
}

xed_reg_enum_t regtoxed(REG reg)
{
    return RegisterIds[reg].XedId;
}

xed_reg_enum_t segtoxed(SEG seg)
{
    return SegmentIds[seg].XedId;
}

const char* regtostring(REG reg)
{
    return RegisterIds[reg].Name;
}

const char* segtostring(SEG seg)
{
    return SegmentIds[seg].Name;
}

REGSIZE getregsize(REG reg)
{
    return RegisterIds[reg].Size;
}

REGSIZE getsegsize(SEG seg)
{
    return SegmentIds[seg].Size;
}
