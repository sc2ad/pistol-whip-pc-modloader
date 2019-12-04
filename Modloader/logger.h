#ifndef NO_LOGGING
#include <windows.h>
static HANDLE log_handle;
char buffer[4096];

#define PATH_MAX

inline void init_logger(const char* name)
{
	char nameWithLog[PATH_MAX + 4];
	strcpy_s(nameWithLog, name);
	strcat_s(nameWithLog, ".log");
	log_handle = CreateFileA(nameWithLog, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
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