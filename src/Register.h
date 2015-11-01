#pragma once

extern "C"
{
#include "..\xed2\include\xed-interface.h"
}

struct RegEntry
{
    const char*     Name;
    enum REG        RegId;
    xed_reg_enum_t  XedId;
    REGSIZE         Size;
};

// BEGIN AUTO PARSING
// Have the compiler parse each enumeration index
#define DEF_REGISTER(Name, Type, XedType, Size) Type,
enum REG
{
#include "RegisterTable.h"
};
#undef DEF_REGISTER

// Have the compiler add each register into the array
#define DEF_REGISTER(Name, Type, XedType, Size) {Name, Type, XedType, Size},
static RegEntry RegisterIds[] =
{
#include "RegisterTable.h"
};
#undef DEF_REGISTER
// END AUTO PARSING

bool IsControlRegister(REG reg);
bool IsDebugRegister(REG reg);
bool IsSegmentRegister(REG reg);
bool IsXmmRegister(REG reg);
bool IsYmmRegister(REG reg);

REG RegFromString(const char* text);

xed_reg_enum_t RegToXed(REG reg);

const char* RegToString(REG reg);

REGSIZE RegGetSize(REG reg);