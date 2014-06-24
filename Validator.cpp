#include "Translator.h"

// This maps an ICLASS to all of its IFORMs
IClassType XedInstLookupTable[XED_ICLASS_LAST];

bool ResizeSingleOperand(XEDPARSE *Parse, xed_iclass_enum_t IClass, InstOperand *Operand)
{
	IClassType *type = &XedInstLookupTable[IClass];

	switch (Operand->Type)
	{
	case OPERAND_REG:
		// Registers can't be resized
		return true;

	case OPERAND_IMM:
	{
		// Find the smallest instruction imm size that fits the input
		// Set an arbitrarily high value for fixedSize
		int targetSize	= opsizetobits(Operand->Size);
		int fixedSize	= INT_MAX;

		for (int i = 0; i < type->InstructionCount; i++)
		{
			const xed_inst_t *inst			= type->Instructions[i];
			const xed_operand_t *op			= xed_inst_operand(inst, 0);
			xed_operand_type_enum_t type	= xed_operand_type(op);

			if (type != XED_OPERAND_TYPE_IMM &&
				type != XED_OPERAND_TYPE_IMM_CONST)
				continue;

			int size = xed_operand_width_bits(op, Operand->XedEOSZ);

			if (size >= targetSize && size <= fixedSize)
				fixedSize = size;
		}

		if (fixedSize == INT_MAX)
		{
			strcpy(Parse->error, "Immediate size is too large");
			return false;
		}

		Operand->Size = bitstoopsize(fixedSize);
		return true;
	}
	break;

	case OPERAND_MEM:
	{
		// If the memory size is set, there's nothing to do here
		if (Operand->Size != SIZE_UNSET)
			return true;

		// Going off of GCC's 'AS.exe' assembler
		// https://defuse.ca/online-x86-assembler.htm#disassembly
		//
		// This defaults to the full size, which is all 32 or all 64 bits,
		// if the instruction has multiple memory types -> AMBIGUOUS
		int memoryOperandCount	= 0;
		unsigned int memoryOperandSize	= 0;

		for (int i = 0; i < type->InstructionCount; i++)
		{
			const xed_inst_t *inst		= type->Instructions[i];
			const xed_operand_t *op		= xed_inst_operand(inst, 0);
			xed_operand_enum_t name		= xed_operand_name(op);

			if (xed_operand_is_memory_addressing(name))
				memoryOperandCount++;

			memoryOperandSize = max(memoryOperandSize, xed_operand_width_bits(op, Operand->XedEOSZ));
		}

		// Check if ambiguous
		if (memoryOperandCount > 1)
		{
			strcpy(Parse->error, "Ambiguous memory size");
			return false;
		}

		Operand->Size = bitstoopsize(memoryOperandSize);
		return true;
	}
	break;
	}

	return false;
}

bool ResizeDoubleOperands(XEDPARSE *Parse, xed_iclass_enum_t IClass, InstOperand *Operands)
{
	IClassType *type = &XedInstLookupTable[IClass];

	const xed_inst_t *inst;
	const xed_operand_t *xedOp[2];

	switch (Operands[0].Type)
	{
	case OPERAND_REG:
		switch (Operands[1].Type)
		{
		case OPERAND_IMM: // <instr> reg, value
		{
			int targetSize = opsizetobits(Operands[1].Size);
			int fixedSize = INT_MAX;

			for (int i = 0; i < type->InstructionCount; i++)
			{
				inst		= type->Instructions[i];
				xedOp[0]	= xed_inst_operand(inst, 0);
				xedOp[1]	= xed_inst_operand(inst, 1);

				// Match the register first
				if (!xed_operand_template_is_register(xedOp[0]))
					continue;

				if (xed_operand_width_bits(xedOp[0], Operands[0].XedEOSZ) != opsizetobits(Operands[0].Size))
					continue;

				// Check compatible immediate types
				if (xed_operand_type(xedOp[1]) != XED_OPERAND_TYPE_IMM &&
					xed_operand_type(xedOp[1]) != XED_OPERAND_TYPE_IMM_CONST)
					continue;

				int size = xed_operand_width_bits(xedOp[1], Operands[1].XedEOSZ);

				if (size >= targetSize && size <= fixedSize)
					fixedSize = size;
			}

			if (fixedSize == INT_MAX)
			{
				strcpy(Parse->error, "Immediate size is too large");
				return false;
			}

			Operands[1].Size = bitstoopsize(fixedSize);
			return true;

			return true;
		}
		break;

		case OPERAND_MEM: // <instr> reg, []
			// ...
			return true;
		}
	break;

	case OPERAND_MEM:
		switch (Operands[1].Type)
		{
		case OPERAND_REG: // <instr> [], reg
			return true;
		case OPERAND_IMM: // <instr> [], value
			// Potential ambiguity
			return true;
		}
	break;
	}


	return true;
}

