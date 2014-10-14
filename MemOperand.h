#pragma once

void SetMemoryDisplacementOrBase(XEDPARSE* Parse, const char* Value, InstOperand* Operand);
void SetMemoryIndexOrScale(XEDPARSE* Parse, const char* Value, InstOperand* Operand);
bool HandleMemoryOperand(XEDPARSE* Parse, const char* Value, InstOperand* Operand);