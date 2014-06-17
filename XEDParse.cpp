#include "XEDParse.h"
#include "xed2\include\xed-interface.h"
#include "Parser.h"
#include "Translator.h"
#include "ParseTest.h"

XEDPARSE_EXPORT XEDPARSE_STATUS XEDPARSE_CALL XEDParseAssemble(XEDPARSE* XEDParse)
{
	Inst instruction;
	memset(&instruction, 0, sizeof(Inst));

	if (!ParseInstString(XEDParse, &instruction))
	{
		printf("%s\n", XEDParse->error);
		return XEDPARSE_ERROR;
	}

	xed_state_t state;
#ifdef _WIN64
	state.mmode = XED_MACHINE_MODE_LONG_64;
#else
	state.mmode = XED_MACHINE_MODE_LEGACY_32;
#endif //_WIN64
	state.stack_addr_width = XED_ADDRESS_WIDTH_32b;

	if (!Translate(XEDParse, state, &instruction))
	{
		printf("%s\n", XEDParse->error);
		return XEDPARSE_ERROR;
	}

	InstructionToString(&instruction);
	
	return XEDPARSE_OK;
}

extern "C" __declspec(dllexport) BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch(fdwReason)
    {
    case DLL_PROCESS_ATTACH:
		// Initialize all of the tables only once
		// This is mainly independent to everything else
		LookupTableInit();
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
