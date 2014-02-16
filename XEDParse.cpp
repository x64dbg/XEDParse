#include "XEDParse.h"
#include "xed2\include\xed-interface.h"
#include "parser.h"
#include "parsetest.h"
#include "translate.h"
#include "xed-ex3.h"
#include <stdio.h>

XEDPARSE_EXPORT XEDPARSE_STATUS XEDPARSE_CALL XEDParseAssemble(XEDPARSE* XEDParse)
{
    //for testing
    if(!xed_ex3(XEDParse, XEDParse->instr))
        printf("error: %s\n", XEDParse->error);

    INSTRUCTION instr;
    memset(&instr, 0, sizeof(instr));
    if(!parse(XEDParse, &instr))
    {
        printf("error: %s\n", XEDParse->error);
        return XEDPARSE_ERROR;
    }
    char recode[XEDPARSE_MAXBUFSIZE]="";
    parsedisasm(&instr, recode); //convert parsed instruction back to text
    strcpy((char*)XEDParse->dest, recode);
    puts(recode); //print recoded instruction
    TRANSLATION translation;
    if(!translate(XEDParse, &instr, &translation))
    {
        printf("error: %s\n", XEDParse->error);
        return XEDPARSE_ERROR;
    }
    return XEDPARSE_OK;
}

extern "C" __declspec(dllexport) BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch(fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}
