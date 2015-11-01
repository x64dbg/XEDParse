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
    return (reg >= REG_YMM0 && reg <= REG_YMM31);
}

bool IsZmmRegister(REG reg)
{
    return (reg >= REG_ZMM0 && reg <= REG_ZMM31);
}

REG RegFromString(const char* text)
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

xed_reg_enum_t RegToXed(REG reg)
{
    return RegisterIds[reg].XedId;
}

const char* RegToString(REG reg)
{
    return RegisterIds[reg].Name;
}

REGSIZE RegGetSize(REG reg)
{
    return RegisterIds[reg].Size;
}