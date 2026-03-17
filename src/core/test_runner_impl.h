#pragma once

#include <flecs.h>

#include <string>
#include <functional>
#include <optional>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <unordered_set>

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
			using Type = OperatorType;
			//enum class Type {
			//	EQ,
			//	NEQ,
			//	LT,
			//	LTE,
			//	GT,
			//	GTE,
			//};

			static std::string typeToString(Type type) {
				if (type == Type::EQ) {
					return "Equal";
				}
				if (type == Type::NEQ) {
					return "Not Equal";
				}
				if (type == Type::LT) {
					return "Less Than";
				}
				if (type == Type::LTE) {
					return "Less Than Equal";
				}
				if (type == Type::GT) {
					return "Greater Than";
				}
				if (type == Type::GTE) {
					return "Greater Than Equal";
				}
				return "<INVALID>";
			}

			struct Path {
				Path()
					: path("") {
				}
				Path(const std::string& pathStr) 
					: path(pathStr) {
				}
				Path(const char* pathStr)
					: path(pathStr) {
				}

				operator std::string() const {
					return path;
				}

				friend bool operator==(const Path& lhs, const Path& rhs) {
					return lhs.path == rhs.path;
				}

				friend Path operator+(const Path& lhs, const Path& rhs) {
					return lhs.path + DELIMETER + rhs.path;
				}

				friend Path& operator+=(Path& lhs, const Path& rhs) {
					lhs = lhs + rhs;
					return lhs;
				}

				friend std::ostream& operator<<(std::ostream& os, const Path& rhs) {
					os << rhs.path;
					return os;
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

					// Extract and skip repeated delimeter e.g. "//"
					bool good = false;
					do {
						auto& res = std::getline(ss, segment, DELIMETER);
						good = static_cast<bool>(res);
					} while (good && segment.empty());

					// Extract everything else left in the stream
					std::string remainder;
					if (std::getline(ss, remainder)) { // No delimiter needed for remainder
						path = remainder;
					} else {
						path = ""; // This was the last segment
					}

					return segment;
				}

				bool isAnySegment() {
					return !path.empty();
				}


				inline static constexpr char DELIMETER = '/';

				std::string path;
			};

			friend bool operator==(const Operator& lhs, const Operator& rhs) {
				return lhs.path == rhs.path;
			}

			friend std::ostream& operator<<(std::ostream& os, const Operator& rhs) {
				os << "[path, type]: " 
					<< rhs.path << ", " 
					<< Operator::typeToString(rhs.type);
				return os;
			}

			Path path;
			Type type;
		};

		// TODO: use unordered_set
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
				statusMessage = "Initial configuration is empty";
			} else if (complete && expectedConfiguration.empty()) {
				statusMessage = "Expected configuration is empty";
			}
			return statusMessage;
		}

		void normalizeSystemNames();
		//void runSystems(flecs::world& world) const;

		std::vector<std::string> getSystemNames() const;
	};

	// ================================================================================================
	using WorldComparisonResult = std::unordered_map<UnitTest::Operator, bool>;

	
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

	static std::vector<std::string> resolveModules(const flecs::world& world, const std::vector<std::string>& systemsFullPath);

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
		ecs_entity_t componentId,
		const void* lhs,
		const void* rhs,
		OperatorType operatorType,
		std::ostream& os
	);
	static bool compareComponents(
		ecs_world_t* world,
		ecs_entity_t componentId,
		const void* lhs,
		const void* rhs,
		OperatorType operatorType
	) {
		return compareComponents(world, componentId, lhs, rhs, operatorType, Log::trace());
	}

	// ================================================================================================
	static bool compareWorldsComplete(flecs::world& world1, flecs::world& world2);
	// ================================================================================================
	static bool compareWorlds(
		flecs::world& initial, 
		flecs::world& expected, 
		UnitTest::Operators operators,
		std::ostream& os
	);
	static bool compareWorlds(
		flecs::world& initial,
		flecs::world& expected,
		UnitTest::Operators operators
	) {
		return compareWorlds(initial, expected, operators, Log::trace());
	}
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
	static bool runUnitTest(const flecs::world& world, UnitTest& test, std::ostringstream& out);
	static bool runUnitTest(const flecs::world& world, UnitTest& test) {
		std::ostringstream dummy;
		return runUnitTest(world, test, dummy);
	}
	// ================================================================================================
	/**
	* Returns serialized world 
	*/
	static std::string runUnitTestIncomplete(const flecs::world& world, UnitTest& test);
	// ================================================================================================
	static void applyConfiguration(
		flecs::world& world, UnitTest::WorldConfiguration configuration
	);

	// ================================================================================================
	static std::optional<flecs::system> getSystemByName(
		const flecs::world& world, const std::string& systemName
	);


};

