#ifndef __MAIN_H__
#define __MAIN_H__

#include <windows.h>

//XEDParse defines
#ifdef XEDPARSE_BUILD
#define XEDPARSE_EXPORT __declspec(dllexport)
#else
#define XEDPARSE_EXPORT __declspec(dllimport)
#endif //XEDPARSE_BUILD

#define XEDPARSE_CALL //calling convention

#define XEDPARSE_MAXBUFSIZE 256
#define XEDPARSE_MAXASMSIZE 16


//XEDParse enums
enum XEDPARSE_STATUS
{
    XEDPARSE_ERROR=0,
    XEDPARSE_OK=1
};

//XEDParse structs
struct XEDPARSE
{
    ULONG_PTR cip; //instruction pointer (for relative addressing)
    int dest_size; //destination size (returned by XEDParse)
    unsigned char dest[XEDPARSE_MAXASMSIZE]; //destination buffer
    char instr[XEDPARSE_MAXBUFSIZE]; //instruction text
    char error[XEDPARSE_MAXBUFSIZE]; //error text (in case of an error)
};

#ifdef __cplusplus
extern "C"
{
#endif

XEDPARSE_EXPORT XEDPARSE_STATUS XEDPARSE_CALL XEDParseAssemble(XEDPARSE* XEDParse);

#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__
