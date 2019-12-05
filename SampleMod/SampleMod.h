// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the SAMPLEMOD_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// SAMPLEMOD_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef SAMPLEMOD_EXPORTS
#define SAMPLEMOD_API __declspec(dllexport)
#else
#define SAMPLEMOD_API __declspec(dllimport)
#endif

#include "../pistol-whip-hook/PolyHook_2_0/headers/CapstoneDisassembler.hpp"
#include "../pistol-whip-hook/PolyHook_2_0/headers/IHook.hpp"
#include "../pistol-whip-hook/PolyHook_2_0/headers/Detour/x64Detour.hpp"
#include "../pistol-whip-hook/PolyHook_2_0/headers/Detour/x86Detour.hpp"
#include "../pistol-whip-hook/PolyHook_2_0/headers/Instruction.hpp"
#include "../pistol-whip-hook/PolyHook_2_0/headers/Detour/ADetour.hpp"
#include "../pistol-whip-hook/PolyHook_2_0/headers/Enums.hpp"

extern "C" SAMPLEMOD_API int load(HANDLE logHandle, HMODULE gameAssembly);
