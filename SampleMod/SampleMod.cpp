// SampleMod.cpp : Defines the exported functions for the DLL.
//

#include "pch.h"
#include "framework.h"
#include "SampleMod.h"
#include "logger.h"
#include "il2cpp_functions.hpp"
#include "il2cpp_utils.hpp"


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


static uint64_t gunfire_tramp = NULL;
void (*gunfire)(void* self);
__declspec(noinline) void h_gunfire(void* self)
{
	LOG("Gun::Fire() hook called");
	return PLH::FnCast(gunfire_tramp, gunfire)(self);
}

static uint64_t gunreload_tramp = NULL;
void (*gunreload)(void* self, bool ignoreBulletWaste);
__declspec(noinline) void h_gunreload(void* self, bool ignoreBulletWaste)
{
	LOG("Gun::Reload() hook called");
	return PLH::FnCast(gunreload_tramp, gunreload)(self, ignoreBulletWaste);
}

void dumpErrorLog()
{
	std::string s;
	do
	{
		s = PLH::ErrorLog::singleton().pop().msg;
		LOG(s.c_str());
	} while (!s.empty());
}

NOINLINE void gunfireCallback(const PLH::ILCallback::Parameters* p, const uint8_t count, const PLH::ILCallback::ReturnValue* retVal)
{
	LOG("Gunfire callback called!");
}


SAMPLEMOD_API int load(HANDLE logHandle, HMODULE gameAssembly) {
	init_logger(logHandle);
	il2cpp_functions::Init(gameAssembly);
	LOG("Beginning load!\n");
	// Install hooks onto gameAssembly here
	auto attemptProc = GetProcAddress(gameAssembly, "il2cpp_string_new");
	auto base = (uint64_t)gameAssembly;
	LOG("GameAssembly.dll base: %lx\n", base);
	auto gunfire_ptr = il2cpp_utils::GetMethod("", "Gun", "Fire", 0)->methodPointer;
	LOG("Gun::Fire() proc address: %p\n", gunfire_ptr);
	LOG("Attempting to patch gunfire...\n");

	PLH::CapstoneDisassembler dis(PLH::Mode::x64);
	PLH::ILCallback callback;
	asmjit::FuncSignatureT<void, void*> gunfireSig;
	gunfireSig.setCallConv(asmjit::CallConv::kIdX86Win64);
	uint64_t jit = callback.getJitFunc(gunfireSig, &gunfireCallback);




	PLH::x64Detour gunfireDetour((uint64_t)il2cpp_utils::GetMethod("", "Gun", "Fire", 0)->methodPointer, (uint64_t)jit, callback.getTrampolineHolder(), dis);
	if (gunfireDetour.hook() == false)
	{
		LOG("Failed to hook gun::fire()\n");
		dumpErrorLog();
	}
	//PLH::x64Detour gunreloadDetour((uint64_t)il2cpp_utils::GetMethod("", "Gun", "Reload", 1)->methodPointer, (uint64_t)h_gunreload, &gunreload_tramp, dis);
	//
	//if (gunreloadDetour.hook() == false)
	//{
	//	LOG("Failed to hook gun::reload()\n");
	//	dumpErrorLog();
	//}

	LOG("Installed hooks!!\n");
	// Close logger to flush to file
	//free_logger();
	return 0;
}
