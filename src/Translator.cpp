#include <stdio.h>
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

    // Handle special memory operations with non-standard sizes
    // Examples: FXSAVE [], FLDENV []
    int bitsize = (Operand->Size == SIZE_UNSET) ? Operand->BitSize : OpsizeToBits(Operand->Size);

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
        o.u.mem.seg = RegToXed(Operand->Segment);
        o.width     = bitsize;

        if(Operand->Mem.Base)
            o.u.mem.base = RegToXed(Operand->Mem.BaseVal);

        if(Operand->Mem.Index)
            o.u.mem.index = RegToXed(Operand->Mem.IndexVal);

        if(Operand->Mem.Scale)
            o.u.mem.scale = Operand->Mem.ScaleVal;

        if(Operand->Mem.Disp)
        {
            o.u.mem.disp.displacement       = Operand->Mem.DispVal;
            o.u.mem.disp.displacement_width = OpsizeToBits(Operand->Mem.DispWidth);
        }

        return o;

    case OPERAND_SEGSEL:
        return xed_ptr(Operand->Sel.Offset, bitsize);
    }

    o.type = XED_ENCODER_OPERAND_TYPE_INVALID;
    return o;
}

void InstructionToXed(Inst* Instruction, xed_state_t Mode, xed_encoder_instruction_t* XedInst, unsigned int EffectiveWidth)
{
    // Convert the operands to XED's form first
    xed_encoder_operand_t ops[4];

    for(int i = 0; i < Instruction->OperandCount; i++)
        ops[i] = OperandToXed(&Instruction->Operands[i]);

    // Create the instruction
    xed_inst(XedInst, Mode, Instruction->Class, EffectiveWidth, Instruction->OperandCount, ops);

    // Apply prefixes
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

    // Apply address size override if necessary
    if(Instruction->AddressSizeOverride)
        xed_addr(XedInst, Instruction->AddressSizeOverride);
}

bool TryEncode(XEDPARSE* Parse, xed_state_t State, Inst* Instruction, unsigned int Bits)
{
    // Convert this struct to XED's format
    xed_encoder_instruction_t xedInst;
    memset(&xedInst, 0, sizeof(xed_encoder_instruction_t));

    // XEDParse instruction -> Xed instruction
    InstructionToXed(Instruction, State, &xedInst, Bits);

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

bool TryRecode(XEDPARSE* Parse, xed_state_t State, Inst* Instruction, unsigned int Bits)
{
    // First round, try regular encoding
    if(!TryEncode(Parse, State, Instruction, Bits))
        return false;

    // 32-bit code returns immediately
    if(!Parse->x64)
        return true;

    // Check if any operands are RIP-relative memory references
    bool reEncode = false;

    for(int i = 0; i < Instruction->OperandCount; i++)
    {
        auto operand = &Instruction->Operands[i];

        if(operand->Type != OPERAND_MEM)
            continue;

        if(!operand->Mem.ImplicitRip)
            continue;

        // Adjust displacement based on the instruction length
        operand->Mem.DispVal -= Parse->dest_size;
        reEncode = true;

        // Encoding will fail if displacement is larger than 32 bits
        if(abs((LONGLONG)operand->Mem.DispVal) > ULONG_MAX)
            return false;
    }

    // Operands were modified
    if(reEncode)
        return TryEncode(Parse, State, Instruction, Bits);

    // Nothing was changed
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

    // Try encoding with various different effective width values
    // 32-bit
    if(TryRecode(Parse, State, Instruction, 32))
        return true;

    // 64-bit
    if(TryRecode(Parse, State, Instruction, 64))
        return true;

    // 16-bit
    if(TryRecode(Parse, State, Instruction, 16))
        return true;

    return false;
}
