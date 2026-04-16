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
	// Can pass modules to register
	template <typename... Ts>
	static void initialize(flecs::world& world) {
		// Register test runner itself if not registered
		world.import<TestRunner>();
		registerModules<Ts...>(world);
	}

	// ==============================================================================================
	static int run(flecs::world world) {
		Log::info() << "Flecs Test Runner listening on port " << ECS_REST_DEFAULT_PORT << " ...";
		return world.app()
			.enable_rest()
			.enable_stats()
			.run();
	}

	// ==============================================================================================
	/**
	* Main function.
	*/
	template <typename... Ts>
	static int main(int argc = 0, char* argv[] = nullptr) {
		flecs::world world(argc, argv);
		initialize<Ts...>(world);
		return run(world);
	}

	template <typename... Ts>
	static void registerOperators(flecs::world world) {
		(registerOperatorsForComponent<Ts>(world), ...);
	}

	// ==============================================================================================
	template <typename T>
	static void registerOperatorsForComponent(flecs::world& world) {
		auto compEntity = world.component<T>();

		ecs_cmp_t cmpHandler = flecs::_::compare<T>();
		ecs_equals_t equalsHandler = flecs::_::equals<T>();

		auto opersComponent = compEntity.try_get<SupportedOperators>();
		SupportedOperators opers = opersComponent ? *opersComponent : SupportedOperators{};

		opers.cmp = opers.cmp ? opers.cmp : cmpHandler != NULL;
		opers.equals = opers.equals ? opers.equals : equalsHandler != NULL;

		compEntity.set<SupportedOperators>(opers);
		Log::trace() << "Registered operators for component " << TestRunnerDetail::getTypeName<T>();
	}

	// ==============================================================================================
	static void setLogLevel(LogLevel logLevel);

private:
	friend class TestRunnerImpl;

	

	// ==============================================================================================
	static SupportedOperators getSupportedOperatorsFromHooks(const ecs_type_info_t& ti) {
		SupportedOperators result{};
		result.equals = ti.hooks.equals != NULL && !(ti.hooks.flags & ECS_TYPE_HOOK_EQUALS_ILLEGAL);
		result.cmp		= ti.hooks.cmp		!= NULL && !(ti.hooks.flags & ECS_TYPE_HOOK_CMP_ILLEGAL);
		return result;
	}

	// ==============================================================================================
	/**
	 * Add metadata about supported operators for all components in module T.
	 */
	template <typename T>
	static void setSupportedOperatorsForModule(flecs::world& world) {
		flecs::entity moduleEntity = world.module<T>();

		// Start deferring to avoid locked storage
		world.defer_begin();

		world.query_builder<>()
			// Matches all component entities in the module
			.with<flecs::Component>() 
			.with(flecs::ChildOf, moduleEntity)
			.build()
			.each([&](flecs::entity e) {
					const ecs_type_info_t* ti = ecs_get_type_info(world, e);
					e.set<SupportedOperators>(ti 
						? getSupportedOperatorsFromHooks(*ti) 
						: SupportedOperators{}
					);
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
		setSupportedOperatorsForModule<T>(world);

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

