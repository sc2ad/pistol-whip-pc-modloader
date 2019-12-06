#include "pch.h"
#include "il2cpp_functions.hpp"
#include <windows.h>
#include <libloaderapi2.h>
#include <sstream>
#include "logger.h"
#include <strsafe.h>


void il2cpp_functions::Init()
{
	if (!initialized)
	{
		initialized = true;
	//*(void**)(&class_get_field_from_name) = (void*)0x00007FFA6BD40B90;
		HMODULE module = NULL;
		if ((module = GetModuleHandle(TEXT("gameassembly.dll"))) == NULL)
		{
			LOG("Failed to get handle for gameassembly.dll");
			exit(0);
		}

		*(void**)(&class_get_field_from_name) = GetProcAddress(module,"il2cpp_class_get_field_from_name");
		*(void**)(&field_get_value) = GetProcAddress(module, "il2cpp_field_get_value");
		*(void**)(&class_get_method_from_name) = GetProcAddress(module, "il2cpp_class_get_method_from_name");
		*(void**)(&class_from_name) = GetProcAddress(module, "il2cpp_class_from_name");
		*(void**)(&domain_get_assemblies) = GetProcAddress(module, "il2cpp_domain_get_assemblies");
		*(void**)(&domain_get) = GetProcAddress(module, "il2cpp_domain_get");
		*(void**)(&assembly_get_image) = GetProcAddress(module, "il2cpp_assembly_get_image");

	}
}

