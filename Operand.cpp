#include "Translator.h"
#include <stdio.h>

void SetMemoryDisplacementOrBase(XEDPARSE* Parse, const char* Value, InstOperand* Operand)
{
    // Displacement = name or number
    // Base         = register
    REG registerVal = getregister(Value);
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
        Operand->Mem.Base       = true;
        Operand->Mem.BaseVal    = registerVal;
    }
    else if(valfromstring(Value, &disp) || (Parse->cbUnknown && Parse->cbUnknown(Value, &disp)))
    {
        // It's the displacement
        //
        // Displacement is either /8, /32, /64
        // 5h = 101b
        Operand->Mem.Disp       = true;
        Operand->Mem.DispVal    = disp;
        bool sign               = (Value[0] == '-');

        if(sign)
            Operand->Mem.DispWidth = inttoopsize(xed_shortest_width_signed(disp, 0x5));
        else
            Operand->Mem.DispWidth = inttoopsize(xed_shortest_width_unsigned(disp, 0x5));

        // half     = pow(2, n_bits - 1);
        // minhalf  = -half;
        xed_uint64_t maxhalf = 1 << (opsizetobits(Operand->Mem.DispWidth) - 1);
        xed_uint64_t minhalf = 1 + (~maxhalf);

        // Values that are <  than -(MAX_VALUE/2) need to use the next biggest size
        // Values that are >= than +(MAX_VALUE/2) need to use the next biggest size
        if((sign && disp < minhalf) || (!sign && disp >= maxhalf))
        {
            switch(Operand->Mem.DispWidth)
            {
            case SIZE_BYTE:
                Operand->Mem.DispWidth = SIZE_DWORD;
                break;
            case SIZE_DWORD:
                Operand->Mem.DispWidth = SIZE_QWORD;
                break;
            }
        }
    }
    else
    {
        sprintf(Parse->error, "Unknown displacement or base '%s'", Value);
    }
}

void SetMemoryIndexOrScale(XEDPARSE* Parse, const char* Value, InstOperand* Operand)
{
    // Index = register
    // Scale = 1, 2, 4, 8
    REG registerVal = getregister(Value);
    ULONGLONG scale = 0;

    if(registerVal != REG_INVALID)
    {
        // It's the index
        Operand->Mem.Index      = true;
        Operand->Mem.IndexVal   = registerVal;
    }
    else if(valfromstring(Value, &scale))
    {
        // It's the scale
        Operand->Mem.Scale      = true;
        Operand->Mem.ScaleVal   = scale;
    }
    else
    {
        sprintf(Parse->error, "Unknown index or scale '%s'", Value);
    }
}

