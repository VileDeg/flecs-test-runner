

#include <flecs.h>

#include <string>
#include <functional>
#include <optional>
#include <map>
#include <stdexcept>
#include <algorithm>

#include <charconv>
#include <system_error>


#include <iostream>
#include <algorithm>
#include <sstream>

#include "test_runner_impl.h"

#include <test_runner/test_runner.h>
#include <test_runner/common.h>

using namespace TestRunnerDetail;

// ================================================================================================
// ModuleImporter
// ================================================================================================

// ================================================================================================
class ModuleImporter {
public:
	using ModulesNames = std::vector<std::string>;

	struct ImportError : public AutoPrefixedError<ImportError> {
		using AutoPrefixedError::AutoPrefixedError;
	};

	// ==============================================================================================
	ModuleImporter(
		const ModuleImporterMap& moduleRegistry
	)
		: _moduleRegistry(moduleRegistry)
	{
	}

	// ==============================================================================================
	void setUsedModules(const ModulesNames& modules) {
		_usedModules = modules;
	}

	// ==============================================================================================
	void importAll(flecs::world& world) const
	{
		if (_usedModules.empty()) {
			throw ImportError("No modules to import");
		}
		for (auto& m : _usedModules) {
			_moduleRegistry.at(m)(world);
		}
	}

private:
	const ModuleImporterMap& _moduleRegistry;
	ModulesNames _usedModules;
};

// ================================================================================================
// Internal classes and methods
// ================================================================================================

using WorldConfiguration = TestRunnerImpl::UnitTest::WorldConfiguration;
using Operator = TestRunnerImpl::UnitTest::Operator;
using Operators = TestRunnerImpl::UnitTest::Operators;

// ================================================================================================
// UnitTest
// ================================================================================================

// ================================================================================================
std::vector<std::string> TestRunnerImpl::UnitTest::getSystemNames() const {
	std::vector<std::string> names;
	names.reserve(systems.size());

	std::transform(
		systems.begin(),
		systems.end(),
		std::back_inserter(names),
		[](const TestRunnerImpl::SystemInvocation& si) {
			return si.name;
		}
	);
	return names;
}

// ================================================================================================
std::optional<std::string> TestRunnerImpl::UnitTest::validate(bool complete) const {
	std::optional<std::string> statusMessage = std::nullopt;
	if (name.empty()) {
		statusMessage = "Name is empty";
	} else if (systems.empty()) {
		statusMessage = "No systems to run";
	} else if (initialConfiguration.empty()) {
		statusMessage = "Initial configuration is empty";
	} else if (complete && expectedConfiguration.empty()) {
		statusMessage = "Expected configuration is empty";
	}
	return statusMessage;
}

// ================================================================================================
void TestRunnerImpl::UnitTest::normalizeSystemNames()
{
	for (auto& sys : systems) {
		// Normalize system name: replace "." with "::" to handle both notations
		std::string normalizedName = sys.name;
		
		const std::string s = ".";
		const std::string t = "::";

		std::string::size_type n = 0;
		while ((n = normalizedName.find(s, n)) != std::string::npos)
		{
			normalizedName.replace(n, s.size(), t);
			n += t.size();
		}

		sys.name = normalizedName;
	}
}

// ================================================================================================
void TestRunnerImpl::applyConfiguration(
	flecs::world& world, TestRunnerImpl::UnitTest::WorldConfiguration configuration
) {
	for (auto& serializedEntity : configuration) {
		auto entity = world.entity();
		entity.from_json(serializedEntity.c_str());
	}
}

// ================================================================================================
void TestRunnerImpl::runWorld(
	flecs::world& world, 
	World type, 
	const TestRunnerImpl::UnitTest& test, 
	const ModuleImporter& importer
) {
	importer.importAll(world);

	if (type == World::Actual) {
		applyConfiguration(world, test.initialConfiguration);
		runSystems(world, test.systems);
	} else {
		applyConfiguration(world, test.expectedConfiguration);
	}
}

// ================================================================================================
std::vector<std::string> TestRunnerImpl::resolveModules(const flecs::world& world, const std::vector<std::string>& systemsFullPath)
{
	std::vector<std::string> names;
	for (const auto& path : systemsFullPath) {
		auto system = getSystemByName(world, path);
		if (!system) {
			continue;
		}

		auto module = system->parent();
		if (!module) {
			throw Error("System " + path + " has no parent");
		}

		std::string moduleFullPath = module.name();

		module = module.parent();
		while (module) {
			std::string prefix = module.name();
			prefix += "::";
			moduleFullPath = prefix + moduleFullPath;

			module = module.parent();
		}

		if (std::find(names.begin(), names.end(), moduleFullPath) == names.end()) {
			names.push_back(moduleFullPath);
		}
	}
	return names;
}

