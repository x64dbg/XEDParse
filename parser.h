#ifndef _PARSER_H
#define _PARSER_H

#include "XEDParse.h"
#include "OpSize.h"
#include "RegTable.h"
#include "Operand.h"
#include "Prefix.h"

struct Inst
{
    char				Mnemonic[32];
    xed_iclass_enum_t	Class;
    PREFIX				Prefix;

	bool				Near;
	bool				Far;

    int					OperandCount;
    InstOperand			Operands[4];
};

#include "MnemonicTable.h"
#include "BranchInst.h"
#include "Validator.h"

//functions
bool valfromstring(const char* text, ULONGLONG* value);
bool StrDel(char *Src, char *Needle, char StopAt);
char *GrabInstToken(char *Dest, char *Src, bool Operand);
int InstructionToTokens(const char *Value, char Tokens[8][64]);
bool ParseInstString(XEDPARSE* Parse, Inst *Instruction);

#endif // _PARSER_H
