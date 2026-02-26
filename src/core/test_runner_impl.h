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

#include "common.h"

using namespace TestRunnerDetail;


class Log {
public:
	using Level = TestRunnerDetail::LogLevel;

	static std::ostream& fatal() { return _logLevel >= Level::FATAL ? std::cerr : _nullStream; }
	static std::ostream& error() { return _logLevel >= Level::ERROR ? std::cerr : _nullStream; }
	static std::ostream& warn()  { return _logLevel >= Level::WARN  ? std::cout : _nullStream; }
	static std::ostream& info()  { return _logLevel >= Level::INFO  ? std::cout : _nullStream; }
	static std::ostream& trace() { return _logLevel >= Level::TRACE ? std::cout : _nullStream; }

	static void setLevel(Level level) {
		_logLevel = level;
	}

private:
	class NullBuffer : public std::streambuf {
	protected:
		int overflow(int c) override { return c; }
	};

	class NullStream : public std::ostream {
	public:
		NullStream() : std::ostream(&nullBuffer) {}
	private:
		NullBuffer nullBuffer;
	};

	inline static Level _logLevel = Level::WARN;
	inline static NullStream _nullStream;
};


template<typename T>
using comparison_operand_t = const std::remove_reference_t<T>&;

// Generic Macro-like Trait Structure to avoid repetition
#define DEFINE_COMPARISON_TRAIT(name, op) \
template<typename T, typename = void> \
struct name : std::false_type {}; \
\
template<typename T> \
struct name<T, std::void_t<decltype(std::declval<comparison_operand_t<T>>() op std::declval<comparison_operand_t<T>>())>> \
    : std::bool_constant<std::is_convertible_v< \
        decltype(std::declval<comparison_operand_t<T>>() op std::declval<comparison_operand_t<T>>()), \
        bool>> {}; \
\
template<typename T> \
inline constexpr bool name##_v = name<T>::value;

// Equal (==)
template<typename T, typename = void>
struct has_eq : std::false_type {};

template<typename T>
struct has_eq<T, std::void_t<decltype(std::declval<T>() == std::declval<T>())>> : std::true_type {};

// Not Equal (!==)
template<typename T, typename = void>
struct has_neq : std::false_type {};

template<typename T>
struct has_neq<T, std::void_t<decltype(std::declval<T>() != std::declval<T>())>> : std::true_type {};

// Less Than (<)
template<typename T, typename = void>
struct has_lt : std::false_type {};

template<typename T>
struct has_lt<T, std::void_t<decltype(std::declval<T>() < std::declval<T>())>> : std::true_type {};

// Greater Than (>)
template<typename T, typename = void>
struct has_gt : std::false_type {};

template<typename T>
struct has_gt<T, std::void_t<decltype(std::declval<T>() > std::declval<T>())>> : std::true_type {
};

// Less Than or Equal (<=)
template<typename T, typename = void>
struct has_lte : std::false_type {};

template<typename T>
struct has_lte<T, std::void_t<decltype(std::declval<T>() <= std::declval<T>())>> : std::true_type {};

// Greater Than or Equal (>=)
template<typename T, typename = void>
struct has_gte : std::false_type {};

template<typename T>
struct has_gte<T, std::void_t<decltype(std::declval<T>() >= std::declval<T>())>> : std::true_type {};

// Helper variable templates
template<typename T>
inline static constexpr bool has_eq_v = has_eq<T>::value;

template<typename T>
inline static constexpr bool has_neq_v = has_neq<T>::value;

template<typename T>
inline static constexpr bool has_lt_v = has_lt<T>::value;

template<typename T>
inline static constexpr bool has_gt_v = has_gt<T>::value;

template<typename T>
inline static constexpr bool has_lte_v = has_lte<T>::value;

template<typename T>
inline static constexpr bool has_gte_v = has_gte<T>::value;


struct ResolvedProperty {
	flecs::entity type; // TODO: unused?
	void* ptr = nullptr; // Actual memory address of the property
	TypeMetadata::ComparisonFuncs funcs;
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

				operator std::string() {
					return path;
				}

				std::string getTopSegment() {
					std::stringstream ss(path);
					std::string segment;

					std::getline(ss, segment, DELIMETER);
					return segment;
				}

				std::string popSegment() {
					std::stringstream ss(path);
					std::string segment;

					if (std::getline(ss, segment, DELIMETER)) {
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

				bool isAnySegment() {
					return path.find(DELIMETER) != std::string::npos;
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

		std::vector<SystemInvocation> systems;

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
		void runSystems(flecs::world& world);

		std::vector<std::string> getSystemNames();
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

	// ================================================================================================
	static ResolvedProperty resolveProperty(
		flecs::world& ecs, flecs::entity e, UnitTest::Operator::Path path
	);

	// ================================================================================================
	static bool compareWorldsComplete(flecs::world& world1, flecs::world& world2);
	// ================================================================================================
	static bool compareWorlds(
		flecs::world& initial, flecs::world& expected, UnitTest::Operators operators
	);
	// ================================================================================================
	static void runWorld(
		flecs::world& world, World type, UnitTest& test, const ModuleImporter& importer
	);
	// ================================================================================================
	static void runUnitTest(flecs::entity e, UnitTest& test);
	// ================================================================================================
	static void runUnitTestIncomplete(flecs::entity e, UnitTest& test);
	// ================================================================================================
	static void applyConfiguration(
		flecs::world& world, UnitTest::WorldConfiguration configuration
	);

	// ================================================================================================
	static std::optional<flecs::system> getSystemByName(
		const flecs::world& world, const std::string& systemName
	);


};

