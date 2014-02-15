#include "XEDParse.h"
#include "xed2\include\xed-interface.h"
#include "parser.h"
#include <stdio.h>

XEDPARSE_EXPORT XEDPARSE_STATUS XEDPARSE_CALL XEDParseAssemble(XEDPARSE* XEDParse)
{
    INSTRUCTION instr;
    memset(&instr, 0, sizeof(instr));
    if(!parse(XEDParse, &instr))
    {
        strcpy(XEDParse->error, "parsing failed...");
        return XEDPARSE_ERROR;
    }
    printf("%d \"%s\" \"%s\",\"%s\"\n", (int)instr.prefix, instr.mnemonic, instr.operand1.raw, instr.operand2.raw);

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
