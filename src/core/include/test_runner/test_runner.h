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
/**
* Public-facing static class, implementing the complete logic of the test execution.
*/
class TestRunner {
public:
	using WorldCallback			 = TestRunnerDetail::WorldCallback;
	using LogLevel					 = TestRunnerDetail::LogLevel;
	using Log								 = TestRunnerDetail::Log;
	using SupportedOperators = TestRunnerDetail::SupportedOperators;
	using OperatorType			 = TestRunnerDetail::OperatorType;
	using ModuleImporterMap	 = TestRunnerDetail::ModuleImporterMap;
	using RunTestProfiler		 = TestRunnerDetail::RunTestProfiler;

	inline static constexpr int DEFAULT_NUM_THREADS = 8;

	// ==============================================================================================
	struct Error : public TestRunnerDetail::AutoPrefixedError<Error> {
		using AutoPrefixedError::AutoPrefixedError;
	};

	// ==============================================================================================
	/**
	* Used to allow querying for the available modules.
	*/
	struct TestableModule{};
	
	// ==============================================================================================
	/**
	* To register as a module with Flecs.
	*/
	TestRunner(flecs::world& world);

	// ==============================================================================================
	/**
	* Can pass modules during compile time to register.
	*/
	template <typename... Ts>
	static void initialize(
		flecs::world& world, 
		int numThreads = DEFAULT_NUM_THREADS, 
		std::optional<int> profilingBatchSize = std::nullopt
	) {
		world.set_threads(numThreads);
		world.import<TestRunner>();
		registerModules<Ts...>(world);

		if (profilingBatchSize.has_value()) {
			RunTestProfiler::init(*profilingBatchSize);
		}
	}

	// ==============================================================================================
	/**
	* Start the Flecs run loop.
	*/
	static int run(flecs::world world) {
		Log::info() << "Flecs Test Runner listening on port " << ECS_REST_DEFAULT_PORT << " ...";
		return world.app()
			.enable_rest()
			.enable_stats()
			.run();
	}

	// ==============================================================================================
	/**
	* Main function. Initialize and run.
	*/
	template <typename... Ts>
	static int main(
		int numThreads = DEFAULT_NUM_THREADS,
		std::optional<int> profilingBatchSize = std::nullopt,
		int argc = 0, char* argv[] = nullptr
	) {
		flecs::world world(argc, argv);
		initialize<Ts...>(world, numThreads, profilingBatchSize);
		return run(world);
	}

	// ==============================================================================================
	/**
	* Register on_equals, on_compare Flecs hooks for multiple components.
	*/
	template <typename... Ts>
	static void registerOperators(flecs::world world) {
		(registerOperatorsForComponent<Ts>(world), ...);
	}

	// ==============================================================================================
	/**
	* Register on_equals, on_compare Flecs hooks for a component type.
	*/
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
	/**
	* Set desired level of logging.
	*/
	static void setLogLevel(LogLevel logLevel);

private:
	friend class TestRunnerImpl;

	// ==============================================================================================
	/**
	* Determine supported operators based on the presence of Flecs comparison hooks.
	*/
	static SupportedOperators getSupportedOperatorsFromHooks(const ecs_type_info_t& ti) {
		SupportedOperators result{};
		result.equals = ti.hooks.equals != NULL && !(ti.hooks.flags & ECS_TYPE_HOOK_EQUALS_ILLEGAL);
		result.cmp		= ti.hooks.cmp		!= NULL && !(ti.hooks.flags & ECS_TYPE_HOOK_CMP_ILLEGAL);
		return result;
	}

	// ==============================================================================================
	/**
	* Register metadata about supported operators for all components in module.
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
	/**
	* Register a testable module with the Test Runner.
	*/
	template <typename T>
	static void registerModule(flecs::world& world) {
		// Import module to make available for query
		auto m = world.import<T>();
		m.add<TestableModule>();

		// Set for all contained components.
		// For primitive types assume all ops are supported.
		setSupportedOperatorsForModule<T>(world);

		std::string name = std::string{ TestRunnerDetail::getTypeName<T>() } ;

		Log::trace() << "Registered module \"" << name << "\"";
		// Store in map to allow for later import in test world
		_moduleImporterRegistry[name] = [](flecs::world& w) {
				w.import<T>();
			};
	}

	// ==============================================================================================
	/**
	* Register multiple modules with the Test Runner.
	*/
	template <typename... Ts>
	static void registerModules(flecs::world& world) {
		(registerModule<Ts>(world), ...);
	}

	/**
	* Maps module name to importer function.
	*/
	inline static ModuleImporterMap _moduleImporterRegistry;
};

