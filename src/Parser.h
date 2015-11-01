#ifndef _PARSER_H
#define _PARSER_H

#include "XEDParse.h"
#include "OpSize.h"
#include "Register.h"
#include "Operand.h"
#include "MemOperand.h"
#include "Prefix.h"

struct Inst
{
    char                Mnemonic[32];
    xed_iclass_enum_t   Class;
    PREFIX              Prefix;

    bool                Near;           // Near modifier  (near ptr)
    bool                Far;            // Far modifier   (far ptr)
    bool                Short;          // Short modifier (jump short)

    int                 OperandCount;
    InstOperand         Operands[4];

    int AddressSizeOverride;            // Required to be set for instructions with implicit suppressed memops.
    // 0, 16, 32, or 64
};

#include "MnemonicTable.h"
#include "BranchInst.h"
#include "Validator.h"

//functions
bool valfromstring(const char* text, ULONGLONG* value);
bool StrDel(char* Src, char* Needle, char StopAt);
char* GrabInstToken(char* Dest, char* Src, bool Operand);
int InstructionToTokens(const char* Value, char Tokens[8][64]);
bool ParseInstString(XEDPARSE* Parse, Inst* Instruction);

#endif // _PARSER_H
