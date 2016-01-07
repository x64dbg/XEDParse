#pragma once

bool HandleMemoryOperand(XEDPARSE* Parse, const char* Value, InstOperand* Operand);
void SetMemoryDisplacementOrBase(XEDPARSE* Parse, const char* Value, InstOperand* Operand);
void SetMemoryIndexOrScale(XEDPARSE* Parse, const char* Value, InstOperand* Operand);