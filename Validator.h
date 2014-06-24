#pragma once

#include "Parser.h"

struct IClassType
{
	xed_iclass_enum_t	IClass;
	const xed_inst_t	*Instructions[32];
	int					InstructionCount;
	unsigned int    	MinimumOperands;
};

extern IClassType XedInstLookupTable[];

static XED_INLINE xed_uint_t xed_operand_is_memory_addressing(xed_operand_enum_t name)
{
	return (name == XED_OPERAND_MEM0 || name == XED_OPERAND_MEM1);
}

bool ResizeSingleOperand(XEDPARSE *Parse, xed_iclass_enum_t IClass, InstOperand *Operand);
bool ValidateInstOperands(XEDPARSE *Parse, Inst *Instruction);
void LookupTableInit();