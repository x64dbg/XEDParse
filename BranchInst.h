#pragma once

bool IsIClassJump(xed_iclass_enum_t IClass);
bool IsIClassCall(xed_iclass_enum_t IClass);
int BranchClassBytes(xed_iclass_enum_t IClass, bool Imm8);
bool TranslateBranchInst(XEDPARSE* Parse, Inst* Instruction);