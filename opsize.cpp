#include "Translator.h"

int OpsizeToBits(OPSIZE opsize)
{
    return OpsizeEntryTable[opsize].Bits;
}

int OpsizeToInt(OPSIZE opsize)
{
    return OpsizeEntryTable[opsize].Bytes;
}

OPSIZE OpsizeFromBits(int bits)
{
    return OpsizeFromInt(bits / 8);
}

OPSIZE OpsizeFromInt(int opsize)
{
    switch(opsize)
    {
    case 1:
        return SIZE_BYTE;
    case 2:
        return SIZE_WORD;
    case 4:
        return SIZE_DWORD;
    case 6:
        return SIZE_FWORD;
    case 8:
        return SIZE_QWORD;
    case 10:
        return SIZE_TBYTE;
    case 16:
        return SIZE_XMMWORD;
    case 32:
        return SIZE_YMMWORD;
    case 64:
        return SIZE_ZMMWORD;
    }

    return SIZE_UNSET;
}

int OpsizeToEosz(OPSIZE Value)
{
    // OPSIZE to XED's effective operand size
    switch(Value)
    {
    case SIZE_BYTE:
        return 0;
    case SIZE_WORD:
        return 1;
    case SIZE_DWORD:
        return 2;
    case SIZE_QWORD:
        return 3;
    }

    return 0;
}

OPSIZE OpsizeFromString(const char* Value)
{
    for(int i = 0; i < ARRAYSIZE(OpsizeEntryTable); i++)
    {
        if(!_stricmp(OpsizeEntryTable[i].Name, Value))
            return OpsizeEntryTable[i].Size;
    }

    return SIZE_UNSET;
}

const char* OpsizeToString(OPSIZE Size)
{
    return OpsizeEntryTable[Size].Name;
}

OPSIZE OpsizeFromValue(LONGLONG Value)
{
    int sign    = xed_shortest_width_signed(Value, 0xFF);
    int unsign  = xed_shortest_width_unsigned(Value, 0xFF);

    return OpsizeFromInt(min(sign, unsign));
}