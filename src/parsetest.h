#ifndef _PARSETEST_H
#define _PARSETEST_H

#include "Parser.h"

void OperandToString(char* Buffer, InstOperand* Operand);
void InstructionToString(char* Buffer, Inst* Instruction);
#endif
