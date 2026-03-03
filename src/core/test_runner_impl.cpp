

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

class ModuleImporter {
public:
	using ModulesMap =
		std::map<std::string, WorldCallback>;
	using ModulesNames = std::vector<std::string>;

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

	void setUsedModules(const ModulesNames& modules) {
		_usedModules = modules;
	}

	/*
	void resolveModules(std::vector<std::string> systems)
	{
		bool anyModuleFound = false;
		for (auto& sys : systems) {

			//auto m = matchModuleFromSystem(sys, _moduleRegistry);
			if (m.has_value()) {
				anyModuleFound = true;
				_usedModules.push_back(*m);
			}
		}
		if (!anyModuleFound) {
			throw ImportError("None of the systems belong to any module");
		}
	}
	*/

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
	//std::optional<std::string> matchModuleFromSystem(
	//	std::string systemFullPath, const ModulesMap& registry
	//);

	const ModulesMap& _moduleRegistry;
	const TypeRegistry& _typeRegistry;
	// TODO: used types

	ModulesNames _usedModules;
};

/*
std::optional<std::string> ModuleImporter::matchModuleFromSystem(
	const flecs::entity& system, const ModuleImporter::ModulesMap& registry
) {
	system.par

	//std::string current_path = systemFullPath;

	// Iteratively strip the last "::Section" until we find a match
	//while (true) {
	//	size_t pos = current_path.rfind("::");
	//	if (pos == std::string::npos) {
	//		break; // No more scopes to strip
	//	}

	//	// Cut off the last part to get the potential module name
	//	current_path = current_path.substr(0, pos);

	//	// Check if this substring matches a registered module
	//	if (registry.count(current_path)) {
	//		Log::info() << "System '" << systemFullPath
	//			<< "' belongs to module '" << current_path << "\n";
	//		//_moduleRegistry[current_path](world);
	//		return current_path;
	//	}
	//}

	//Log::error() << "Could not find a registered module for system path: '"
	//	<< systemFullPath << "'\n";
	//return std::nullopt;
}
*/


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
//void TestRunnerImpl::UnitTest::runSystems(flecs::world& world) const
//{
//	for (auto& sys : systems) {
//		// Run system
//		for (int i = 0; i < sys.timesToRun; ++i) {
//			auto system = getSystemByName(world, sys.name);
//			if (!system.has_value()) {
//				std::stringstream ss;
//				ss << "System " << sys.name << " not found";
//				throw Error(ss.str());
//			}
//
//			Log::info() << "[" << i << "] Running system '" << sys.name << "'\n";
//			system->run();
//		}
//	}
//}



void TestRunnerImpl::applyConfiguration(
	flecs::world& world, TestRunnerImpl::UnitTest::WorldConfiguration configuration
) {
	for (auto& serializedEntity : configuration) {

	

		auto entity = world.entity();
		entity.from_json(serializedEntity.c_str());

		//auto ser = entity.to_json();
		//ser = entity.to_json();
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
		//world.script_run("ScriptActual", test.scriptActual.c_str());
		runSystems(world, test.systems);
	} else {
		applyConfiguration(world, test.expectedConfiguration);
		//world.script_run("ScriptExpected", test.scriptExpected.c_str());
	}
}

