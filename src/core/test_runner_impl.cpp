

#include <flecs.h>

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

#include "test_runner_impl.h"

#include <test_runner/test_runner.h>
#include "common.h"

using namespace TestRunnerDetail;


// ================================================================================================
// ModuleImporter
// ================================================================================================

class ModuleImporter {
public:
	using ModulesMap =
		std::map<std::string, WorldCallback>;

	template <typename Derived>
	using AutoPrefixedError = AutoPrefixedError<Derived>;

	struct ImportError : public AutoPrefixedError<ImportError> {
		using AutoPrefixedError::AutoPrefixedError;
	};

	ModuleImporter(
		const ModulesMap& moduleRegistry,
		const TypeRegistry& typRegistry
	)
		: _moduleRegistry(moduleRegistry)
		, _typeRegistry(typRegistry)
	{
	}

	void resolveModules(std::vector<std::string> systems)
	{
		bool anyModuleFound = false;
		for (auto& sys : systems) {
			auto m = matchModuleFromSystemPath(sys, _moduleRegistry);
			if (m.has_value()) {
				anyModuleFound = true;
				_usedModules.push_back(*m);
			}
		}
		if (!anyModuleFound) {
			throw ImportError("None of the systems belong to any module");
		}
	}

	void importAll(flecs::world& world) const
	{
		if (_usedModules.empty()) {
			throw ImportError("No modules to import");
		}
		for (auto& m : _usedModules) {
			_moduleRegistry.at(m)(world);
		}

		// Register all additional metadata for types
		_typeRegistry.applyAll(world);

		/*for (auto& typeImporter : _typeRegistry) {
			typeImporter(world);
		}*/
	}

private:
	std::optional<std::string> matchModuleFromSystemPath(
		std::string systemFullPath, const ModulesMap& registry
	);

	const ModulesMap& _moduleRegistry;
	const TypeRegistry& _typeRegistry;
	// TODO: used types

	std::vector<std::string> _usedModules;
};

std::optional<std::string> ModuleImporter::matchModuleFromSystemPath(
	std::string systemFullPath, const ModuleImporter::ModulesMap& registry
) {
	std::string current_path = systemFullPath;

	// Iteratively strip the last "::Section" until we find a match
	while (true) {
		size_t pos = current_path.rfind("::");
		if (pos == std::string::npos) {
			break; // No more scopes to strip
		}

		// Cut off the last part to get the potential module name
		current_path = current_path.substr(0, pos);

		// Check if this substring matches a registered module
		if (registry.count(current_path)) {
			Log::info() << "[Info] System '" << systemFullPath
				<< "' belongs to module '" << current_path << "\n";
			//_moduleRegistry[current_path](world);
			return current_path;
		}
	}

	Log::warn() << "[Error] Could not find a registered module for system path: '"
		<< systemFullPath << "'\n";
	return std::nullopt;
}


// ================================================================================================


// ================================================================================================
// Internal classes and methods
// ================================================================================================


using WorldConfiguration = TestRunnerImpl::UnitTest::WorldConfiguration;
using Operator = TestRunnerImpl::UnitTest::Operator;
using Operators = TestRunnerImpl::UnitTest::Operators;

// ================================================================================================




// ================================================================================================
// UnitTest
// ================================================================================================

// ================================================================================================
std::vector<std::string> TestRunnerImpl::UnitTest::getSystemNames() {
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
void TestRunnerImpl::UnitTest::normalizeSystemNames()
{
	for (auto& sys : systems) {
		// Normalize system name: replace "." with "::" to handle both notations
		size_t pos = 0;
		if ((pos = sys.name.find(".", pos)) == std::string::npos) {
			continue;
		}

		std::string normalizedName = sys.name;
		do {
			normalizedName.replace(pos, 1, "::");
			pos += 2; // Move past the replacement
		} while ((pos = normalizedName.find(".", pos)) != std::string::npos);
		sys.name = normalizedName;
	}
}

// ================================================================================================
void TestRunnerImpl::UnitTest::runSystems(flecs::world& world)
{
	for (auto& sys : systems) {
		// Run system
		for (int i = 0; i < sys.timesToRun; ++i) {
			auto system = getSystemByName(world, sys.name);
			if (!system.has_value()) {
				std::stringstream ss;
				ss << "System " << sys.name << " not found";
				throw Error(ss.str());
			}

			Log::info() << "[" << i << "] Running system '" << sys.name << "'\n";
			system->run();
		}
	}
}



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
	flecs::world& world, World type, TestRunnerImpl::UnitTest& test, const ModuleImporter& importer
) {
	importer.importAll(world);

	if (type == World::Actual) {
		applyConfiguration(world, test.initialConfiguration);
		//world.script_run("ScriptActual", test.scriptActual.c_str());
		test.runSystems(world);
	} else {
		applyConfiguration(world, test.expectedConfiguration);
		//world.script_run("ScriptExpected", test.scriptExpected.c_str());
	}
}

