#include "Translator.h"

// This maps an ICLASS to all of its IFORMs
IClassType XedInstLookupTable[XED_ICLASS_LAST];

int ResizeOperandImmediate(InstOperand* Operand, xed_iclass_enum_t IClass, int FixedSize, int LargestSize)
{
    // Branches are excluded
    if(IClassIsBranch(IClass))
        return FixedSize;

    // The value size might need adjustment if the sign bit is set,
    // but check if it can be supported
    if(FixedSize < LargestSize)
    {
        OPSIZE bitSize = OpsizeFromBits(FixedSize);
        OPSIZE newSize = PromoteImmediateWidth(Operand->Imm.Signed, Operand->Imm.imm, bitSize);
        int newBits    = OpsizeToBits(newSize);

        // The newer size might exceed LargestSize, so limit it
        return min(newBits, LargestSize);
    }

    return FixedSize;
}

bool ResizeSingleOperand(XEDPARSE* Parse, xed_iclass_enum_t IClass, InstOperand* Operand)
{
    IClassType* type = &XedInstLookupTable[IClass];

    switch(Operand->Type)
    {
    case OPERAND_REG:
        // Registers can't be resized
        return true;

    case OPERAND_IMM:
    {
        // Find the smallest instruction imm size that fits the input.
        // Set an arbitrarily high value for fixedSize.
        int targetSize  = OpsizeToBits(Operand->Size);
        int largestSize = 0;
        int fixedSize   = INT_MAX;

        for(int i = 0; i < type->InstructionCount; i++)
        {
            const xed_inst_t* inst  = type->Instructions[i];
            const xed_operand_t* op = xed_inst_operand(inst, 0);

            if(!xed_operand_type_is_immediate(xed_operand_type(op)))
                continue;

            int size = xed_operand_width_bits(op, Operand->XedEOSZ);

            if(size >= targetSize && size <= fixedSize)
                fixedSize = size;

            largestSize = max(largestSize, size);
        }

        if(fixedSize == INT_MAX)
        {
            strcpy(Parse->error, "Immediate size is too large");
            return false;
        }

        fixedSize     = ResizeOperandImmediate(Operand, IClass, fixedSize, largestSize);
        Operand->Size = OpsizeFromBits(fixedSize);
        return true;
    }
    break;

    case OPERAND_MEM:
    {
        // If the memory size is set, there's nothing to do
        if(Operand->Size != SIZE_UNSET)
            return true;

        // Going off of GCC's 'AS.exe' assembler
        // https://defuse.ca/online-x86-assembler.htm#disassembly
        //
        // This defaults to the full size, which is all 32 or all 64 bits,
        // if the instruction has multiple memory types -> AMBIGUOUS
        unsigned int memoryOperandCount = 0;
        int memoryOperandSize  = 0;

        for(int i = 0; i < type->InstructionCount; i++)
        {
            const xed_inst_t* inst  = type->Instructions[i];
            const xed_operand_t* op = xed_inst_operand(inst, 0);
            xed_operand_enum_t name = xed_operand_name(op);

            if(!xed_operand_is_memory_addressing(name))
                continue;

            // Some instructions (FSAVE, FXSAVE, FNSAVE) have different bit
            // sizes such as 752 or 864. Anything > 512 bits is skipped after
            // the first entry.
            int size = (int)xed_operand_width_bits(op, Operand->XedEOSZ);

            // Is it a non-standard operand size?
            if(size > OpsizeToBits(SIZE_ZMMWORD) || memoryOperandSize > OpsizeToBits(SIZE_ZMMWORD))
                memoryOperandCount = 0;

            memoryOperandCount++;
            memoryOperandSize = max(memoryOperandSize, size);
        }

        // Check if ambiguous
        if(memoryOperandCount > 1)
        {
            strcpy(Parse->error, "Ambiguous memory size");
            return false;
        }

        Operand->Size    = OpsizeFromBits(memoryOperandSize);
        Operand->BitSize = memoryOperandSize;
        return true;
    }
    break;
    }

    return false;
}

