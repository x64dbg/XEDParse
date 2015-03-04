#pragma once

bool IClassIsJump(xed_iclass_enum_t IClass);
bool IClassIsCall(xed_iclass_enum_t IClass);
bool IClassIsBranch(xed_iclass_enum_t IClass);
int IClassBranchLength(xed_iclass_enum_t IClass, bool Imm8);
bool TranslateBranchInst(XEDPARSE* Parse, Inst* Instruction);