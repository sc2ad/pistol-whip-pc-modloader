#ifndef NO_LOGGING
#include <windows.h>
static HANDLE log_handle;
char buffer[4096];

#define PATH_LENGTH 240

inline HANDLE make_logger(const wchar_t* name) {
	wchar_t path[PATH_LENGTH] = { 0 };
	wsprintf(path, L"Logs/%ls.log", name);
	return CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}

inline void init_logger(const wchar_t* name)
{
	log_handle = make_logger(name);
}

inline void free_logger()
{
	CloseHandle(log_handle);
}

#define LOG(message, ...) \
	{ \
		size_t len = wsprintfA(buffer, message, __VA_ARGS__); \
		strcat_s(buffer, "\n"); \
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