bool ResizeDoubleOperands(XEDPARSE* Parse, xed_iclass_enum_t IClass, InstOperand* Operands)
{
    IClassType* type = &XedInstLookupTable[IClass];

    const xed_inst_t* inst;
    const xed_operand_t* xedOp[2];

    switch(Operands[0].Type)
    {
    case OPERAND_REG:
        switch(Operands[1].Type)
        {
        // INSTRUCTION REG, IMM
        case OPERAND_IMM:
        {
            int targetSize  = OpsizeToBits(Operands[1].Size);
            int largestSize = 0;
            int fixedSize   = INT_MAX;

            for(int i = 0; i < type->InstructionCount; i++)
            {
                inst     = type->Instructions[i];
                xedOp[0] = xed_inst_operand(inst, 0);
                xedOp[1] = xed_inst_operand(inst, 1);

                // Match the register first
                if(!xed_operand_template_is_register(xedOp[0]))
                    continue;

                if(xed_operand_width_bits(xedOp[0], Operands[0].XedEOSZ) != OpsizeToBits(Operands[0].Size))
                    continue;

                // Check compatible immediate types
                if(!xed_operand_type_is_immediate(xed_operand_type(xedOp[1])))
                    continue;

                int size = xed_operand_width_bits(xedOp[1], Operands[1].XedEOSZ);

                if(size >= targetSize && size <= fixedSize)
                    fixedSize = size;

                largestSize = max(largestSize, size);
            }

            if(fixedSize == INT_MAX)
            {
                strcpy(Parse->error, "Immediate size is too large");
                return false;
            }

            fixedSize        = ResizeOperandImmediate(&Operands[1], IClass, fixedSize, largestSize);
            Operands[1].Size = OpsizeFromBits(fixedSize);
            return true;
        }
        break;

        // INSTRUCTION REG, MEM
        case OPERAND_MEM:
        {
            //
            // See above: ResizeSingleOperand(), case OPERAND_MEM
            //
            unsigned int memoryOperandCount = 0;
            unsigned int memoryOperandSize  = 0;

            for(int i = 0; i < type->InstructionCount; i++)
            {
                inst     = type->Instructions[i];
                xedOp[0] = xed_inst_operand(inst, 0);
                xedOp[1] = xed_inst_operand(inst, 1);

                // Match the register first
                if(!xed_operand_template_is_register(xedOp[0]))
                    continue;

                if(xed_operand_width_bits(xedOp[0], Operands[0].XedEOSZ) != OpsizeToBits(Operands[0].Size))
                    continue;

                // Check compatible memory types
                if(!xed_operand_is_memory_addressing(xed_operand_name(xedOp[1])))
                    continue;

                // Count if there is more than one possible operand size
                unsigned int size = xed_operand_width_bits(xedOp[1], Operands[1].XedEOSZ);

                if(size != memoryOperandSize)
                    memoryOperandCount++;

                memoryOperandSize = max(memoryOperandSize, size);
            }

            // Check if ambiguous
            if(memoryOperandCount > 1)
            {
                strcpy(Parse->error, "Ambiguous memory size");
                return false;
            }

            Operands[1].Size    = OpsizeFromBits(memoryOperandSize);
            Operands[1].BitSize = memoryOperandSize;
            return true;
        }
        break;
        }

    case OPERAND_MEM:
        switch(Operands[1].Type)
        {
        // INSTRUCTION MEM, REG
        case OPERAND_REG:
        {
            //
            // See above: ResizeSingleOperand(), case OPERAND_MEM
            //
            unsigned int memoryOperandCount = 0;
            unsigned int memoryOperandSize  = 0;

            for(int i = 0; i < type->InstructionCount; i++)
            {
                inst     = type->Instructions[i];
                xedOp[0] = xed_inst_operand(inst, 0);
                xedOp[1] = xed_inst_operand(inst, 1);

                // Match the register first
                if(!xed_operand_template_is_register(xedOp[1]))
                    continue;

                if(xed_operand_width_bits(xedOp[1], Operands[1].XedEOSZ) != OpsizeToBits(Operands[1].Size))
                    continue;

                // Check compatible memory types
                if(!xed_operand_is_memory_addressing(xed_operand_name(xedOp[0])))
                    continue;

                // Count if there is more than one possible operand size
                unsigned int size = xed_operand_width_bits(xedOp[0], Operands[0].XedEOSZ);

                if(size != memoryOperandSize)
                    memoryOperandCount++;

                memoryOperandSize = max(memoryOperandSize, size);
            }

            // Check if ambiguous
            if(memoryOperandCount > 1)
            {
                strcpy(Parse->error, "Ambiguous memory size");
                return false;
            }

            Operands[0].Size    = OpsizeFromBits(memoryOperandSize);
            Operands[0].BitSize = memoryOperandSize;
            return true;
        }
        break;

        // INSTRUCTION MEM, IMM
        case OPERAND_IMM:
        {
            //
            // This is a special case:
            // Both the memory and immediate operands need to be resized if needed
            // First get the smallest IMM possible, then also store MEM types
            //
            int memoryOperandCount = 0;
            int memoryOperandSize  = 0;

            int targetSize  = OpsizeToBits(Operands[1].Size);
            int fixedSize   = INT_MAX;

            for(int i = 0; i < type->InstructionCount; i++)
            {
                inst     = type->Instructions[i];
                xedOp[0] = xed_inst_operand(inst, 0);
                xedOp[1] = xed_inst_operand(inst, 1);

                // Match memory types
                if(!xed_operand_is_memory_addressing(xed_operand_name(xedOp[0])))
                    continue;

                // Match immediate types
                if(!xed_operand_type_is_immediate(xed_operand_type(xedOp[1])))
                    continue;

                int memSize = xed_operand_width_bits(xedOp[0], Operands[0].XedEOSZ);
                int immSize = xed_operand_width_bits(xedOp[1], Operands[1].XedEOSZ);

                // Skip sizes smaller than the initial immediate
                if(immSize < targetSize)
                    continue;

                if(Operands[0].Size == SIZE_UNSET)
                {
                    // Count if there is more than one possible operand size
                    if(memSize != memoryOperandSize)
                        memoryOperandCount++;

                    memoryOperandSize = max(memoryOperandSize, memSize);
                }
                else
                {
                    // Memory size was set by user, skip anything smaller
                    if(memSize < (int)OpsizeToBits(Operands[0].Size))
                        continue;
                }

                if(immSize <= fixedSize)
                    fixedSize = immSize;
            }

            // Only updated if the memory was SIZE_UNSET
            if(Operands[0].Size == SIZE_UNSET)
            {
                // Check if ambiguous
                if(memoryOperandCount > 1)
                {
                    strcpy(Parse->error, "Ambiguous memory size");
                    return false;
                }

                Operands[0].Size    = OpsizeFromBits(memoryOperandSize);
                Operands[0].BitSize = memoryOperandSize;
            }

            if(fixedSize == INT_MAX)
            {
                strcpy(Parse->error, "Immediate size is too large");
                return false;
            }

            Operands[1].Size    = OpsizeFromBits(fixedSize);
            Operands[1].BitSize = fixedSize;
            return true;
        }
        break;
        }
        break;
    }

    return true;
}

