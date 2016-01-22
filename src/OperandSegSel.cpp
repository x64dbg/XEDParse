#include "Translator.h"
#include <stdio.h>

bool HandleSegSelectorOperand(XEDPARSE* Parse, const char* Value, InstOperand* Operand)
{
    char selector[64];
    char offset[64];

    // Copy the prefix into a buffer
    strcpy(selector, Value);
    *strchr(selector, ':') = '\0';

    // Copy the offset
    strcpy(offset, strrchr(Value, ':') + 1);

    // The segment selector is always a number from 0 to 0xFFFF
    if(strlen(selector) > 0)
    {
        ULONGLONG selVal = 0;

        if(!valfromstring(selector, &selVal) || abs((LONGLONG)selVal) > USHRT_MAX)
        {
            strcpy(Parse->error, "Invalid segment selector value");
            return false;
        }

        Operand->Sel.Selector = selVal & 0xFFFF;
    }
    else
    {
        strcpy(Parse->error, "Invalid segment selector");
        return false;
    }

    // Determine offset
    if(strlen(offset) <= 0)
    {
        strcpy(Parse->error, "Invalid offset");
        return false;
    }

    // Ex: 033:X86SwitchTo64BitMode
    ULONGLONG offsetVal = 0;

    if(!valfromstring(offset, &offsetVal) && !(Parse->cbUnknown && Parse->cbUnknown(offset, &offsetVal)))
    {
        sprintf(Parse->error, "Unable to parse offset '%s'", offset);
        return false;
    }

    Operand->Sel.Offset = offsetVal & 0xFFFFFFFF;
    return true;
}