// ================================================================================================
void TestRunnerImpl::runUnitTest(flecs::entity e, UnitTest& test)
{
	Log::info() << "Running test: " << test.name << "\n";

	auto status = test.validate();
	if (status.has_value()) {
		std::stringstream ss;
		ss << "Failed to run test " << test.name << ": " << *status << "\n";
		e.set<UnitTest::Executed>({ *status });
		throw Error(ss.str());
	}
	test.normalizeSystemNames();

	flecs::world worldActual, worldExpected;

	ModuleImporter importer(TestRunner::_moduleImporterRegistry, TestRunner::_typeRegistry);
	importer.resolveModules(test.getSystemNames());
	runWorld(worldActual, World::Actual, test, importer);
	runWorld(worldExpected, World::Expected, test, importer);

	std::string statusMessage;
	if (compareWorlds(worldActual, worldExpected, test.operators)) {
		e.add<UnitTest::Passed>();
		statusMessage = "OK";
	} else {
		statusMessage = "Worlds do not match. Actual vs. expected";
	}
	e.set<UnitTest::Executed>({ statusMessage });
}

// ================================================================================================
void TestRunnerImpl::runUnitTestIncomplete(flecs::entity e, UnitTest& test)
{
	Log::info() << "Running test (Incomplete): " << test.name << "\n";

	auto status = test.validate(false);
	if (status.has_value()) {
		std::stringstream ss;
		ss << "Failed to run test " << test.name << ": " << *status << "\n";
		e.set<UnitTest::Executed>({ *status });
		return;
	}
	test.normalizeSystemNames();

	flecs::world worldActual;

	ModuleImporter importer(TestRunner::_moduleImporterRegistry, TestRunner::_typeRegistry);
	importer.resolveModules(test.getSystemNames());
	runWorld(worldActual, World::Actual, test, importer);

	std::string worldExpectedSerialized = worldActual.to_json();
	e.set<UnitTest::Executed>({ "OK" });
	e.set<UnitTest::Incomplete>({ worldExpectedSerialized });
}


// ================================================================================================
std::optional<flecs::system> TestRunnerImpl::getSystemByName(
	const flecs::world& world, const std::string& systemName
) {
	// Lookup the system by name
	flecs::entity systemEntity = world.lookup(systemName.c_str());

	if (!systemEntity) {
		Log::error() << "ERROR: System '" << systemName << "' not found!\n";
		return std::nullopt;
	}

	// Check if it's a system
	if (!systemEntity.has(flecs::System) || !world.system(systemEntity)) {
		Log::error() << "ERROR: Entity '" << systemName << "' is not a system!\n";
		return std::nullopt;
	}

	return world.system(systemEntity);
}


/*

static std::string getTopSegment(const Operator::Path& path, const char delim = Operator::PATH_SEP) {
	std::stringstream ss(path);
	std::string segment;

	std::getline(ss, segment, delim);
	return segment;
}

static std::string popSegment(Operator::Path& path, const char delim = Operator::PATH_SEP) {
	std::stringstream ss(path);
	std::string segment;

	if (std::getline(ss, segment, delim)) {
		// Extract the remainder of the stream into the original path
		std::string remainder;
		if (std::getline(ss, remainder, '\0')) {
			path = remainder;
		} else {
			path = ""; // No more segments remain
		}
	}

	return segment;
}

static bool isAnySegment(const Operator::Path& path, const char delim = Operator::PATH_SEP) {
	return path.find(delim) != std::string::npos;
}
*/


