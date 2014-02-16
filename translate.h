#ifndef _TRANSLATE_H
#define _TRANSLATE_H

#include "XEDParse.h"
#include "parser.h"

//structures
struct TRANSOP //translating operand
{
    OPERAND* operand;
    int memop; //number of memory operands
    int regop; //number of register operands
    int operand_index; //operand index
};

bool translate(XEDPARSE* XEDParse, INSTRUCTION* instruction);

#endif
