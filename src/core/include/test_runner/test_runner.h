#pragma once


#include <flecs.h>

#include <string>
#include <functional>
#include <optional>
#include <map>
#include <stdexcept>
#include <algorithm>

#include <test_runner/common.h>

class TestRunner {
public:
	using WorldCallback = TestRunnerDetail::WorldCallback;
	using LogLevel = TestRunnerDetail::LogLevel;
	using Log = TestRunnerDetail::Log;
	using TypeRegistry = TestRunnerDetail::TypeRegistry;
	
	using ModulesMap = std::map<std::string, WorldCallback>;

	// ================================================================================================
	struct Error : public TestRunnerDetail::AutoPrefixedError<Error> {
		using AutoPrefixedError::AutoPrefixedError;
	};

	// Module tag
	// ================================================================================================
	struct TestableModule {};
	
	TestRunner(flecs::world& world);

	// TODO: must be called after modules are registered?
	template <typename... Ts>
	static void registerTypes(flecs::world& world) { // flecs::world& world
		if (_world != world) {
			throw Error("Is not the same world as passed during construction");
		}

		_typeRegistry.add<Ts...>();
		_typeRegistry.applyAll(world);
	}

	// Can pass modules to register if want
	template <typename... Ts>
	static void initialize(flecs::world& world) {
		_world = world;
		
		if (!_initialized) {
			// Register test runner itself if not registered
			world.import<TestRunner>();

			// Register primitive types
			// TODO: remove?
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
  
  static void setLogLevel(LogLevel logLevel);

protected:
	template <typename T>
	static void registerModule(flecs::world& world) {
		// Import module to make available for query
		auto m = world.import<T>();
		m.add<TestableModule>();

		std::string name = TestRunnerDetail::getTypeName<T>();

		Log::trace() << "Registered module \"" << name << "\"";
		// Store in map to allow for later import in test world
		_moduleImporterRegistry[name] = [](flecs::world& w) {
			w.import<T>();
			};
	}

	template <typename... Ts>
	static void registerModules(flecs::world& world) {
		(registerModule<Ts>(world), ...);
	}
private:
	friend class TestRunnerImpl;

	inline static flecs::world _world;

	/**
	* Maps module name to importer function.
	*/
	inline static ModulesMap _moduleImporterRegistry; // TODO: use function object instead of func ptr?

	inline static TypeRegistry _typeRegistry;

	inline static bool _initialized = false;
};

