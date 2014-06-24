#include "ParseTest.h"
#include "RegTable.h"
#include <stdio.h>

void OperandToString(char *Buffer, InstOperand *Operand)
{
	switch (Operand->Type)
	{
	case OPERAND_INVALID:
		strcpy(Buffer, "(INVALID OPERAND)");
		break;

	case OPERAND_REG:
		strcpy(Buffer, regtostring(Operand->Reg.Reg));
		break;

	case OPERAND_IMM:
		if (Operand->Imm.Signed)
			sprintf(Buffer, "-0x%llx", (~Operand->Imm.imm) + 1);
		else
			sprintf(Buffer, "0x%llx", Operand->Imm.imm);
		break;

	case OPERAND_MEM:
	{
		char base[64];
		char scale[64];

		memset(base, 0, sizeof(base));
		memset(scale, 0, sizeof(scale));

		if (Operand->Mem.Base)
			sprintf(base, "%s + ", regtostring(Operand->Mem.BaseVal));

		if (Operand->Mem.Index)
			sprintf(scale, "%s * %d + ", regtostring(Operand->Mem.IndexVal), Operand->Mem.ScaleVal);

		sprintf(Buffer, "%s ptr %s:[%s%s0x%llX/%d]",
			OpsizeToString(Operand->Size),
			segtostring(Operand->Segment),
			base,
			scale,
			Operand->Mem.DispVal,
			opsizetobits(Operand->Mem.DispWidth));
	}
	break;
	}
}

void InstructionToString(char *Buffer, Inst *Instruction)
{
	// Add the prefix is there was one
	if (Instruction->Prefix != PREFIX_NONE)
	{
		strcat(Buffer, PrefixToString(Instruction->Prefix));
		strcat(Buffer, " ");
	}

	// Append mnemonic
	strcat(Buffer, Instruction->Mnemonic);

	// Append all operands
	if (Instruction->OperandCount > 0)
	{
		strcat(Buffer, " ");

		for (int i = 0; i < Instruction->OperandCount; i++)
		{
			char op[XEDPARSE_MAXBUFSIZE];
			OperandToString(op, &Instruction->Operands[i]);

			strcat(Buffer, op);
			strcat(Buffer, ", ");
		}

		*strrchr(Buffer, ',') = '\0';
	}
}