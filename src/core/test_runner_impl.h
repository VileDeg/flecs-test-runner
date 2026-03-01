#pragma once

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

#include <test_runner/common.h>

using namespace TestRunnerDetail;

struct ResolvedPropertyMetadata {
	flecs::entity type; // TODO: unused?
	void* ptr = nullptr; // Actual memory address of the property
	TypeMetadata::ComparisonFuncs funcs;
};

struct ResolvedProperty {
	flecs::entity type; 
	void* ptr = nullptr; // Actual memory address of the property
};


class ModuleImporter;

class TestRunnerImpl {
public:
	using TypeImporterMap =
		std::vector<WorldCallback>;

	// ================================================================================================
	struct Error : public AutoPrefixedError<Error> {
		using AutoPrefixedError::AutoPrefixedError;
	};

	// ================================================================================================
	struct SystemInvocation {
		std::string name;
		int timesToRun;
	};

	// ================================================================================================
	struct UnitTest {
		using Systems = std::vector<SystemInvocation>;

		struct Error : public AutoPrefixedError<Error> {
			using AutoPrefixedError::AutoPrefixedError;
		};

		// Tags
		struct Ready {};

		struct Executed {
			std::string statusMessage;
		};
		struct Passed {};

		struct Incomplete {
			std::string worldExpectedSerialized;
		};

		struct Operator {
			enum class Type {
				EQ,
				NEQ,
				LT,
				LTE,
				GT,
				GTE,
			};

			struct Path {

				Path(const std::string& pathStr) 
					: path(pathStr)
				{
				}
				Path(const char* pathStr)
					: path(pathStr)
				{
				}

				operator std::string() const {
					return path;
				}

				std::string getTopSegment() {
					std::stringstream ss(path);
					std::string segment;

					std::getline(ss, segment, DELIMETER);
					return segment;
				}

				std::string popSegment() {
					if (path.empty()) return "";

					std::istringstream ss(path);
					std::string segment;

					// 1. Extract the first segment
					if (std::getline(ss, segment, DELIMETER)) {
						// 2. Extract everything else left in the stream
						std::string remainder;
						if (std::getline(ss, remainder)) { // No delimiter needed for remainder
							path = remainder;
						} else {
							path = ""; // This was the last segment
						}
					}

					return segment;
				}

				bool isAnySegment() {
					//return path.find(DELIMETER) != std::string::npos;
					return !path.empty();
				}


				inline static constexpr char DELIMETER = '/';

				std::string path;
			};

			//using Path = std::string;

			

			Path path;
			Type type;
		};

		using Operators = std::vector<Operator>;

		std::string name; // TODO: use some ID (hash) instead of name?

		Systems systems;

		// vector of serialized entities
		using WorldConfiguration = std::vector<std::string>;

		WorldConfiguration initialConfiguration;
		WorldConfiguration expectedConfiguration;

		Operators operators;

		/*std::string scriptActual;
		std::string scriptExpected;*/

		std::optional<std::string> validate(bool complete = true) const {
			std::optional<std::string> statusMessage = std::nullopt;
			if (name.empty()) {
				statusMessage = "Name is empty";
			} else if (systems.empty()) {
				statusMessage = "No systems to run";
			} else if (initialConfiguration.empty()) {
				statusMessage = "Actual script is empty";
			} else if (complete && expectedConfiguration.empty()) {
				statusMessage = "Expected script is empty";
			}
			return statusMessage;
		}

		void normalizeSystemNames();
		//void runSystems(flecs::world& world) const;

		std::vector<std::string> getSystemNames() const;
	};

	

	
	// ================================================================================================
	template <typename... Args>
	static void addTestEntity(flecs::world& world, const char* name, Args&&... args) {
		world.entity(name)
			.set<UnitTest>({ std::forward<Args>(args)... });
	}
	// ================================================================================================
	enum class World {
		Actual,
		Expected
	};


	using ModulesMap =
		std::map<std::string, WorldCallback>;

	static flecs::entity getComponentByName(flecs::world& ecs, const std::string& name) {
		flecs::entity component = ecs.lookup(name.c_str(), ".", "."); // "/", "/"
		if (!component) {
			std::stringstream ss;
			ss << "Component \"" << name << "\" does not exist";
			throw Error(ss.str());
		}
		return component;
	}

	// ================================================================================================
	static ResolvedPropertyMetadata resolvePropertyMetadata(
		flecs::world& ecs, flecs::entity e, UnitTest::Operator::Path path
	);

	// ================================================================================================
	/**
	* Expect path to include component name (first segment).
	* TODO
	*/
	static ResolvedProperty resolveProperty(
		flecs::world& ecs, flecs::entity entity, UnitTest::Operator::Path path
	);

	// ================================================================================================
	/**
	* TODO
	* @param propertyPath Path of a property inside a component definiton.
	*/
	static ResolvedProperty resolveProperty(
		flecs::world& ecs, flecs::entity entity, flecs::entity component, UnitTest::Operator::Path propertyPath
	);

	//  ================================================================================================
	static bool compareComponents(
		ecs_world_t* world,
		ecs_entity_t component_id,
		const void* lhs,
		const void* rhs,
		UnitTest::Operator::Type operatorType
	);

	// ================================================================================================
	static bool compareWorldsComplete(flecs::world& world1, flecs::world& world2);
	// ================================================================================================
	static bool compareWorlds(
		flecs::world& initial, flecs::world& expected, UnitTest::Operators operators
	);
	// ================================================================================================
	static void runSystems(const flecs::world& world, const UnitTest::Systems& systems);
	// ================================================================================================
	static void runWorld(
		flecs::world& world, 
		World type, 
		const UnitTest& test, 
		const ModuleImporter& importer
	);
	// ================================================================================================
	static bool runUnitTest(UnitTest& test);
	// ================================================================================================
	/**
	* Returns serialized world 
	*/
	static std::string runUnitTestIncomplete(UnitTest& test);
	// ================================================================================================
	static void applyConfiguration(
		flecs::world& world, UnitTest::WorldConfiguration configuration
	);

	// ================================================================================================
	static std::optional<flecs::system> getSystemByName(
		const flecs::world& world, const std::string& systemName
	);


};