bool ValidateInstOperands(XEDPARSE *Parse, Inst *Instruction)
{
	// Only the first two operands actually matter right now
	// Instructions with 3+ operands will be handled later

	// XED IS WRONG (xed_inst_noperands() shouldn't be used)
	// This is incorrect for now
	// int minimumOperands = XedInstLookupTable[Instruction->Class].MinimumOperands;

	// So what happens first?
	//
	// Verify that the number of user-supplied operands is valid
	//
	//if (Instruction->OperandCount < minimumOperands)
	//	return false;

	//
	// Instructions with no operands do not apply here
	//
	if (Instruction->OperandCount <= 0)
		return true;

	//
	// Instructions with a single operand are "easy" to solve
	// Simply match the [imm/mem] size with an IFORM
	//
	if (Instruction->OperandCount <= 1)
		return ResizeSingleOperand(Parse, Instruction->Class, &Instruction->Operands[0]);

	//
	// Check all invalid cases and then pass it to ResizeDoubleOperands
	//
	switch (Instruction->Operands[0].Type)
	{
	case OPERAND_REG:
		switch (Instruction->Operands[1].Type)
		{
		case OPERAND_REG: // <instr> reg, reg
			// Registers can't be resized
			return true;

		case OPERAND_IMM: // <instr> reg, value
			return ResizeDoubleOperands(Parse, Instruction->Class, Instruction->Operands);

		case OPERAND_MEM: // <instr> reg, []
			// Don't care if it is already set
			if (Instruction->Operands[1].Size != SIZE_UNSET)
				return true;

			// ...
			return true;
		}
		break;

	case OPERAND_IMM:
		switch (Instruction->Operands[1].Type)
		{
		case OPERAND_REG:
		case OPERAND_IMM:
		case OPERAND_MEM:
			strcpy(Parse->error, "Operand immediate mismatch");
			return false;

		//case OPERAND_MEM1:
		//	return true;
		}
		break;

	case OPERAND_MEM:
		switch (Instruction->Operands[1].Type)
		{
		case OPERAND_REG: // <instr> [], reg
			return true;
		case OPERAND_IMM: // <instr> [], value
			// Potential ambiguity
			return true;
		case OPERAND_MEM: // <instr> [], []
			strcpy(Parse->error, "Too many memory references");
			return false;
		}
		break;
	}

	return false;
}

void LookupTableInit()
{
	xed_tables_init();

	// Initialize everything
	for (int i = 0; i < XED_ICLASS_LAST; i++)
	{
		IClassType *type		= &XedInstLookupTable[i];
		type->IClass			= XED_ICLASS_INVALID;
		type->MinimumOperands	= INT_MAX;
		type->InstructionCount	= 0;
	}

	// Query XED
	for (int i = 0; i < XED_MAX_INST_TABLE_NODES; i++)
	{
		const xed_inst_t *inst		= &xed_inst_table_base()[i];
		xed_iclass_enum_t iclass	= xed_inst_iclass(inst);

		// Set the basic information/update the pointers
		IClassType *type		= &XedInstLookupTable[iclass];
		type->IClass			= iclass;
		type->MinimumOperands	= min(xed_inst_noperands(inst), type->MinimumOperands);

		type->Instructions[type->InstructionCount++] = inst;
	}
}