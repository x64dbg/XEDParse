#include "Translator.h"

char *TranslateInstMnemonic(XEDPARSE *Parse, Inst *Instruction)
{
	char *mnemonic = Instruction->Mnemonic;

	// Convert the name to XED format
	strcpy(mnemonic, MnemonicToXed(mnemonic));

	/*
	TODO:

	BEXTR_XOP
	CMPSD_XMM
	MOVSD_XMM
	PEXTRW_SSE4
	PREFETCH_EXCLUSIVE
	PREFETCH_RESERVED
	*/

	// Special cases depending on the operands
	for (int i = 0; i < Instruction->OperandCount; i++)
	{
		InstOperand *operand = &Instruction->Operands[i];

		if (!_stricmp(mnemonic, "mov"))
		{
			if (operand->Type == OPERAND_REG)
			{
				if (IsControlRegister(operand->Reg.Reg))
					strcpy(mnemonic, "mov_cr");
				else if (IsDebugRegister(operand->Reg.Reg))
					strcpy(mnemonic, "mov_dr");
			}
		}
	}

	return _strupr(mnemonic);
}

LONGLONG TranslateRelativeCip(XEDPARSE *Parse, ULONGLONG Value, bool Signed)
{
	if (Signed)
		return (LONGLONG)((LONGLONG)Value - Parse->cip);

	return (LONGLONG)((ULONGLONG)Value - Parse->cip);
}

xed_encoder_operand_t OperandToXed(InstOperand *Operand)
{
	xed_encoder_operand_t o;
	memset(&o, 0, sizeof(xed_encoder_operand_t));

	switch (Operand->Type)
	{
	case OPERAND_REG:
		return xed_reg(Operand->Reg.XedReg);

	case OPERAND_IMM:
		if (Operand->Imm.RelBranch)
			return xed_relbr(Operand->Imm.simm, opsizetobits(Operand->Size));
		else
			return xed_imm0(Operand->Imm.imm, opsizetobits(Operand->Size));

	case OPERAND_MEM:
		// See xed_mem_bisd @ xed-encoder-hl.h
		o.type		= XED_ENCODER_OPERAND_TYPE_MEM;
		o.u.mem.seg = segtoxed(Operand->Segment);
		o.width		= opsizetobits(Operand->Size);

		if (Operand->Mem.Base)
			o.u.mem.base = regtoxed(Operand->Mem.BaseVal);

		if (Operand->Mem.Index)
			o.u.mem.index = regtoxed(Operand->Mem.IndexVal);

		if (Operand->Mem.Scale)
			o.u.mem.scale = Operand->Mem.ScaleVal;

		if (Operand->Mem.Disp)
		{
			o.u.mem.disp.displacement		= Operand->Mem.DispVal;
			o.u.mem.disp.displacement_width = opsizetobits(Operand->Mem.DispWidth);
		}

		return o;
	}

	o.type = XED_ENCODER_OPERAND_TYPE_INVALID;
	return o;
}

void ConvertInstToXed(Inst *Instruction, xed_state_t Mode, xed_encoder_instruction_t *XedInst)
{
	// Convert the operands to XED's form first
	xed_encoder_operand_t ops[4];

	int effectiveWidth = 0;
	if (Instruction->OperandCount > 0)
	{
		for (int i = 0; i < Instruction->OperandCount; i++)
		{
			if (Instruction->Operands[i].Type != OPERAND_MEM)
				effectiveWidth = max(effectiveWidth, opsizetobits(Instruction->Operands[i].Size));

			ops[i] = OperandToXed(&Instruction->Operands[i]);
		}
	}
	else
	{
		if (Mode.mmode == XED_MACHINE_MODE_LONG_64)
			effectiveWidth = 64;
		else
			effectiveWidth = 32;
	}

	// Create the instruction
	xed_inst(XedInst, Mode, Instruction->Class, effectiveWidth, Instruction->OperandCount, ops);

	// Apply any prefixes
	switch (Instruction->Prefix)
	{
	case PREFIX_LOCK:
		xed_lock(XedInst);
		break;

	case PREFIX_REP:
		xed_rep(XedInst);
		break;

	case PREFIX_REPNEZ:
		xed_repne(XedInst);
		break;
	}
}

bool Translate(XEDPARSE* Parse, xed_state_t State, Inst *Instruction)
{
	// Jumps are IP-based and need to be fixed
	if (!TranslateBranchInst(Parse, Instruction))
		return false;

	// Fix up any operand sizes
	if (!ValidateInstOperands(Parse, Instruction))
		return false;

	// Convert this struct to XED's format
	xed_encoder_instruction_t xedInst;
	ConvertInstToXed(Instruction, State, &xedInst);

	// Conversion request -> encoder request
	xed_encoder_request_t encReq;
	xed_encoder_request_zero_set_mode(&encReq, &State);

	if (!xed_convert_to_encoder_request(&encReq, &xedInst))
	{
		strcpy(Parse->error, "Failed while converting encoder request");
		return false;
	}

	// Output the request
	//char buf[5000] = "";
	//xed_encode_request_print(&encReq, buf, 5000);
	//printf(buf);

	// Finally encode the assembly
	xed_error_enum_t err = xed_encode(&encReq, Parse->dest, XED_MAX_INSTRUCTION_BYTES, &Parse->dest_size);

	if (err != XED_ERROR_NONE)
	{
		strcpy(Parse->error, "Failed to encode instruction!");
		return false;
	}
	
	return true;
}
