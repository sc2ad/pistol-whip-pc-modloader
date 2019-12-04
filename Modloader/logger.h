#ifdef _VERBOSE
#include <windows.h>
static HANDLE log_handle;
char buffer[4096];

inline void init_logger()
{
	log_handle = CreateFileA("modloader.log", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
		NULL);
}

inline void free_logger()
{
	CloseHandle(log_handle);
}

#define LOG(message, ...) \
	{ \
		size_t len = wsprintfA(buffer, message, __VA_ARGS__); \
		WriteFile(log_handle, buffer, len, NULL, NULL); \
		FlushFileBuffers(log_handle); \
	}
#else
inline void init_logger()
{
}

inline void free_logger()
{
}

#define LOG(message, ...) 
#endif