std::vector<std::string> TestRunnerImpl::resolveModules(const std::vector<std::string>& systemsFullPath)
{
	std::vector<std::string> names;
	for (const auto& path : systemsFullPath) {
		auto system = getSystemByName(TestRunner::_world, path);
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
bool TestRunnerImpl::runUnitTest(UnitTest& test, std::ostringstream& out)
{
	test.normalizeSystemNames();

	flecs::world worldActual, worldExpected;

	ModuleImporter importer(TestRunner::_moduleImporterRegistry, TestRunner::_typeRegistry);
	auto modules = resolveModules(test.getSystemNames());
	importer.setUsedModules(modules);

	runWorld(worldActual, World::Actual, test, importer);
	runWorld(worldExpected, World::Expected, test, importer);
	
	return compareWorlds(worldActual, worldExpected, test.operators, out);
}

// ================================================================================================
std::string TestRunnerImpl::runUnitTestIncomplete(UnitTest& test)
{
	test.normalizeSystemNames();

	flecs::world worldActual;

	ModuleImporter importer(TestRunner::_moduleImporterRegistry, TestRunner::_typeRegistry);
	auto modules = resolveModules(test.getSystemNames());
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

ResolvedPropertyMetadata TestRunnerImpl::resolvePropertyMetadata(
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
		//throw std::invalid_argument("Not a valid integer");
	} else if (ec == std::errc::result_out_of_range) {
		throw std::out_of_range("Integer overflow");
	} else if (ptr != s.data() + s.size()) {
		throw std::invalid_argument("Trailing non-numeric characters found");
	}
	return value;
}

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

bool TestRunnerImpl::compareComponents(
	ecs_world_t* world, 
	ecs_entity_t componentId, 
	const void* lhs, 
	const void* rhs,
	Operator::Type operatorType,
	std::ostream& os
) {
	const ecs_type_info_t* ti = ecs_get_type_info(world, componentId);

	if (!ti) {
		std::ostringstream ss;
		ss << "Component " << componentId << " is missing type info" << "\n";
		Log::error() << ss.str();
		os << ss.str();
		return false;
	}

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

	throw Error("Invalid operator type");
}

bool TestRunnerImpl::compareWorldsComplete(flecs::world& world1, flecs::world& world2) {
	flecs::string json1 = world1.to_json();
	flecs::string json2 = world2.to_json();

	// TODO: find a more efficient way?
	bool matches = json1 == json2;

	if (!matches) {
		Log::trace() << "WORLDS DO NOT MATCH!\n";
		Log::trace() << "JSON byte length: \nWorld1=" << json1.size()
			<< "\nWorld2=" << json2.size();
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

// TODO: find more efficient way to compare
static bool compareEntitiesEq(flecs::entity initial, flecs::entity expected) {
	return initial.to_json() == expected.to_json();
}

static bool compareEntities(flecs::entity initial, flecs::entity expected, Operator::Type operatorType) {
	if (operatorType == Operator::Type::EQ) {
		return compareEntitiesEq(initial, expected);
	} else if (operatorType == Operator::Type::NEQ) {
		return !compareEntitiesEq(initial, expected);
	} else if (
		operatorType == Operator::Type::LTE || 
		operatorType == Operator::Type::GTE
	) {
		Log::warn() << "Entity comparison treats LTE and GTE as EQ!\n";
		return compareEntitiesEq(initial, expected);
 	} else {
		Log::error() << "Entity comparison supports only EQ and NEQ comparison operators\n";
		return false;
	}
}

static bool compareProperties(ResolvedPropertyMetadata initial, ResolvedPropertyMetadata expected, Operator::Type operatorType) {
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
	flecs::world& initial, 
	flecs::world& expected, 
	TestRunnerImpl::UnitTest::Operators operators,
	std::ostream& out
) {
	// TODO: add setting to compare only until first failure.

	if (operators.empty()) {
		// TODO: support == vs !==
		if (compareWorldsComplete(initial, expected)) {
			out << "Worlds match";
			return true;
		} else {
			out << "Worlds do not match";
			return false;
		}
	}

	auto getEntity = [&](flecs::world& ecs, const std::string& name) -> flecs::entity {
			auto entity = ecs.lookup(name.c_str());
			if (entity == 0) {
				Log::warn() << "Comparison failed: Enttity with name " << name << " not found\n";
			}
			return entity;
		};

	bool result = false;

	for (auto& oper : operators) {
		std::string entityName = oper.path.popSegment();
		if (entityName.empty()) {
			Log::error() << "Operator path is empty";
			continue;
		}

		out << "Comparison for:\n\t" << oper << "\n\t";

		auto initialEntity = getEntity(initial, entityName);
		auto expectedEntity = getEntity(expected, entityName);

		if (!oper.path.isAnySegment()) {
			if (compareEntities(initialEntity, expectedEntity, oper.type)) {
				out << "OK\n";
				return true;
			} else {
				out << "FAIL\n";
				return false;
			}
		}

		auto propertyIntial		= resolveProperty(initial, initialEntity, oper.path);
		auto propertyExpected = resolveProperty(expected, expectedEntity, oper.path);

		if (
			compareComponents(
				initial, 
				propertyIntial.type, 
				propertyIntial.ptr, 
				propertyExpected.ptr, 
				oper.type
			)
		) {
			out << "OK\n";
			result = true;
		} else {
			out << "FAIL\n";
			result = false;
		}
	}

	return result;
}

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
