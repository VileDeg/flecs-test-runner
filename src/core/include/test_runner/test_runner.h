#pragma once


#include <flecs.h>

#include <string>
#include <functional>
#include <optional>
#include <map>
#include <stdexcept>
#include <algorithm>

#include <test_runner/common.h>

// ================================================================================================
class TestRunner {
public:
	using WorldCallback			 = TestRunnerDetail::WorldCallback;
	using LogLevel					 = TestRunnerDetail::LogLevel;
	using Log								 = TestRunnerDetail::Log;
	using SupportedOperators = TestRunnerDetail::SupportedOperators;
	using OperatorType			 = TestRunnerDetail::OperatorType;
	using ModuleImporterMap	 = TestRunnerDetail::ModuleImporterMap;

	// ==============================================================================================
	struct Error : public TestRunnerDetail::AutoPrefixedError<Error> {
		using AutoPrefixedError::AutoPrefixedError;
	};

	// ==============================================================================================
	/**
	 * Used to query the available modules in client
	 */
	struct TestableModule{};
	
	// ==============================================================================================
	// To register the module
	TestRunner(flecs::world& world);

	// ==============================================================================================
	// Can pass modules to register if want
	template <typename... Ts>
	static void initialize(flecs::world& world) {
		// Register test runner itself if not registered
		world.import<TestRunner>();
		registerModules<Ts...>(world);
	}
  
	// ==============================================================================================
	static void setLogLevel(LogLevel logLevel);

private:
	friend class TestRunnerImpl;

	// ==============================================================================================
	static SupportedOperators getSupportedOperators(const ecs_type_info_t& ti) {
		SupportedOperators result{};
		result.equals = ti.hooks.equals != NULL && !(ti.hooks.flags & ECS_TYPE_HOOK_EQUALS_ILLEGAL);
		result.cmp = ti.hooks.cmp != NULL && !(ti.hooks.flags & ECS_TYPE_HOOK_CMP_ILLEGAL);
		return result;
	}

	// ==============================================================================================
	/**
	 * Add metadata about supported operators for all components in module T.
	 */
	template <typename T>
	static void setSupportedOperators(flecs::world& world) {
		flecs::entity moduleEntity = world.module<T>();

		// Start deferring to avoid locked storage
		world.defer_begin();

		world.query_builder<>()
			.with<flecs::Component>() // Matches all component entities in the module
			.with(flecs::ChildOf, moduleEntity)
			.build()
			.each([&](flecs::entity e) {
					const ecs_type_info_t* ti = ecs_get_type_info(world, e);
					e.set<SupportedOperators>(getSupportedOperators(*ti));
				});

		// Execute all queued structural changes
		world.defer_end();
	}

	// ==============================================================================================
	template <typename T>
	static void registerModule(flecs::world& world) {
		// Import module to make available for query
		auto m = world.import<T>();
		m.add<TestableModule>();

		// Set for all contained components.
		// For primitive types assume all ops are supported.
		setSupportedOperators<T>(world);

		std::string name = TestRunnerDetail::getTypeName<T>();

		Log::trace() << "Registered module \"" << name << "\"";
		// Store in map to allow for later import in test world
		_moduleImporterRegistry[name] = [](flecs::world& w) {
				w.import<T>();
			};
	}

	// ==============================================================================================
	template <typename... Ts>
	static void registerModules(flecs::world& world) {
		(registerModule<Ts>(world), ...);
	}

	/**
	* Maps module name to importer function.
	*/
	inline static ModuleImporterMap _moduleImporterRegistry;
};

