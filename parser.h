#ifndef _PARSER_H
#define _PARSER_H

#include "XEDParse.h"

//enums
enum PREFIX
{
    PREFIX_NONE,
    PREFIX_LOCK,
    PREFIX_REP,
    PREFIX_REPEZ,
    PREFIX_REPNEZ
};

//structures
struct OPERAND
{
    char raw[XEDPARSE_MAXBUFSIZE/2];
};


struct INSTRUCTION
{
    PREFIX prefix;
    char mnemonic[10];
    OPERAND operand1;
    OPERAND operand2;
};

//functions
bool parse(XEDPARSE* raw, INSTRUCTION* parsed);

#endif // _PARSER_H
