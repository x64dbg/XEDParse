#include "translate.h"

REG getregister(const char* text)
{
	size_t textLen = strlen(text);

	// Loop through each entry looking for the register
	for (int i = 0; i < ARRAYSIZE(RegisterIds); i++)
	{
		if (!_strnicmp(text, RegisterIds[i].Name, textLen))
			return RegisterIds[i].RegId;
	}

	// No register found or it was not implemented
	return REG_NAN;
}

SEG getsegment(const char* text)
{
	size_t textLen = strlen(text);

	// Loop through each entry looking for the segment register
	for (int i = 0; i < ARRAYSIZE(SegmentIds); i++)
	{
		if (!_strnicmp(text, SegmentIds[i].Name, textLen))
			return SegmentIds[i].SegId;
	}

	// X86 and X64 both default to DS
	return SEG_DS;
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