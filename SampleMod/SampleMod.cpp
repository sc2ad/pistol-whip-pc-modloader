// SampleMod.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "SampleMod.h"
#include "logger.h"
#include "assert_util.h"

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

SAMPLEMOD_API int load(HMODULE gameAssembly) {
	init_logger("MyMod");
	// Install hooks onto gameAssembly here
	auto attemptProc = GetProcAddress(gameAssembly, "il2cpp_string_new");
	auto base = (uint64_t)gameAssembly;
	LOG("GameAssembly.dll base: %lx\n", base);
	LOG("il2cpp_string_new proc address: %p\n", attemptProc);
	LOG("il2cpp_string_new RVA: %lx\n", (uint64_t)attemptProc - base);
	LOG("Attempting to patch il2cpp_string_new...\n");

	PLH::CapstoneDisassembler dis(PLH::Mode::x64);
	PLH::x64Detour detour((uint64_t)attemptProc, (uint64_t)test_il2cpp_string_new, &il2cpp_string_new_orig_offset, dis);
	detour.hook();
	// Close logger to flush to file
	free_logger();
	return 0;
}
