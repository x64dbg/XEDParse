#ifndef __MAIN_H__
#define __MAIN_H__

#include <windows.h>

/*  To use this exported function of dll, include this header
 *  in your project.
 */

#ifdef XEDPARSE_BUILD
#define XEDPARSE_EXPORT __declspec(dllexport)
#else
#define XEDPARSE_EXPORT __declspec(dllimport)
#endif //XEDPARSE_BUILD


#ifdef __cplusplus
extern "C"
{
#endif


#ifdef __cplusplus
}
#endif

#endif // __MAIN_H__
