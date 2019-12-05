// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the MODLOADER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// MODLOADER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MODLOADER_EXPORTS
#define MODLOADER_API __declspec(dllexport)
#else
#define MODLOADER_API __declspec(dllimport)
#endif

template<typename TRet, typename ...TArgs>
// A generic function pointer
using function_ptr_t = TRet(*)(TArgs...);

extern "C" MODLOADER_API int load(void);
