#pragma once

#include <string>
#include <functional>
#include <optional>
#include <map>
#include <stdexcept>
#include <algorithm>

#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>

#include <flecs.h>

namespace TestRunnerDetail {

// This handles the difference between Windows (MSVC) and Linux/Mac (GCC/Clang)
#if defined(_MSC_VER) // Windows
#include <typeinfo>
template <typename T>
static std::string getTypeName() {
	std::string name = typeid(T).name();
	// MSVC returns "struct MyModule" or "class MyModule". 
	// We remove the prefix to get just "MyModule".
	size_t space_pos = name.find(' ');
	if (space_pos != std::string::npos) {
		return name.substr(space_pos + 1);
	}
	return name;
}
#else // Linux / macOS / GCC / Clang
#include <cxxabi.h>
#include <typeinfo>
#include <memory>
template <typename T>
static std::string getTypeName() {
	int status;
	// specific GCC/Clang API to "demangle" the weird internal names
	std::unique_ptr<char, void(*)(void*)> res{
			abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status),
			std::free
	};
	return (status == 0) ? res.get() : typeid(T).name();
}
#endif


template <typename Derived>
class AutoPrefixedError : public std::runtime_error {
public:
	explicit AutoPrefixedError(const std::string& message)
		: std::runtime_error(generatePrefix() + ": " + message) 
	{}

private:
	// Helper to get the nice "Namespace::Class" string
	static std::string generatePrefix() {
		return getTypeName<Derived>();
	}
};

using WorldCallback = std::function<void(flecs::world&)>;


struct TypeMetadata {
	using ComparisonFunc = bool (*)(const void*, const void*);

	struct ComparisonFuncs {
		ComparisonFunc eq = nullptr;
		ComparisonFunc neq = nullptr;
		ComparisonFunc lt = nullptr;
		ComparisonFunc lte = nullptr;
		ComparisonFunc gt = nullptr;
		ComparisonFunc gte = nullptr;

		friend bool operator==(const ComparisonFuncs& lhs, const ComparisonFuncs& rhs) {
			return
				lhs.eq == rhs.eq
				&& lhs.neq == rhs.neq
				&& lhs.lt == rhs.lt
				&& lhs.lte == rhs.lte
				&& lhs.gt == rhs.gt
				&& lhs.gte == rhs.gte
				;
		}
	};

	ComparisonFuncs funcs;
};

class TypeRegistry {
public:

	template <typename... Ts>
	void add() {

		// Use a fold expression to populate the vector
		(_registrars.push_back([](flecs::world& world) {
			registerTypeImpl<Ts>(world);
			}), ...);
	}

	const std::vector<WorldCallback>& getRegistrars() const {
		return _registrars;


		//return registrars;
	}

	void applyAll(flecs::world& world) const {
		std::for_each(_registrars.begin(), _registrars.end(), [&](const auto& f) { f(world); });
	}

private:
	template <typename T>
	static void registerTypeImpl(flecs::world& world) {
		// TODO: fisrt check if component is already registered
				// Maybe don't wanna register here
		// but should be already registered by imported module
		auto comp = world.component<T>();

		TypeMetadata meta{};

		if constexpr (has_eq_v<T>) {
			meta.funcs.eq =
				[](const void* a, const void* b) {
				return *static_cast<const T*>(a) == *static_cast<const T*>(b);
				};
		}
		if constexpr (has_neq_v<T>) {
			meta.funcs.neq =
				[](const void* a, const void* b) {
				return *static_cast<const T*>(a) != *static_cast<const T*>(b);
				};
		}
		if constexpr (has_lt_v<T>) {
			meta.funcs.lt =
				[](const void* a, const void* b) {
				return *static_cast<const T*>(a) < *static_cast<const T*>(b);
				};
		}
		if constexpr (has_lte_v<T>) {
			meta.funcs.lte =
				[](const void* a, const void* b) {
				return *static_cast<const T*>(a) <= *static_cast<const T*>(b);
				};
		}
		if constexpr (has_gt_v<T>) {
			meta.funcs.gt =
				[](const void* a, const void* b) {
				return *static_cast<const T*>(a) > *static_cast<const T*>(b);
				};
		}
		if constexpr (has_gte_v<T>) {
			meta.funcs.gte =
				[](const void* a, const void* b) {
				return *static_cast<const T*>(a) >= *static_cast<const T*>(b);
				};
		}

		comp.set<TypeMetadata>(meta);

		//} else {
			// Optional: Trigger a static_assert if you want to force 
			// the user to only call this on valid types.
			// static_assert(IsComparable<T>, "Type T does not support operator<");
		//}
	}

	std::vector<WorldCallback> _registrars;
};


enum class LogLevel {
	FATAL = 0,
	ERROR,
	WARN,
	INFO,
	TRACE,
};

}
