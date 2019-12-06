#pragma once
#include "il2cpp_functions.hpp"
#include <string_view>
namespace il2cpp_utils
{
	const MethodInfo* GetMethod(Il2CppClass* klass, std::string_view methodName, int argsCount);
	const MethodInfo* GetMethod(std::string_view nameSpace, std::string_view className, std::string_view methodName, int argsCount);
	Il2CppClass* GetClassFromName(const char* name_space, const char* type_name);
};

