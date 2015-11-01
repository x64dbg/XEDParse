#include <stdio.h>
#include "Translator.h"

LONGLONG TranslateRelativeCip(XEDPARSE* Parse, ULONGLONG Value, bool Signed)
{
    if(Signed)
        return (LONGLONG)((LONGLONG)Value - Parse->cip);

    return (LONGLONG)((ULONGLONG)Value - Parse->cip);
}

const char* XedErrorToString(xed_error_enum_t Error)
{
    switch(Error)
    {
    case XED_ERROR_BUFFER_TOO_SHORT:
        return "There were not enough bytes in the given buffer.";
    case XED_ERROR_GENERAL_ERROR:
        return "XED could not encode the given instruction.";
    case XED_ERROR_INVALID_FOR_CHIP:
        return "The instruciton is not valid for the specified chip.";
    case XED_ERROR_BAD_REGISTER:
        return "XED could not decode the given instruction because an invalid register encoding was used.";
    case XED_ERROR_BAD_LOCK_PREFIX:
        return "A lock prefix was found where none is allowed.";
    case XED_ERROR_BAD_REP_PREFIX:
        return "An F2 or F3 prefix was found where none is allowed.";
    case XED_ERROR_BAD_LEGACY_PREFIX:
        return "A 66, F2 or F3 prefix was found where none is allowed.";
    case XED_ERROR_BAD_REX_PREFIX:
        return "A REX prefix was found where none is allowed.";
    case XED_ERROR_BAD_EVEX_UBIT:
        return "An illegal value for the EVEX.U bit was present in the instruction.";
    case XED_ERROR_BAD_MAP:
        return "An illegal value for the MAP field was detected in the instruction.";
    case XED_ERROR_NO_OUTPUT_POINTER:
        return "The output pointer for xed_agen was zero";
    case XED_ERROR_NO_AGEN_CALL_BACK_REGISTERED:
        return "One or both of the callbacks for xed_agen were missing.";
    case XED_ERROR_BAD_MEMOP_INDEX:
        return "Memop indices must be 0 or 1.";
    case XED_ERROR_CALLBACK_PROBLEM:
        return "The register or segment callback for xed_agen experienced a problem";
    case XED_ERROR_GATHER_REGS:
        return "The index, dest and mask regs for AVX2 gathers must be different.";
    case XED_ERROR_INSTR_TOO_LONG:
        return "Full decode of instruction would exeed 15 bytes.";
    }

    // XED_ERROR_NONE
    return "There was no error.";
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

bool TryEncode(XEDPARSE* Parse, xed_state_t State, Inst* Instruction, unsigned int EffectiveWidth)
{
    // Convert this struct to XED's format
    xed_encoder_instruction_t xedInst;
    memset(&xedInst, 0, sizeof(xed_encoder_instruction_t));

    // XEDParse instruction -> Xed instruction
    InstructionToXed(Instruction, State, &xedInst, EffectiveWidth);

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
        const char* fullError = XedErrorToString(err);

        if(fullError)
            sprintf(Parse->error, "Failed to encode instruction! %s", fullError);
        else
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

    // Try encoding with various different effective width values
    // 32-bit
    if(TryEncode(Parse, State, Instruction, 32))
        return true;

    // 64-bit
    if(Parse->x64 && TryEncode(Parse, State, Instruction, 64))
        return true;

    // 16-bit
    if(TryEncode(Parse, State, Instruction, 16))
        return true;

    return false;
}
