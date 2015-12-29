#include "Translator.h"
#include <stdio.h>

void SetMemoryDisplacementOrBase(XEDPARSE* Parse, const char* Value, InstOperand* Operand)
{
    // Displacement = name or number
    // Base         = register
    REG registerVal = RegFromString(Value);
    ULONGLONG disp  = 0;

    if(registerVal != REG_INVALID)
    {
        // If the base is already set, then use IMPLICIT scale
        // REG + (REG * 1)
        if(Operand->Mem.Base)
        {
            SetMemoryIndexOrScale(Parse, Value, Operand);
            SetMemoryIndexOrScale(Parse, "1", Operand);
            return;
        }

        // It's the base
        Operand->Mem.Base    = true;
        Operand->Mem.BaseVal = registerVal;
    }
    else if(valfromstring(Value, &disp) || (Parse->cbUnknown && Parse->cbUnknown(Value, &disp)))
    {
        // It's the displacement
        //
        // Displacement is either /8, /32, or /64
        // 5h = 101b
        Operand->Mem.Disp    = true;
        Operand->Mem.DispVal = disp;
        bool sign            = (Value[0] == '-');

        if(sign)
            Operand->Mem.DispWidth = OpsizeFromInt(xed_shortest_width_signed(disp, 0x5));
        else
            Operand->Mem.DispWidth = OpsizeFromInt(xed_shortest_width_unsigned(disp, 0x5));

        Operand->Mem.DispWidth = PromoteImmediateWidth(sign, disp, Operand->Mem.DispWidth);
    }
    else
    {
        sprintf(Parse->error, "Unknown displacement or base '%s'", Value);
    }
}

void SetMemoryIndexOrScale(XEDPARSE* Parse, const char* Value, InstOperand* Operand)
{
    // Index = register
    // Scale = 1, 2, 4, or 8
    REG registerVal = RegFromString(Value);
    ULONGLONG scale = 0;

    if(registerVal != REG_INVALID)
    {
        // It's the index
        Operand->Mem.Index    = true;
        Operand->Mem.IndexVal = registerVal;
    }
    else if(valfromstring(Value, &scale))
    {
        // It's the scale
        Operand->Mem.Scale    = true;
        Operand->Mem.ScaleVal = scale;
    }
    else
    {
        sprintf(Parse->error, "Unknown index or scale '%s'", Value);
    }
}

bool HandleMemoryOperand(XEDPARSE* Parse, const char* Value, InstOperand* Operand)
{
    // Copy the prefix into a buffer
    char prefix[64];

    strcpy(prefix, Value);
    *strchr(prefix, '[') = '\0';

    // Copy the calculation into a buffer (strrchr is intentional)
    char calc[64];

    strcpy(calc, strrchr(Value, '[') + 1);
    *strchr(calc, ']') = '\0';

    // Gather any information & check the prefix validity
    if(strlen(prefix) > 0)
    {
        // Prefix must be lowercase
        _strlwr(prefix);

        // Remove 'ptr' if it exists and remove colons
        StrDel(prefix, "ptr", '\0');
        StrDel(prefix, ":", '\0');

        // Check if the segment can be used
        size_t len = strlen(prefix);

        if(len >= 2)
        {
            // Move backwards in order to get the segment (SIZE-SEG[CALC])
            char* segPtr = (prefix + (len - 2));

            // See if the segment is actually valid
            REG segment = RegFromString(segPtr);

            if(segment != REG_INVALID)
            {
                // Set the new segment
                Operand->Segment = segment;

                // End the string here
                *segPtr = '\0';
            }
        }

        // Determine the prefix size
        Operand->Size = OpsizeFromString(prefix);

        // If the size is UNSET and there's still chars left in the string,
        // it is an invalid identifier
        if(Operand->Size == SIZE_UNSET && strlen(prefix) > 0)
        {
            sprintf(Parse->error, "Unknown identifier in '%s'", Value);
            return false;
        }

        // Update the effective operand size for Xed
        Operand->XedEOSZ = OpsizeToEosz(Operand->Size);
    }

    // Begin determining the calculation
    // [Base + (Index * Scale) + Displacement] -- IN ANY ORDER
    if(strlen(calc) <= 0)
    {
        strcpy(Parse->error, "Invalid memory calculation");
        return false;
    }

    char temp[32];
    char* ptr    = calc;
    char* base   = temp;
    bool mulFlag = false;

    for(;; ptr++)
    {
        *base = '\0';

        switch(*ptr)
        {
        case '\0':
        case '+':
        case '-':
            if(mulFlag)
                SetMemoryIndexOrScale(Parse, temp, Operand);
            else
                SetMemoryDisplacementOrBase(Parse, temp, Operand);

            base    = temp;
            mulFlag = false;

            if(*ptr == '-')
                *base++ = '-';
            break;

        case '*':
            SetMemoryIndexOrScale(Parse, temp, Operand);

            base    = temp;
            mulFlag = true;
            break;

        default:
            *base++ = *ptr;
            break;
        }

        if(*ptr == '\0')
            break;
    }

    // Fix up the operand segment
    if(Operand->Segment == REG_INVALID)
    {
        if((Operand->Mem.BaseVal == REG_ESP || Operand->Mem.BaseVal == REG_EBP) && !Parse->x64)
        {
            // If the segment isn't set and the base is ESP or EBP,
            // auto-set the segment to SS
            Operand->Segment = REG_SS;
        }
        else
        {
            // Default to DS
            // 64-bit also doesn't have true segments except for FS/GS/DS
            Operand->Segment = REG_DS;
        }
    }

    if(Operand->Mem.Disp)
    {
        // If the base isn't set, the displacement must be at least 32 bits
        if(!Operand->Mem.Base)
            Operand->Mem.DispWidth = max(Operand->Mem.DispWidth, SIZE_DWORD);

        // Use RIP-relative addressing per default when on x64 and when the displacement is set
        // and when the segment is SEG_DS
        if(Parse->x64 && Operand->Segment == REG_DS)
        {
            if(!Operand->Mem.Base && !Operand->Mem.Index && !Operand->Mem.Scale)
            {
                // If a 32-bit address was given, don't apply RIP-relative addressing
                if(Operand->Mem.DispWidth != SIZE_DWORD)
                {
                    LONGLONG newDisp = TranslateRelativeCip(Parse, Operand->Mem.DispVal - 6, true);
                    ULONGLONG masked = newDisp & 0xFFFFFFFF00000000;

                    // Check if the mask fits into a 32-bit variable (taking sign-extension into account)
                    if(masked == 0 || masked == 0xFFFFFFFF00000000)
                    {
                        Operand->Mem.DispVal         = newDisp;
                        Operand->Mem.DispWidth       = SIZE_DWORD;
                        Operand->Mem.Base            = true;
                        Operand->Mem.BaseVal         = REG_RIP;
                    }
                    else
                        Operand->Mem.DispWidth = SIZE_QWORD;
                }
            }
        }
    }
    else if(!Operand->Mem.Base && Operand->Mem.Index && Operand->Mem.Scale)
    {
        // Handle when only an index register and scale are set
        // Ex: MOV EAX, [ECX * 8]
        Operand->Mem.Disp      = true;
        Operand->Mem.DispVal   = 0;
        Operand->Mem.DispWidth = SIZE_DWORD;
    }

    return true;
}