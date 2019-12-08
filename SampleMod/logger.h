#ifndef LOGGING_H
#define LOGGING_H
#ifndef NO_LOGGING
#include <windows.h>
static HANDLE log_handle;
static char buffer[4096];

inline void init_logger(HANDLE handle)
{
	log_handle = handle;
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
#endif