bool HandleMemoryOperand(XEDPARSE* Parse, const char* Value, InstOperand* Operand)
{
    char prefix[64];
    char calc[64];

    // Copy the prefix into a buffer
    strcpy(prefix, Value);
    *strchr(prefix, '[') = '\0';

    // Copy the calculation into a buffer (strrchr is intentional)
    strcpy(calc, strrchr(Value, '[') + 1);
    *strchr(calc, ']') = '\0';

    // Gather any information & check the prefix validity
    if(strlen(prefix) > 0)
    {
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
            SEG segment = getsegment(segPtr);

            if(segment != SEG_INVALID)
            {
                // Set the new segment
                Operand->Segment = segment;

                // End the string here
                *segPtr = '\0';
            }
        }

        // Determine the prefix size
        Operand->Size = StringToOpsize(prefix);

        // If the size is UNSET and there's still chars left in the string,
        // it is an invalid identifier
        if(Operand->Size == SIZE_UNSET && strlen(prefix) > 0)
        {
            sprintf(Parse->error, "Unknown identifier in '%s'", Value);
            return false;
        }

        // Update the effective operand size for XED
        Operand->XedEOSZ = opsizetoeosz(Operand->Size);
    }

    // Begin determining the calculation
    // [Base + (Index * Scale) + Displacement] -- IN ANY ORDER
    if(strlen(calc) <= 0)
    {
        strcpy(Parse->error, "Invalid memory calculation");
        return false;
    }

    char temp[32];
    char* base      = temp;
    bool mulFlag    = false;

    for(char* ptr = calc;; ptr++)
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
    if(Operand->Segment == SEG_INVALID)
    {
        if(Operand->Mem.BaseVal == REG_ESP && !Parse->x64)
        {
            // If the segment isn't set and the base is ESP,
            // set the segment to SS
            Operand->Segment = SEG_SS;
        }
        else
        {
            // Default to DS
            // 64-bit doesn't have true segments except for FS/GS/DS
            Operand->Segment = SEG_DS;
        }
    }

    if(Operand->Mem.Disp)
    {
        // If the base isn't set, the displacement must be at least 32 bits
        if(!Operand->Mem.Base)
            Operand->Mem.DispWidth = max(Operand->Mem.DispWidth, SIZE_DWORD);

        // Use RIP-relative addressing per default when on x64 and when the displacement is set
        // and when the segment is SEG_DS
        if(Parse->x64 && Operand->Segment == SEG_DS)
        {
            if(!Operand->Mem.Base && !Operand->Mem.Index && !Operand->Mem.Scale)
            {
                // If a 32-bit address was given, don't apply RIP-relative addressing
                if(Operand->Mem.DispWidth != SIZE_DWORD)
                {
                    LONGLONG newDisp = TranslateRelativeCip(Parse, Operand->Mem.DispVal - 6, true);
                    ULONGLONG masked = newDisp & 0xFFFFFFFF00000000;

                    if(masked == 0 || masked == 0xFFFFFFFF00000000)
                    {
                        Operand->Mem.DispRipRelative = true;
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
    else if(!Operand->Mem.Base && Operand->Mem.Index && Operand->Mem.Scale) // MOV EAX, [ECX*8]
    {
        Operand->Mem.Disp       = true;
        Operand->Mem.DispVal    = 0;
        Operand->Mem.DispWidth  = SIZE_DWORD;
    }

    return true;
}

bool HandleSegSelectorOperand(XEDPARSE* Parse, const char* Value, InstOperand* Operand)
{
    char selector[64];
    char offset[64];

    // Copy the prefix into a buffer
    strcpy(selector, Value);
    *strchr(selector, ':') = '\0';

    // Copy the offset
    strcpy(offset, strrchr(Value, ':') + 1);

    // The segment selector is always a number
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

bool AnalyzeOperand(XEDPARSE* Parse, const char* Value, InstOperand* Operand)
{
    REG registerVal     = getregister(Value);
    ULONGLONG immVal    = 0;

    if(registerVal != REG_INVALID)
    {
        // Register
        Operand->Type       = OPERAND_REG;
        Operand->Segment    = SEG_INVALID;
        Operand->Size       = getregsize(registerVal);
        Operand->XedEOSZ    = opsizetoeosz(Operand->Size);
        Operand->Reg.Reg    = registerVal;
        Operand->Reg.XedReg = regtoxed(registerVal);
    }
    else if(strchr(Value, '[') && strchr(Value, ']'))
    {
        // Memory
        Operand->Type       = OPERAND_MEM;
        Operand->Segment    = SEG_INVALID;
        Operand->Size       = SIZE_UNSET;
        Operand->XedEOSZ    = EOSZ_64_32(Parse->x64);

        return HandleMemoryOperand(Parse, Value, Operand);
    }
    else if(strchr(Value, ':'))
    {
        // Segment selector operand
        Operand->Type       = OPERAND_SEGSEL;
        Operand->Segment    = SEG_INVALID;
        Operand->Size       = SIZE_DWORD;
        Operand->XedEOSZ    = EOSZ_64_32(Parse->x64);

        return HandleSegSelectorOperand(Parse, Value, Operand);
    }
    else if(valfromstring(Value, &immVal) || (Parse->cbUnknown && Parse->cbUnknown(Value, &immVal)))
    {
        // Immediate
        Operand->Type       = OPERAND_IMM;
        Operand->Segment    = SEG_INVALID;
        Operand->Size       = ValueToOpsize(immVal);
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