ResolvedProperty TestRunnerImpl::resolveProperty(
	flecs::world& ecs, flecs::entity e, Operator::Path path
) {


	//std::stringstream ss(path);
	std::string segment;

	// Start with the first segment (The Component)

	segment = path.popSegment();
	//std::getline(ss, segment, Operator::PATH_SEP);
	flecs::entity comp_id = ecs.lookup(segment.c_str(), ".", ".");
	void* current_ptr = e.get_mut(comp_id); // Get base memory

	flecs::cursor cur(ecs, comp_id, current_ptr);
	// need to do push here?

	while (path.isAnySegment()) {
		segment = path.popSegment();

		// Handle array syntax: member[index]
		size_t bracket_open = segment.find('[');
		if (bracket_open != std::string::npos) {
			std::string member_name = segment.substr(0, bracket_open);
			int index = std::stoi(segment.substr(bracket_open + 1, segment.find(']') - bracket_open - 1));

			cur.member(member_name.c_str());
			cur.push();     // Enter the array
			cur.elem(index); // Move to index
		} else {
			if (cur.member(segment.c_str())) {
				throw Error("Cursor member failed");
			}
		}

		// If there are more segments, we need to "push" into the struct
		if (path.isAnySegment()) {
			cur.push();
		}
	}

	flecs::entity type_ent = cur.get_type();
	const auto* meta = type_ent.try_get<TypeMetadata>();

	return {
		type_ent,
		cur.get_ptr(),
		meta ? meta->funcs : TypeMetadata::ComparisonFuncs{}
	};
}

static bool compareComponents(
	ecs_world_t* world, 
	ecs_entity_t component_id, 
	const void* lhs, 
	const void* rhs,
	Operator::Type operatorType
) {
	const ecs_type_info_t* ti = ecs_get_type_info(world, component_id);

	if (!ti) {
		Log::error() << "Component " << component_id << " is missing type info" << "\n";
		return false;
	}
	
	/* TODO: need that?
		if (!ti) return memcmp(p1, p2, ecs_get_typeid_size(world, component_id)) == 0;
	*/

	if (operatorType == Operator::Type::EQ) {
		return ti->hooks.equals(lhs, rhs, ti);
	}
	if (operatorType == Operator::Type::NEQ) {
		return !ti->hooks.equals(lhs, rhs, ti);
	}

	if (operatorType == Operator::Type::LT) {
		return ti->hooks.cmp(lhs, rhs, ti) < 0;
	}
	if (operatorType == Operator::Type::LTE) {
		return ti->hooks.cmp(lhs, rhs, ti) <= 0;
	}

	if (operatorType == Operator::Type::GT) {
		return ti->hooks.cmp(lhs, rhs, ti) > 0;
	}
	if (operatorType == Operator::Type::GTE) {
		return ti->hooks.cmp(lhs, rhs, ti) >= 0;
	}

#if 0

	// Priority 1: Explicit Equality Hook (fastest for bool check)
	if (ti->hooks.equals) {
		return ti->hooks.equals(lhs, rhs, ti);
	}

	// Priority 2: Comparison Hook (returns 0 for equal)
	if (ti->hooks.cmp) {
		return ti->hooks.cmp(lhs, rhs, ti) == 0;
	}

	// Fallback: Byte-wise comparison
	return memcmp(lhs, rhs, ti->size) == 0;
#endif
}

bool TestRunnerImpl::compareWorldsComplete(flecs::world& world1, flecs::world& world2) {
	Log::info() << "\nWorld Comparison (using serialization):\n";
	Log::info() << "========================================\n";

	flecs::string json1 = world1.to_json();
	flecs::string json2 = world2.to_json();

	/*
	Log::info() << "\nWorld 1 JSON:\n";
	Log::info() << json1 << "\n";

	Log::info() << "\nWorld 2 JSON:\n";
	Log::info() << json2 << "\n";
	*/

	// Compare the serialized representations
	// TODO: find a more efficient way
	bool matches = (json1 == json2);

	Log::info() << "\n";
	if (matches) {
		Log::info() << "WORLDS MATCH!\n";
		Log::info() << "The serialized JSON representations are identical.\n";
	} else {
		Log::info() << "WORLDS DO NOT MATCH!\n";
		Log::info() << "The serialized JSON representations differ.\n";

		// Show size difference as a hint
		Log::info() << "\nJSON sizes: World1=" << json1.size()
			<< " bytes, World2=" << json2.size() << " bytes\n";
	}

	return matches;
}

