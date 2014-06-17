#include "Translator.h"

bool IsControlRegister(REG reg)
{
	switch (reg)
	{
	case REG_CR0:
	case REG_CR2:
	case REG_CR3:
	case REG_CR4:
#ifdef _WIN64
	case REG_CR8:
#endif
		return true;
	}

	return false;
}

bool IsDebugRegister(REG reg)
{
	switch (reg)
	{
	case REG_DR0:
	case REG_DR1:
	case REG_DR2:
	case REG_DR3:
	case REG_DR6:
	case REG_DR7:
		return true;
	}

	return false;
}

bool IsSegmentRegister(REG reg)
{
	switch (reg)
	{
	case REG_CS:
	case REG_DS:
	case REG_ES:
	case REG_FS:
	case REG_GS:
	case REG_SS:
		return true;
	}

	return false;
}

bool IsXmmRegister(REG reg)
{
#ifdef _WIN64
	return (reg >= REG_XMM0 && reg <= REG_XMM15);
#else
	return (reg >= REG_XMM0 && reg <= REG_XMM7);
#endif
}

bool IsYmmRegister(REG reg)
{
#ifdef _WIN64
	return (reg >= REG_YMM0 && reg <= REG_YMM15);
#else
	return (reg >= REG_YMM0 && reg <= REG_YMM7);
#endif
}

REG getregister(const char* text)
{
	// Loop through each entry looking for the register
	for (int i = 0; i < ARRAYSIZE(RegisterIds); i++)
	{
		if (!_stricmp(RegisterIds[i].Name, text))
			return RegisterIds[i].RegId;
	}

	// No register found or it was not implemented
	return REG_INVALID;
}

SEG getsegment(const char* text)
{
	// Loop through each entry looking for the segment register
	for (int i = 0; i < ARRAYSIZE(SegmentIds); i++)
	{
		if (!_stricmp(SegmentIds[i].Name, text))
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

const char *regtostring(REG reg)
{
	return RegisterIds[reg].Name;
}

const char *segtostring(SEG seg)
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