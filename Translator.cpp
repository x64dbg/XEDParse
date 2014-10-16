#include "Translator.h"

LONGLONG TranslateRelativeCip(XEDPARSE* Parse, ULONGLONG Value, bool Signed)
{
    if(Signed)
        return (LONGLONG)((LONGLONG)Value - Parse->cip);

    return (LONGLONG)((ULONGLONG)Value - Parse->cip);
}

xed_encoder_operand_t OperandToXed(InstOperand* Operand)
{
    xed_encoder_operand_t o;
    memset(&o, 0, sizeof(xed_encoder_operand_t));

    // Handle special cases for certain memory operations
    // Examples: FXSAVE [], FLDENV []
    int bitsize = (Operand->Size == SIZE_UNSET) ? Operand->BitSize : opsizetobits(Operand->Size);

    // Use the high level encoder interface
    switch(Operand->Type)
    {
    case OPERAND_REG:
        return xed_reg(Operand->Reg.XedReg);

    case OPERAND_IMM:
        if(Operand->Imm.RelBranch)
            return xed_relbr(Operand->Imm.simm, bitsize);
        else
            return xed_imm0(Operand->Imm.imm, bitsize);

    case OPERAND_MEM:
        // See xed_mem_bisd @ xed-encoder-hl.h
        o.type      = XED_ENCODER_OPERAND_TYPE_MEM;
        o.u.mem.seg = segtoxed(Operand->Segment);
        o.width     = bitsize;

        if(Operand->Mem.Base)
            o.u.mem.base = regtoxed(Operand->Mem.BaseVal);

        if(Operand->Mem.Index)
            o.u.mem.index = regtoxed(Operand->Mem.IndexVal);

        if(Operand->Mem.Scale)
            o.u.mem.scale = Operand->Mem.ScaleVal;

        if(Operand->Mem.Disp)
        {
            o.u.mem.disp.displacement       = Operand->Mem.DispVal;
            o.u.mem.disp.displacement_width = opsizetobits(Operand->Mem.DispWidth);
        }

        return o;

    case OPERAND_SEGSEL:
        return xed_ptr(Operand->Sel.Offset, bitsize);
    }

    o.type = XED_ENCODER_OPERAND_TYPE_INVALID;
    return o;
}

void ConvertInstToXed(Inst* Instruction, xed_state_t Mode, xed_encoder_instruction_t* XedInst, unsigned int EffectiveWidth)
{
    // Convert the operands to XED's form first
    xed_encoder_operand_t ops[4];

    for(int i = 0; i < Instruction->OperandCount; i++)
        ops[i] = OperandToXed(&Instruction->Operands[i]);

    // Create the instruction
    xed_inst(XedInst, Mode, Instruction->Class, EffectiveWidth, Instruction->OperandCount, ops);

    // Apply any prefixes
    switch(Instruction->Prefix)
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

bool TryEncode(XEDPARSE* Parse, xed_state_t State, Inst* Instruction, unsigned int EffectiveWidth)
{
    // Convert this struct to XED's format
    xed_encoder_instruction_t xedInst;
    memset(&xedInst, 0, sizeof(xed_encoder_instruction_t));
    ConvertInstToXed(Instruction, State, &xedInst, EffectiveWidth);

    // Conversion request -> encoder request
    xed_encoder_request_t encReq;
    xed_encoder_request_zero_set_mode(&encReq, &State);

    if(!xed_convert_to_encoder_request(&encReq, &xedInst))
    {
        strcpy(Parse->error, "Failed while converting encoder request");
        return false;
    }

    // Finally encode the assembly
    xed_error_enum_t err = xed_encode(&encReq, Parse->dest, XED_MAX_INSTRUCTION_BYTES, &Parse->dest_size);

    if(err != XED_ERROR_NONE)
    {
        strcpy(Parse->error, "Failed to encode instruction!");
        return false;
    }

    return true;
}

bool Translate(XEDPARSE* Parse, xed_state_t State, Inst* Instruction)
{
    // Jumps are IP-based and need to be fixed
    if(!TranslateBranchInst(Parse, Instruction))
        return false;

    // Fix up any operand sizes
    if(!ValidateInstOperands(Parse, Instruction))
        return false;

    // Try encoding with various different effectiveWidth values
    if(TryEncode(Parse, State, Instruction, 32))
        return true;

    // Fix RIP-relative commands
    for(int i = 0; i < Instruction->OperandCount; i++)
    {
        if(Instruction->Operands[i].Type != OPERAND_MEM)
            continue;

        if(!Instruction->Operands[i].Mem.DispRipRelative)
            continue;

        Instruction->Operands[i].Mem.DispVal--;
    }

    if(Parse->x64 && TryEncode(Parse, State, Instruction, 64))
        return true;

    if(TryEncode(Parse, State, Instruction, 16))
        return true;

    return false;
}
