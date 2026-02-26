#pragma once


#include <flecs.h>

#include <string>
#include <functional>
#include <optional>
#include <map>
#include <stdexcept>
#include <algorithm>

#include "common.h"

//class TypeRegistry;

class TestRunner {
public:
	using WorldCallback = TestRunnerDetail::WorldCallback;
	using LogLevel = TestRunnerDetail::LogLevel;
	using TypeRegistry = TestRunnerDetail::TypeRegistry;

	//using WorldCallback = void(*)(flecs::world&);
	using ModulesMap =
		std::map<std::string, WorldCallback>;

	// Module tag
	// ================================================================================================
	struct TestableModule {
	};

	
	TestRunner(flecs::world& world);

	// TODO: must be called after modules are registered?
	template <typename... Ts>
	static void registerTypes(flecs::world& world) { // flecs::world& world
		_typeRegistry.add<Ts...>();
		_typeRegistry.applyAll(world);
	}

	template <typename T>
	static void registerModule(flecs::world& world) {
		//initialize(world);

		// Import module to make available for query
		auto m = world.import<T>();
		m.add<TestableModule>();

		std::string name = getTypeName<T>();

		std::cout << "Registered module: " << name << std::endl;
		// Store in map to allow for later import in test world
		_moduleImporterRegistry[name] = [](flecs::world& w) {
			w.import<T>();
		};
	}

	template <typename... Ts>
	static void registerModules(flecs::world& world) {
		(registerModule<Ts>(world), ...);
	}

	// Can pass modules to register if want
	template <typename... Ts>
	static void initialize(flecs::world& world) {
		if (!_initialized) {
			// Registered test runner itself if not registered
			world.import<TestRunner>();

			// Register primitive types
			registerTypes<
				bool, char,
				int8_t, int16_t, int32_t, int64_t,
				uint8_t, uint16_t, uint32_t, uint64_t,
				float, double
			>(world);

			_initialized = true;
		}

		registerModules<Ts...>(world);
	}
	//	if (_initialized) {
	//		return;
	//	}
	//	// Registered test runner itself if not registered
	//	world.import<TestRunner>();

	//	// Register primitive types
	//	registerTypes<
	//		bool, char,
	//		int8_t, int16_t, int32_t, int64_t,
	//		uint8_t, uint16_t, uint32_t, uint64_t,
	//		float, double
	//	>(world);

	//	_initialized = true;
	//}

  
  static void setLogLevel(LogLevel logLevel);

private:
	friend class TestRunnerImpl;

	/**
* Maps module name to importer function.
*/
	inline static ModulesMap _moduleImporterRegistry; // TODO: use function object instead of func ptr?

	//inline static TypeImporterMap _typeImporterRegistry;


	inline static TypeRegistry _typeRegistry;

	inline static bool _initialized = false;

  /**
  * Maps module name to importer function.
 // */
 // inline static ModulesMap _moduleImporterRegistry; // TODO: use function object instead of func ptr?

	////inline static TypeImporterMap _typeImporterRegistry;

	//
	//inline static TypeRegistry _typeRegistry;


	//inline static bool _initialized = false;
};

