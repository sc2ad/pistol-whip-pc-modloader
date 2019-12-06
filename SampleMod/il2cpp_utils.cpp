#include "il2cpp_utils.hpp"
#include <sstream>
#include "logger.h"

// Many implementaions from https://github.com/sc2ad/pistol-whip-hook/blob/fd7edc3d1d39d231e43c1430dbf4336045a056cc/shared/utils/il2cpp-utils.cpp
namespace il2cpp_utils {
	using namespace std;
	const MethodInfo* GetMethod(Il2CppClass* klass, std::string_view methodName, int argsCount) {
		if (!klass) return nullptr;
		auto methodInfo = il2cpp_functions::class_get_method_from_name(klass, methodName.data(), argsCount);
		if (!methodInfo) {
			LOG("ERROR: could not find method %s with %i parameters in class %s (namespace '%s')!", methodName.data(),
				argsCount, il2cpp_functions::class_get_name(klass), il2cpp_functions::class_get_namespace(klass));
			return nullptr;
		}
		return methodInfo;
	}

	const MethodInfo* GetMethod(std::string_view nameSpace, std::string_view className, std::string_view methodName, int argsCount) {
		return GetMethod(GetClassFromName(nameSpace.data(), className.data()), methodName, argsCount);
	}

	Il2CppClass* GetClassFromName(const char* name_space, const char* type_name) {

		auto dom = il2cpp_functions::domain_get();
		if (!dom) {
			LOG("ERROR: GetClassFromName: Could not get domain!");
			return nullptr;
		}
		size_t assemb_count;
		const Il2CppAssembly** allAssemb = il2cpp_functions::domain_get_assemblies(dom, &assemb_count);

		for (int i = 0; i < assemb_count; i++) {
			auto assemb = allAssemb[i];
			auto img = il2cpp_functions::assembly_get_image(assemb);
			if (!img) {
				LOG("ERROR: Assembly with name: %s has a null image!", assemb->aname.name);
				continue;
			}
			auto klass = il2cpp_functions::class_from_name(img, name_space, type_name);
			if (klass) {
				return klass;
			}
		}
		LOG("ERROR: il2cpp_utils: GetClassFromName: Could not find class with namepace: %s and name: %s", name_space, type_name);
		return nullptr;
	}
	
}