bool ValidateInstOperands(XEDPARSE* Parse, Inst* Instruction)
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
    //  return false;

    //
    // Instructions with no operands do not apply here
    //
    if(Instruction->OperandCount <= 0)
        return true;

    //
    // Instructions with a single operand are "easy" to solve
    // Simply match the [imm/mem] size with an IFORM
    //
    if(Instruction->OperandCount <= 1)
        return ResizeSingleOperand(Parse, Instruction->Class, &Instruction->Operands[0]);

    //
    // Special case for LEA (Segments can't be used)
    //
    if(Instruction->Class == XED_ICLASS_LEA)
        Instruction->Operands[1].Segment = REG_INVALID;

    //
    // Special case for XCHG (Swap memory operand)
    //
    if(Instruction->Class == XED_ICLASS_XCHG)
    {
        if(Instruction->Operands[1].Type == OPERAND_MEM)
        {
            InstOperand save;

            memcpy(&save, &Instruction->Operands[0], sizeof(InstOperand));
            memcpy(&Instruction->Operands[0], &Instruction->Operands[1], sizeof(InstOperand));
            memcpy(&Instruction->Operands[1], &save, sizeof(InstOperand));
        }
    }

    //
    // Check all invalid cases and then pass it to ResizeDoubleOperands
    //
    switch(Instruction->Operands[0].Type)
    {
    case OPERAND_REG:
        switch(Instruction->Operands[1].Type)
        {
        // INSTRUCTION REG, REG
        case OPERAND_REG:
            // Registers can't be resized
            return true;

        // INSTRUCTION REG, IMM
        case OPERAND_IMM:
            // Use EOSZ of the register
            Instruction->Operands[1].XedEOSZ = Instruction->Operands[0].XedEOSZ;

            return ResizeDoubleOperands(Parse, Instruction->Class, Instruction->Operands);

        // INSTRUCTION REG, []
        case OPERAND_MEM:
            // Don't care if size is already set
            if(Instruction->Operands[1].Size != SIZE_UNSET)
                return true;

            // Use EOSZ of the register
            Instruction->Operands[1].XedEOSZ = Instruction->Operands[0].XedEOSZ;

            return ResizeDoubleOperands(Parse, Instruction->Class, Instruction->Operands);
        }
        break;

    case OPERAND_IMM:
        if(Instruction->Class == XED_ICLASS_OUT)
        {
            if(Instruction->Operands[1].Type == OPERAND_REG)
            {
                // Example: OUT 0xE9, EAX
                return ResizeSingleOperand(Parse, Instruction->Class, Instruction->Operands);
            }
        }

        strcpy(Parse->error, "Operand immediate mismatch");
        return false;

    case OPERAND_MEM:
        switch(Instruction->Operands[1].Type)
        {
        // INSTRUCTION [], REG
        case OPERAND_REG:
            // Don't care if size is already set
            if(Instruction->Operands[0].Size != SIZE_UNSET)
                return true;

            // Use EOSZ of the register
            Instruction->Operands[0].XedEOSZ = Instruction->Operands[1].XedEOSZ;

            return ResizeDoubleOperands(Parse, Instruction->Class, Instruction->Operands);

        // INSTRUCTION [], IMM
        case OPERAND_IMM:
            // Use EOSZ of the memory
            Instruction->Operands[1].XedEOSZ = Instruction->Operands[0].XedEOSZ;

            return ResizeDoubleOperands(Parse, Instruction->Class, Instruction->Operands);

        // INSTRUCTION [], []
        case OPERAND_MEM:
            strcpy(Parse->error, "Too many memory references");
            return false;
        }
        break;

    case OPERAND_SEGSEL:
        return true;
    }

    return false;
}

void LookupTableInit()
{
    static bool bInitialized = false;

    if(bInitialized)
        return;

    // Initialize Xed's internal state
    xed_tables_init();

    // Invalidate initial data
    for(int i = 0; i < XED_ICLASS_LAST; i++)
    {
        IClassType* type        = &XedInstLookupTable[i];
        type->IClass            = XED_ICLASS_INVALID;
        type->MinimumOperands   = INT_MAX;
        type->InstructionCount  = 0;
    }

    // Query XED
    for(int i = 0; i < XED_MAX_INST_TABLE_NODES; i++)
    {
        const xed_inst_t* inst      = &xed_inst_table_base()[i];
        xed_iclass_enum_t iclass    = xed_inst_iclass(inst);

        // Set the basic information/update the pointers
        IClassType* type        = &XedInstLookupTable[iclass];
        type->IClass            = iclass;
        type->MinimumOperands   = min(xed_inst_noperands(inst), type->MinimumOperands);

        type->Instructions[type->InstructionCount++] = inst;
    }

    bInitialized = true;
}
