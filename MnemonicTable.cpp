#include "Translator.h"

const char *MnemonicToXed(const char *Mnemonic)
{
	for (int i = 0; i < ARRAYSIZE(XedMnemonicTable); i++)
	{
		if (!_stricmp(Mnemonic, XedMnemonicTable[i].Name))
			return XedMnemonicTable[i].XedName;
	}

	return Mnemonic;
}

char *InstMnemonicToXed(XEDPARSE *Parse, Inst *Instruction)
{
	char *mnemonic = Instruction->Mnemonic;

	// Convert the name to XED format
	strcpy(mnemonic, MnemonicToXed(mnemonic));

	/*
	TODO:

	STOS
	BEXTR_XOP
	PEXTRW_SSE4
	PREFETCH_EXCLUSIVE
	PREFETCH_RESERVED
	*/

	// Special cases depending on the operands
	for (int i = 0; i < Instruction->OperandCount; i++)
	{
		InstOperand *operand = &Instruction->Operands[i];

		if (operand->Type == OPERAND_REG)
		{
			if (!_stricmp(mnemonic, "mov"))
			{
				if (IsControlRegister(operand->Reg.Reg))
					strcpy(mnemonic, "mov_cr");
				else if (IsDebugRegister(operand->Reg.Reg))
					strcpy(mnemonic, "mov_dr");
			}

			if (!_stricmp(mnemonic, "cmpsd"))
			{
				if (IsXmmRegister(operand->Reg.Reg))
					strcpy(mnemonic, "cmpsd_xmm");
			}

			if (!_stricmp(mnemonic, "movsd"))
			{
				if (IsXmmRegister(operand->Reg.Reg))
					strcpy(mnemonic, "movsd_xmm");
			}
		}
	}

	// MOVS(B/W/D/Q) case
	if (!_strnicmp(mnemonic, "movs", 4))
	{
		switch (tolower(mnemonic[4]))
		{
		case 's':
		case 'w':
		case 'd':
		case 'q':
			Instruction->OperandCount = 0;
			break;
		}

		// MOVS case
		if (Instruction->OperandCount > 0)
		{
			// Default to the normal MOV
			strcpy(mnemonic, "mov");
			
			// See if there's any explicit operands to convert
			InstOperand *operands = Instruction->Operands;

			if (operands[0].Type == OPERAND_MEM && operands[1].Type == OPERAND_MEM)
			{
				if ((operands[0].Mem.BaseVal == REG_RDI && operands[1].Mem.BaseVal == REG_RSI) ||
					(operands[0].Mem.BaseVal == REG_EDI && operands[1].Mem.BaseVal == REG_ESI))
				{
					switch (operands[0].Size)
					{
					case SIZE_BYTE:		strcpy(mnemonic, "movsb");	break;
					case SIZE_WORD:		strcpy(mnemonic, "movsw");	break;
					case SIZE_DWORD:	strcpy(mnemonic, "movsd");	break;
					case SIZE_QWORD:	strcpy(mnemonic, "movsq");	break;
					}

					Instruction->OperandCount = 0;
				}
			}
		}
	}


	return _strupr(mnemonic);
}