#include "parser.h"
#include <stdio.h>

static PREFIX isprefix(const char* text, int* i)
{
    if(!_strnicmp(text, "lock ", 5))
    {
        *i+=5;
        return PREFIX_LOCK;
    }
    else if(!_strnicmp(text, "rep ", 4))
    {
        *i+=4;
        return PREFIX_REP;
    }
    else if(!_strnicmp(text, "repe ", 5) || !_strnicmp(text, "repz ", 5))
    {
        *i+=5;
        return PREFIX_REPEZ;
    }
    else if(!_strnicmp(text, "repne ", 6) || !_strnicmp(text, "repnz ", 6))
    {
        *i+=6;
        return PREFIX_REPNEZ;
    }
    return PREFIX_NONE;
}

bool parse(XEDPARSE* raw, INSTRUCTION* parsed)
{
    if(!raw || !parsed)
        return false;
    int len=strlen(raw->instr);
    if(!len)
        return false;
    //skip initial spaces
    int skip=0;
    while(raw->instr[skip]==' ' && skip<len)
        skip++;
    //get prefix
    parsed->prefix=isprefix(raw->instr+skip, &skip);
    len=strlen(raw->instr+skip);
    if(!len)
        return false;
    if(!sscanf(raw->instr+skip, "%s", parsed->mnemonic))
        return false;
    while(raw->instr[skip]!=' ' && skip<len)
        skip++;
    char instr[XEDPARSE_MAXBUFSIZE]="";
    strcpy(instr, raw->instr+skip+1);
    len=strlen(instr);
    if(!len)
        return true; //we have a mnemonic already
    int commacount=0;
    for(int i=0; i<len; i++)
        if(instr[i]==',')
            commacount++;
    if(commacount>2) //invalid instruction
        return false;
    if(commacount) //two operands
    {
        char* operand2=strstr(instr, ","); //find comma
        *operand2=0;
        operand2++;
        strcpy(parsed->operand2.raw, operand2);
    }
    strcpy(parsed->operand1.raw, instr);
    parsed->operand1.type=TYPE_NONE;
    parsed->operand2.type=TYPE_NONE;
    return true;
}
