#include <windows.h>

#define EXIT_FAILURE -1

#define ASSERT(test, message)                    \
	if(!(test))                                  \
	{                                            \
		MessageBox(NULL, message, L"Modloader: Fatal", MB_OK | MB_ICONERROR); \
		ExitProcess(EXIT_FAILURE); \
	}

#define ASSERT_SOFT(test, ...)                   \
	if(!(test))                                  \
	{                                            \
		return __VA_ARGS__;                      \
	}