/*
bool entities_are_equal(ecs_world_t* world, ecs_entity_t e1, ecs_entity_t e2) {
	if (e1 == e2) return true;

	// 1. Archetype check: Must have the same component IDs/Tags/Pairs
	ecs_table_t* table = ecs_get_table(world, e1);
	if (table != ecs_get_table(world, e2)) {
		return false;
	}

	// 2. Get storage records (contains table-relative row index)
	const ecs_record_t* r1 = ecs_record_get(world, e1);
	const ecs_record_t* r2 = ecs_record_get(world, e2);

	int32_t row1 = ECS_RECORD_TO_ROW(r1->row);
	int32_t row2 = ECS_RECORD_TO_ROW(r2->row);

	// 3. Iterate through data-carrying columns
	int32_t column_count = ecs_table_column_count(table);
	for (int32_t i = 0; i < column_count; ++i) {
		size_t size = ecs_table_column_size(table, i);
		if (size == 0) continue; // Skip tags

		void* ptr = ecs_table_get_column(table, i, 0);
		void* data1 = (char*)ptr + (size * row1);
		void* data2 = (char*)ptr + (size * row2);

		if (memcmp(data1, data2, size) != 0) {
			return false;
		}
	}

	return true;
}
*/


static bool compareEntities(flecs::entity initial, flecs::entity expected, Operator::Type operatorType) {
	if (operatorType == Operator::Type::EQ) {


		return initial.to_json() == expected.to_json();
	} else if (operatorType == Operator::Type::NEQ) {
		return initial.to_json() != expected.to_json();
	} else {
		Log::warn() << "Entity comparison supports only '==' or '!==' comparison types\n";
		return false;
	}
}

static bool compareProperties(ResolvedProperty initial, ResolvedProperty expected, Operator::Type operatorType) {
	// Must be same since they come from the same component entity
	assert(initial.funcs == expected.funcs);
	auto funcs = initial.funcs;

	if (operatorType == Operator::Type::EQ) {
		return funcs.eq(initial.ptr, expected.ptr);
	} else if (operatorType == Operator::Type::NEQ) {
		return funcs.neq(initial.ptr, expected.ptr);
	} else if (operatorType == Operator::Type::LT) {
		return funcs.lt(initial.ptr, expected.ptr);
	} else if (operatorType == Operator::Type::LTE) {
		return funcs.lte(initial.ptr, expected.ptr);
	} else if (operatorType == Operator::Type::GT) {
		return funcs.gt(initial.ptr, expected.ptr);
	} else if (operatorType == Operator::Type::GTE) {
		return funcs.gte(initial.ptr, expected.ptr);
	} else {
		Log::warn() << __func__ << ": invalid comparison operator\n";
		return false;
	}
}

bool TestRunnerImpl::compareWorlds(
	flecs::world& initial, flecs::world& expected, TestRunnerImpl::UnitTest::Operators operators
) {
	if (operators.empty()) {
		// TODO: support == vs !==
		return compareWorldsComplete(initial, expected);
	}

	auto getEntity = [&](flecs::world& ecs, const std::string& name) -> flecs::entity {
			auto entity = ecs.lookup(name.c_str());
			if (entity == 0) {
				Log::warn() << "Comparison failed: Enttity with name " << name << " not found\n";
			}
			return entity;
		};

	bool comparisonFailed = false;

	for (auto& oper : operators) {
		std::string entityName = oper.path.popSegment();
		if (entityName.empty()) {
			// TODO: warning operator path empty
			continue;
		}

		auto initialEntity = getEntity(initial, entityName);
		auto expectedEntity = getEntity(expected, entityName);

		if (!oper.path.isAnySegment()) {
			if (!compareEntities(initialEntity, expectedEntity, oper.type)) {
				comparisonFailed = true;
				break;
			}
		}

		auto propertyIntial = resolveProperty(initial, initialEntity, oper.path);
		auto propertyExpected = resolveProperty(expected, expectedEntity, oper.path);

		/*/
		if (!compareProperties(propertyIntial, propertyExpected, oper.type)) {
			comparisonFailed = true;
		}
		/*/
		if (
			!compareComponents(
				initial, 
				propertyIntial.type, 
				propertyIntial.ptr, 
				propertyExpected.ptr, 
				oper.type
			)
		) {
			comparisonFailed = true;
		}
		//*/
	}


	return !comparisonFailed;
}

