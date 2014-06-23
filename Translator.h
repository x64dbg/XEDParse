#ifndef _TRANSLATE_H
#define _TRANSLATE_H

#include "XEDParse.h"
#include "Parser.h"

char *TranslateInstMnemonic(XEDPARSE *Parse, Inst *Instruction);
LONGLONG TranslateRelativeCip(XEDPARSE *Parse, ULONGLONG Value, bool Signed);
xed_encoder_operand_t OperandToXed(InstOperand *Operand);
void ConvertInstToXed(Inst *Instruction, xed_state_t Mode, xed_encoder_instruction_t *XedInst);
bool Translate(XEDPARSE* Parse, xed_state_t State, Inst *Instruction);
#endif
