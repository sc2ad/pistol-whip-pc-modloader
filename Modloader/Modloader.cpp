// Modloader.cpp : Defines the exported functions for the DLL.
//

#define _VERBOSE 1 // For logging

//////////////////////////////// TODO REMOVE
#ifndef X64
#define X64
#endif
////////////////////////////////

#include "pch.h"
#include "framework.h"
#include "Modloader.h"
#include "logger.h"
#include "assert_util.h"

//void* (*il2cpp_string_new_orig)(const char* text);
static uint64_t il2cpp_string_new_orig_offset;
void* (*il2cpp_string_new_orig)(const char* text);

void* test_il2cpp_string_new(const char* text) {
	//init_logger(); // again
	LOG("Hello from il2cpp_string_new! - Creating text with name: %s\n", text);
	//free_logger(); // again
	auto tmp = PLH::FnCast(il2cpp_string_new_orig_offset, il2cpp_string_new_orig)(text);
	//auto tmp = il2cpp_string_new_orig(text);
	LOG("Created string with pointer: %p\n", tmp);
	return tmp;
}

// This function is used to load in all of the mods into the main assembly.
// It will construct them, create a hook into il2cpp_init to use
// and load the mods in il2cpp_init.
MODLOADER_API int load(void)
{
	init_logger();

	if (!CreateDirectory(L"Mods/", NULL) && ERROR_ALREADY_EXISTS != GetLastError())
	{
		LOG("FAILED TO CREATE MODS DIRECTORY!\n");
		MessageBoxW(NULL, L"Could not create Mods/ directory!", L"Could not create Mods directory!'",
			MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_TOPMOST | MB_SETFOREGROUND);
		free_logger();
		ExitProcess(GetLastError());
	}

	WIN32_FIND_DATAW findDataAssemb;
	LOG("Attempting to find GameAssembly.dll\n");
	HANDLE findHandleAssemb = FindFirstFileW(L"GameAssembly.dll", &findDataAssemb);
	if (findHandleAssemb == INVALID_HANDLE_VALUE)
	{
		LOG("FAILED TO FIND GameAssembly.dll!\n");
		free_logger();
		MessageBoxW(NULL, L"Could not locate game being injected!", L"Could not find GameAssembly.dll'",
			MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_TOPMOST | MB_SETFOREGROUND);
		ExitProcess(GetLastError());
		return 0;
	}
	auto gameassemb = LoadLibraryA("GameAssembly.dll");
	ASSERT(gameassemb, L"GameAssembly.dll failed to load!");
	auto attemptProc = GetProcAddress(gameassemb, "il2cpp_string_new");
	auto base = (uint64_t)gameassemb;
	LOG("GameAssembly.dll base: %x\n", base);
	LOG("il2cpp_string_new proc address: %p\n", attemptProc);
	LOG("il2cpp_string_new RVA: %x\n", (uint64_t)attemptProc - base);
	LOG("Attempting to patch il2cpp_string_new...\n");

	PLH::CapstoneDisassembler dis(PLH::Mode::x64);
	PLH::x64Detour detour((uint64_t)attemptProc, (uint64_t)test_il2cpp_string_new, &il2cpp_string_new_orig_offset, dis);
	detour.hook();

	//il2cpp_string_new_orig = patch_raw(gameassemb, (DWORD)attemptProc - base, test_il2cpp_string_new, 0x40);
	LOG("Patched il2cpp_string_new!\n");

	WIN32_FIND_DATAW findData;
	LOG("Attempting to find all files that match: 'Mods/*.dll'\n");
	HANDLE findHandle = FindFirstFileW(L"Mods/*.dll", &findData);

	if (findHandle == INVALID_HANDLE_VALUE)
	{
		LOG("FAILED TO FIND ANY MODS TO LOAD!\n");
		//free_logger();
		return 0;
	}


	do
	{
		if (findData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) {
			// Maybe we can just call LoadLibrary on this file, and GetProcAddress of load and call that?
			// LOG!
			LOG("%ls: Loading\n", findData.cFileName);
			auto lib = LoadLibrary(findData.cFileName);
			if (!lib) {
				LOG("%ls: Failed to find library!\n", findData.cFileName);
				continue;
			}
			// TODO move this to il2cpp_init hook
			auto loadCall = GetProcAddress(lib, "load");
			if (!loadCall) {
				LOG("%ls: Failed to find load call!\n", findData.cFileName);
				continue;
			}
			LOG("%ls: Calling 'load' function!\n", findData.cFileName);
			loadCall();
			LOG("%ls: Loaded!\n", findData.cFileName);
		}
	} while (FindNextFileW(findHandle, &findData) != 0);

	LOG("Loaded all mods!\n");

	free_logger();
	return 0;
}
