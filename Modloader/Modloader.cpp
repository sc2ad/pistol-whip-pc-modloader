// Modloader.cpp : Defines the exported functions for the DLL.
//

#define _VERBOSE 1 // For logging

////////////////////////////////
#ifndef X64
#define X64
#endif
////////////////////////////////

#include "pch.h"
#include "framework.h"
#include "Modloader.h"
#include "crt.h"
#include "logger.h"
#include "assert_util.h"
#include "relocation.h"

template<typename Ptr = void*>
Ptr rva_to_pointer(HMODULE module, size_t rva) {
	IMAGE_DOS_HEADER* mz = (PIMAGE_DOS_HEADER)module;
	IMAGE_NT_HEADERS* nt = RVA2PTR(PIMAGE_NT_HEADERS, mz, mz->e_lfanew);
	return reinterpret_cast<Ptr>(nt->OptionalHeader.ImageBase + rva);
}

// returns address of "original"
// copy and allocate half a page lmao (0x0800)
void* patch_module_func(HMODULE module, size_t offset, void const* target_fn, bool copy = false, size_t copy_size = 0x0800) {
	//MSG_F(L"ImageBase: %#x\r\nBaseOfCode: %#x", nt->OptionalHeader.ImageBase, nt->OptionalHeader.BaseOfCode);

	void* target_patch = rva_to_pointer(module, offset);

	// 15 here is the max x86 instruction size
	constexpr auto protect_size = 15;

	//MSG_F(L"Module base: 0x%p\r\nRVA: %#lx\r\nReal Target: 0x%p\r\nRedirected Target: 0x%p\r\n", module, rva, target_patch, target_fn);
	DWORD oldState;
	ASSERT(VirtualProtect(target_patch, protect_size, PAGE_READWRITE, &oldState), L"Could not change protection of target: error code 0x%x", GetLastError());
	void* old_mem = nullptr;
	LOG("About to attempt to copy pointer: %p", target_patch);
	if (copy) {
		DWORD oldState2;
		old_mem = memalloc(copy_size);
		ASSERT(old_mem, L"Could not allocate memory for copy!");
		ASSERT(VirtualProtect(old_mem, copy_size, PAGE_EXECUTE_READWRITE, &oldState2), L"Could not change protection of copy: error code 0x%x", GetLastError());
		memcpy(old_mem, target_patch, copy_size);
		copy_relocate(module, target_patch, old_mem, copy_size);
		LOG("Relocated memory: %p", old_mem);
	}

#ifdef X32
	// x86 cannot jump to an absolute address with one instruction, so we load into eax and jump
	char target_mem[] = { 0xb8, 0,0,0,0, 0xff, 0xe0 }; // mov eax,<addr>; jmp eax
	*reinterpret_cast<void const**>(target_mem + 1) = target_fn;
#elif defined(X64)
	// x64 has no jump with a 64 bit operand, but we can index into rip so the next "instruction" will be the address
	// this might utterly fuck performance
	char target_mem[] = { 0xff, 0x25, 0,0,0,0, 0,0,0,0,0,0,0,0 }; // jmp qword ptr [rip]; <address>
	*reinterpret_cast<void const**>(target_mem + 6) = target_fn;
#endif
	LOG("About to memcpy target_mem to target_patch");
	memcpy(target_patch, target_mem, sizeof(target_mem));

	ASSERT(VirtualProtect(target_patch, protect_size, oldState, &oldState), L"Could not revert protection of target: error code 0x%x", GetLastError());

	LOG("Resultant old_mem pointer: %p", old_mem);
	return old_mem;
}

// returns old pointer
void const* patch_il2cpp_meta_pointer(HMODULE module, size_t rva, void const* target_fn) {
	auto target_patch = rva_to_pointer<void const**>(module, rva);

	DWORD oldState;
	ASSERT(VirtualProtect(target_patch, sizeof(void*), PAGE_READWRITE, &oldState), L"Could not change protection of target: error code 0x%x", GetLastError());

	auto old_ptr = *target_patch;
	*target_patch = target_fn;

	ASSERT(VirtualProtect(target_patch, sizeof(void*), oldState, &oldState), L"Could not revert protection of target: error code 0x%x", GetLastError());

	return old_ptr;
}

template<typename Ptr>
Ptr patch_il2cpp_meta_pointer_as(HMODULE module, size_t rva, Ptr target_fn) {
	return reinterpret_cast<Ptr>(patch_il2cpp_meta_pointer(module, rva, reinterpret_cast<void const*>(target_fn)));
}

template<typename Ptr>
Ptr patch_raw(HMODULE module, size_t offset, Ptr target_fn, size_t copy_size = 0x0800) {
	return reinterpret_cast<Ptr>(patch_module_func(module, offset, reinterpret_cast<void const*>(target_fn), true, copy_size));
}

void* (*il2cpp_string_new_orig)(const char* text);

void* test_il2cpp_string_new(const char* text) {
	//init_logger(); // again
	LOG("Hello from il2cpp_string_new! - Creating text with name: %s\n", text);
	//free_logger(); // again
	auto tmp = il2cpp_string_new_orig(text);
	LOG("Created string with pointer: %p\n", tmp);
	return tmp;
}

// This function is used to load in all of the mods into the main assembly.
// It will construct them, create a hook into il2cpp_init to use
// and load the mods in il2cpp_init.
MODLOADER_API int load(void)
{
	hHeap = GetProcessHeap();
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
	auto base = (DWORD)gameassemb;
	LOG("GameAssembly.dll base: %x\n", base);
	LOG("il2cpp_string_new proc address: %p\n", attemptProc);
	LOG("il2cpp_string_new RVA: %x\n", (DWORD)attemptProc - base);
	LOG("Attempting to patch il2cpp_string_new...\n");

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
		/*MessageBoxW(NULL, L"Could not locate game being injected!", L"No files found in current directory matching 'Mods/*.dll'",
			MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_TOPMOST | MB_SETFOREGROUND);
		free_logger();
		ExitProcess(GetLastError());*/
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
