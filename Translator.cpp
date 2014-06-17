#include "Translator.h"

char *TranslateInstMnemonic(Inst *Instruction)
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
	if (!_stricmp(mnemonic, "mov"))
	{
		for (int i = 0; i < 2; i++)
		{
			InstOperand *operand = &Instruction->Operands[i];

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

xed_encoder_operand_t OperandToXed(InstOperand *Operand)
{
	xed_encoder_operand_t o;
	memset(&o, 0, sizeof(xed_encoder_operand_t));

	switch (Operand->Type)
	{
	case OPERAND_REG:
		return xed_reg(Operand->Reg.XedReg);

	case OPERAND_IMM:
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

	for (int i = 0; i < Instruction->OperandCount; i++)
		ops[i] = OperandToXed(&Instruction->Operands[i]);

	// Create the instruction
	xed_inst(XedInst, Mode, Instruction->Class, 64, Instruction->OperandCount, ops);

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

bool Translate(XEDPARSE* XEDParse, xed_state_t State, Inst *instruction)
{
	// Fix up any operand sizes
	if (!ValidateInstOperands(XEDParse, instruction))
		return false;

	// Convert this struct to XED's format
	xed_encoder_instruction_t xedInst;
	ConvertInstToXed(instruction, State, &xedInst);

	// Conversion request -> encoder request
	xed_encoder_request_t encReq;
	xed_encoder_request_zero_set_mode(&encReq, &State);

	if (!xed_convert_to_encoder_request(&encReq, &xedInst))
	{
		strcpy(XEDParse->error, "Failed while converting encoder request");
		return false;
	}

	// Output the request
	char buf[5000] = "";
	xed_encode_request_print(&encReq, buf, 5000);
	printf(buf);

	// Finally encode the assembly
	xed_error_enum_t err = xed_encode(&encReq, XEDParse->dest, XED_MAX_INSTRUCTION_BYTES, &XEDParse->dest_size);

	if (err != XED_ERROR_NONE)
	{
		strcpy(XEDParse->error, "Failed to encode instruction!");
		return false;
	}
	
	return true;
}
