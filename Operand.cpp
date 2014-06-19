#include "Translator.h"

void SetMemoryDisplacementOrBase(XEDPARSE *Parse, const char *Value, InstOperand *Operand)
{
	// Displacement = name or number
	// Base			= register
	REG registerVal	= getregister(Value);
	ULONGLONG disp	= 0;

	if (registerVal != REG_INVALID)
	{
		// It's the base
		Operand->Mem.Base		= true;
		Operand->Mem.BaseVal	= registerVal;
	}
	else if (valfromstring(Value, &disp) || (Parse->cbUnknown && Parse->cbUnknown(Value, &disp)))
	{
		// It's the displacement
		//
		// Displacement is either /8, /32, /64
		// 5h = 101b
		Operand->Mem.Disp		= true;
		Operand->Mem.DispVal	= disp;

		if (Value[0] == '-')
			Operand->Mem.DispWidth = inttoopsize(xed_shortest_width_signed(disp, 0x5));
		else
			Operand->Mem.DispWidth = inttoopsize(xed_shortest_width_unsigned(disp, 0x5));
	}
	else
	{
		sprintf(Parse->error, "Unknown displacement or base '%s'", Value);
	}
}

void SetMemoryIndexOrScale(XEDPARSE *Parse, const char *Value, InstOperand *Operand)
{
	// Index = register
	// Scale = 1, 2, 4, 8
	REG registerVal = getregister(Value);
	ULONGLONG scale	= 0;

	if (registerVal != REG_INVALID)
	{
		// It's the index
		Operand->Mem.Index		= true;
		Operand->Mem.IndexVal	= registerVal;
	}
	else if (valfromstring(Value, &scale))
	{
		// It's the scale
		Operand->Mem.Scale		= true;
		Operand->Mem.ScaleVal	= scale;
	}
	else
	{
		sprintf(Parse->error, "Unknown index or scale '%s'", Value);
	}
}

bool HandleMemoryOperand(XEDPARSE *Parse, const char *Value, InstOperand *Operand)
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
	if (strlen(prefix) > 0)
	{
		// Remove 'ptr' if it exists and remove colons
		{
			char *base	= prefix;
			char *ptr	= prefix;

			while (*ptr)
			{
				if (ptr[0] == 'p' && ptr[1] == 't' && ptr[2] == 'r')
					ptr += 3;

				if (ptr[0] == ':')
				{
					ptr++;
					continue;
				}

				*base++ = *ptr++;
			}

			*base = '\0';
		}

		// Check if the segment can be used
		size_t len = strlen(prefix);

		if (len > 0)
		{
			// Move backwards in order to get the segment (SIZE SG[CALC])
			char *segPtr = (prefix + (len - 2));

			// See if the segment is actually valid
			SEG seg = getsegment(segPtr);

			if (seg != SEG_INVALID)
			{
				// End the string here
				*segPtr = '\0';

				Operand->Segment = seg;
			}
		}

		// Determine the prefix size
		Operand->Size = StringToOpsize(prefix);
		
		// If the size is UNSET and there's still chars left in the string,
		// it is an invalid identifier
		if (Operand->Size == SIZE_UNSET && strlen(prefix) > 0)
		{
			sprintf(Parse->error, "Unknown identifier in '%s'", Value);
			return false;
		}
	}

	// Begin determining the calculation
	// [Base + (Index * Scale) + Displacement] -- IN ANY ORDER
	if (strlen(calc) <= 0)
		return false;

	char temp[32];
	char *base		= temp;
	bool mulFlag	= false;

	for (char *ptr = calc;; ptr++)
	{
		*base = '\0';

		switch (*ptr)
		{
		case '\0':
		case '+':
		case '-':
			if (mulFlag)
				SetMemoryIndexOrScale(Parse, temp, Operand);
			else
				SetMemoryDisplacementOrBase(Parse, temp, Operand);

			base	= temp;
			mulFlag = false;
			
			if (*ptr == '-')
				*base++ = *ptr;
			break;

		case '*':
			SetMemoryIndexOrScale(Parse, temp, Operand);

			base	= temp;
			mulFlag = true;
			break;

		default:
			*base++ = *ptr;
			break;
		}

		if (*ptr == '\0')
			break;
	}

	return true;
}

bool AnalyzeOperand(XEDPARSE *Parse, const char *Value, InstOperand *Operand)
{
	REG registerVal		= getregister(Value);
	ULONGLONG immVal	= 0;

	if (strchr(Value, '[') && strchr(Value, ']'))
	{
		// Memory operand
		// Assume SEG_DS by default
		Operand->Type		= OPERAND_MEM;
		Operand->Segment	= SEG_DS;
		Operand->Size		= SIZE_UNSET;
		Operand->XedEOSZ	= 0;

		return HandleMemoryOperand(Parse, Value, Operand);
	}
	else if (registerVal != REG_INVALID)
	{
		// Register
		Operand->Type		= OPERAND_REG;
		Operand->Segment	= SEG_INVALID;
		Operand->Size		= getregsize(registerVal);
		Operand->XedEOSZ	= opsizetoeosz(Operand->Size);
		Operand->Reg.Reg	= registerVal;
		Operand->Reg.XedReg = regtoxed(registerVal);
	}
	else if (valfromstring(Value, &immVal) || (Parse->cbUnknown && Parse->cbUnknown(Value, &immVal)))
	{
		// Immediate
		Operand->Type		= OPERAND_IMM;
		Operand->Segment	= SEG_INVALID;
		Operand->XedEOSZ	= (Parse->x64) ? 3 : 2;
		Operand->Imm.Signed = (Value[0] == '-');
		Operand->Imm.imm	= immVal;

		if (Operand->Imm.Signed)
			Operand->Size = inttoopsize(xed_shortest_width_signed(immVal, 0xFF));
		else
			Operand->Size = inttoopsize(xed_shortest_width_unsigned(immVal, 0xFF));
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