// ================================================================================================
bool TestRunnerImpl::runUnitTest(const flecs::world& world, UnitTest& test, std::ostringstream& out)
{
	test.normalizeSystemNames();

	flecs::world worldActual, worldExpected;

	ModuleImporter importer(TestRunner::_moduleImporterRegistry);
	auto modules = resolveModules(world, test.getSystemNames());
	importer.setUsedModules(modules);

	runWorld(worldActual, World::Actual, test, importer);
	runWorld(worldExpected, World::Expected, test, importer);
	
	return compareWorlds(worldActual, worldExpected, test.operators, out);
}

// ================================================================================================
std::string TestRunnerImpl::runUnitTestIncomplete(const flecs::world& world, UnitTest& test)
{
	test.normalizeSystemNames();

	flecs::world worldActual;

	ModuleImporter importer(TestRunner::_moduleImporterRegistry);
	auto modules = resolveModules(world, test.getSystemNames());
	importer.setUsedModules(modules);

	runWorld(worldActual, World::Actual, test, importer);

	return worldActual.to_json();
}

// ================================================================================================
std::optional<flecs::system> TestRunnerImpl::getSystemByName(
	const flecs::world& world, const std::string& systemName
) {
	// Lookup the system by name
	flecs::entity systemEntity = world.lookup(systemName.c_str());

	if (!systemEntity) {
		Log::error() << "System '" << systemName << "' not found!\n";
		return std::nullopt;
	}

	// Check if it's a system
	if (!systemEntity.has(flecs::System) || !world.system(systemEntity)) {
		Log::error() << "Entity '" << systemName << "' is not a system!\n";
		return std::nullopt;
	}

	return world.system(systemEntity);
}

// ================================================================================================
int getPositiveInteger(const std::string& s) {
	if (s.empty()) {
		throw std::invalid_argument("Empty string");
	}

	// Check for negative sign immediately
	if (s[0] == '-') {
		throw std::domain_error("Negative numbers not allowed");
	}

	int value;
	auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);

	if (ec == std::errc::invalid_argument) {
		return -1;
	} else if (ec == std::errc::result_out_of_range) {
		throw std::out_of_range("Integer overflow");
	} else if (ptr != s.data() + s.size()) {
		throw std::invalid_argument("Trailing non-numeric characters found");
	}
	return value;
}

// ================================================================================================
ResolvedProperty TestRunnerImpl::resolveProperty(
	flecs::world& ecs, 
	flecs::entity entity, 
	UnitTest::Operator::Path path
) {
	if (path.path.empty()) {
		throw Error("Cannot resolve empty path");
	}
	std::string initialPath = path;
	// First segment (The Component)
	std::string segment = path.popSegment();
	flecs::entity component = getComponentByName(ecs, segment);
	return resolveProperty(ecs, entity, component, path);
}

// ================================================================================================
ResolvedProperty TestRunnerImpl::resolveProperty(
	flecs::world& ecs, 
	flecs::entity entity, 
	flecs::entity component, 
	UnitTest::Operator::Path propertyPath
) {
	std::string unmodifiedPath = propertyPath;
	
	void* basePtr = entity.get_mut(component);
	if (!basePtr) {
		std::stringstream ss;
		ss << "Component \"" << component.name() << "\" does not exist on entity \"" << entity.name() << "\"";
		throw Error(ss.str());
	}

	flecs::cursor cur(ecs, component, basePtr);

	std::string segment;
	while (propertyPath.isAnySegment()) {
		segment = propertyPath.popSegment();

		if (cur.push()) {
			throw Error("Failed to push into cursor scope");
		}

		int index = getPositiveInteger(segment);
		if (index > -1) {
			if (!cur.is_collection()) {
				throw Error("Invalid address, is not a collection");
			}
			// Move to index
			if (cur.elem(index) < 0) {
				throw Error("Cursor element failed");
			}
		} else {
			if (cur.member(segment.c_str())) {
				throw Error("Cursor member failed");
			}
		}
	}

	flecs::entity type_ent = cur.get_type();

	Log::trace() << "Resolved path \"" << unmodifiedPath << "\" to property \"" << type_ent.name() << "\"";

	return {
		type_ent,
		cur.get_ptr(),
	};
}

