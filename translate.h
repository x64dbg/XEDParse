#ifndef _TRANSLATE_H
#define _TRANSLATE_H

#include "XEDParse.h"
#include "parser.h"

//structures
struct TRANSLATION
{
    char instr[XEDPARSE_MAXBUFSIZE];
};

bool translate(XEDPARSE* XEDParse, INSTRUCTION* instruction, TRANSLATION* translation);

#endif