// ================================================================================================
bool TestRunnerImpl::compareProperty(
	ecs_world_t* world, 
	ecs_entity_t componentId, 
	const void* lhs, 
	const void* rhs,
	OperatorType operatorType,
	std::ostream& os
) {
	const ecs_type_info_t* ti = ecs_get_type_info(world, componentId);

	auto runCmp = [&]() -> int {
		if (ti->hooks.cmp && !(ti->hooks.flags & ECS_TYPE_HOOK_CMP_ILLEGAL)) {
			return ti->hooks.cmp(lhs, rhs, ti);
		}
		throw Error("Missing comparison hooks\n");
	};

	auto runEquals = [&]() -> bool {
		return ti->hooks.equals && !(ti->hooks.flags & ECS_TYPE_HOOK_EQUALS_ILLEGAL)
			? ti->hooks.equals(lhs, rhs, ti)
			: runCmp() == 0;
	};

	try {
		if (!ti) {
			std::ostringstream ss;
			ss << "Component " << componentId << " is missing type info" << "\n";
			throw Error(ss.str());
		}

		if (operatorType == OperatorType::EQ) {
			return runEquals();
		}
		if (operatorType == OperatorType::NEQ) {
			return !runEquals();
		}

		if (operatorType == OperatorType::LT) {
			return runCmp() < 0;
		}
		if (operatorType == OperatorType::LTE) {
			return runCmp() <= 0;
		}
		if (operatorType == OperatorType::GT) {
			return runCmp() > 0;
		}
		if (operatorType == OperatorType::GTE) {
			return runCmp() >= 0;
		}
	} catch (const Error& e) {
		os << e.what();
	}

	return false;
}

// ================================================================================================
// TODO: find more efficient way to compare
static bool compareEntitiesEq(flecs::entity initial, flecs::entity expected) {
	return initial.to_json() == expected.to_json();
}

// ================================================================================================
static bool compareEntities(flecs::entity initial, flecs::entity expected, OperatorType operatorType) {
	if (initial.id() < 1 || expected.id() < 1) {
		return false;
	}
	if (operatorType == OperatorType::EQ) {
		return compareEntitiesEq(initial, expected);
	} else if (operatorType == OperatorType::NEQ) {
		return !compareEntitiesEq(initial, expected);
	} else if (
		operatorType == OperatorType::LTE || 
		operatorType == OperatorType::GTE
	) {
		Log::warn() << "Entity comparison treats LTE and GTE as EQ!\n";
		return compareEntitiesEq(initial, expected);
 	} else {
		Log::error() << "Entity comparison supports only EQ and NEQ comparison operators\n";
		return false;
	}
}

// ================================================================================================
bool TestRunnerImpl::compareWorlds(
	flecs::world& initial, 
	flecs::world& expected, 
	TestRunnerImpl::UnitTest::Operators operators,
	std::ostream& out
) {
	// TODO: add setting to compare only until first failure.

	if (operators.empty()) {
		out << "Nothing to compare";
		return false;
	}

	auto getEntity = [&](flecs::world& ecs, const std::string& name) -> flecs::entity {
			auto entity = ecs.lookup(name.c_str());
			if (entity == 0) {
				Log::warn() << "Comparison failed: Enttity with name " << name << " not found\n";
			}
			return entity;
		};

	bool result = true;

	for (auto& oper : operators) {
		out << "Comparison for:\n\t" << oper << "\n\t";

		std::string entityName = oper.path.popSegment();
		if (entityName.empty()) {
			out << "Operator path is empty";
			continue;
		}

		auto initialEntity = getEntity(initial, entityName);
		auto expectedEntity = getEntity(expected, entityName);

		if (!oper.path.isAnySegment()) {
			if (compareEntities(initialEntity, expectedEntity, oper.type)) {
				out << "OK\n";
			} else {
				out << "FAIL\n";
				result = false;
			}
			continue;
		}

		auto propertyIntial		= resolveProperty(initial, initialEntity, oper.path);
		auto propertyExpected = resolveProperty(expected, expectedEntity, oper.path);

		if (
			compareProperty(
				initial, 
				propertyIntial.type, 
				propertyIntial.ptr, 
				propertyExpected.ptr, 
				oper.type,
				out
			)
		) {
			out << "\tOK\n";
		} else {
			out << "\tFAIL\n";
			result = false;
		}
	}

	return result;
}

// ================================================================================================
void TestRunnerImpl::runSystems(const flecs::world& world, const UnitTest::Systems& systems)
{
	for (auto& sys : systems) {
		if (sys.name.empty()) {
			Log::warn() << "System name is empty";
			continue;
		}

		auto system = getSystemByName(world, sys.name);
		if (!system.has_value()) {
			std::stringstream ss;
			ss << "System " << sys.name << " not found";
			throw Error(ss.str());
		}

		// Run system
		for (int i = 0; i < sys.timesToRun; ++i) {
			Log::info() << "[" << i << "] Running system '" << sys.name << "'\n";
			system->run();
		}
	}
}
