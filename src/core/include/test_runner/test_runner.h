#pragma once

#include <flecs.h>

#include <string>
#include <functional>
#include <optional>
#include <map>
#include <stdexcept>
#include <algorithm>


// Equal (==)
template<typename T, typename = void>
struct has_eq : std::false_type {};

template<typename T>
struct has_eq<T, std::void_t<decltype(std::declval<T>() == std::declval<T>())>> : std::true_type {};

// Not Equal (!==)
template<typename T, typename = void>
struct has_neq : std::false_type {};

template<typename T>
struct has_neq<T, std::void_t<decltype(std::declval<T>() == std::declval<T>())>> : std::true_type {};

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


class TestRunner {
public:
	//using WorldCallback = void(*)(flecs::world&);

	using WorldCallback = std::function<void(flecs::world&)>;

	using ModuleImporterMap =
		//std::vector<WorldCallback>;
		
		std::map<std::string, WorldCallback>;
	using TypeImporterMap = 
		std::vector<WorldCallback>;
		//std::map<std::string, WorldCallback>;

  template <typename Derived>
  class AutoPrefixedError : public std::runtime_error {
  public:
    explicit AutoPrefixedError(const std::string& message) 
      : std::runtime_error(generatePrefix() + message) {}

  private:
    // Helper to get the nice "Namespace::Class" string
    static std::string generatePrefix() {
      return TestRunner::getTypeName<Derived>();
    }
  };

  struct Error : public AutoPrefixedError<Error> {
    using AutoPrefixedError::AutoPrefixedError;
  };

  /* TODO:
  * Maybe support system triggered by an event?
  * */
  enum class LogLevel {
    FATAL = 0,
    ERROR,
    WARN,
    INFO,
    TRACE,
  };

  struct SystemInvocation {
    std::string name;
    int timesToRun;
  };

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

			using Path = std::string;

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
      if(name.empty()) {
        statusMessage = "Name is empty";
      } else if(systems.empty()) {
        statusMessage = "No systems to run";
      } else if(initialConfiguration.empty()) {
        statusMessage = "Actual script is empty";
      } else if(complete && expectedConfiguration.empty()) {
        statusMessage = "Expected script is empty";
      }
      return statusMessage;
    }

    void normalizeSystemNames();
    void runSystems(flecs::world& world);

    std::vector<std::string> getSystemNames();
  };

  // Module tag
  struct TestableModule {
  };

  TestRunner(flecs::world& world);

  template <typename... Args>
  static void addTestEntity(flecs::world& world, const char* name, Args&&... args) {
    world.entity(name)
      .set<UnitTest>({ std::forward<Args>(args)... });
  }
  
  // This handles the difference between Windows (MSVC) and Linux/Mac (GCC/Clang)
#if defined(_MSC_VER) // Windows
#include <typeinfo>
  template <typename T>
  static std::string getTypeName() {
    std::string name = typeid(T).name();
    // MSVC returns "struct MyModule" or "class MyModule". 
    // We remove the prefix to get just "MyModule".
    size_t space_pos = name.find(' ');
    if (space_pos != std::string::npos) {
      return name.substr(space_pos + 1);
    }
    return name;
  }
#else // Linux / macOS / GCC / Clang
#include <cxxabi.h>
#include <typeinfo>
#include <memory>
  template <typename T>
  static std::string getTypeName() {
      int status;
      // specific GCC/Clang API to "demangle" the weird internal names
      std::unique_ptr<char, void(*)(void*)> res {
          abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status),
          std::free
      };
      return (status == 0) ? res.get() : typeid(T).name();
  }
#endif

	
	

	struct TypeMetadata {
		using ComparisonFunc = bool (*)(const void*, const void*);

		struct ComparisonFuncs {
			ComparisonFunc eq  = nullptr;
			ComparisonFunc neq = nullptr;
			ComparisonFunc lt  = nullptr;
			ComparisonFunc lte = nullptr;
			ComparisonFunc gt  = nullptr;
			ComparisonFunc gte = nullptr;

			friend bool operator==(const ComparisonFuncs& lhs, const ComparisonFuncs& rhs) {
				return 
					lhs.eq == rhs.eq 
					&& lhs.neq == rhs.neq
					&& lhs.lt == rhs.lt
					&& lhs.lte == rhs.lte
					&& lhs.gt == rhs.gt
					&& lhs.gte == rhs.gte
					;
			}
		};

		ComparisonFuncs funcs;
	};

	

	
	class TypeRegistry {
	public:

		template <typename... Ts>
		void add() {

			// Use a fold expression to populate the vector
			(_registrars.push_back([](flecs::world& world) {
				registerTypeImpl<Ts>(world);
				}), ...);
		}

		const std::vector<WorldCallback>& getRegistrars() const {
			return _registrars;


			//return registrars;
		}

		void applyAll(flecs::world& world) const {
			std::for_each(_registrars.begin(), _registrars.end(), [&](const auto& f) { f(world); });
		}

	private:
		template <typename T>
		static void registerTypeImpl(flecs::world& world) {
			// TODO: fisrt check if component is already registered
					// Maybe don't wanna register here
			auto comp = world.component<T>();

			TypeMetadata meta{};

			if constexpr (has_eq_v<T>) {
				meta.funcs.eq =
					[](const void* a, const void* b) {
						return *static_cast<const T*>(a) == *static_cast<const T*>(b);
					};
			}
			if constexpr (has_neq_v<T>) {
				meta.funcs.neq =
					[](const void* a, const void* b) {
					  return *static_cast<const T*>(a) != *static_cast<const T*>(b);
					};
			}
			if constexpr (has_lt_v<T>) {
				meta.funcs.lt =
					[](const void* a, const void* b) {
					  return *static_cast<const T*>(a) < *static_cast<const T*>(b);
					};
			}
			if constexpr (has_lte_v<T>) {
				meta.funcs.lte =
					[](const void* a, const void* b) {
					  return *static_cast<const T*>(a) <= *static_cast<const T*>(b);
					};
			}
			if constexpr (has_gt_v<T>) {
				meta.funcs.gt =
					[](const void* a, const void* b) {
					  return *static_cast<const T*>(a) > *static_cast<const T*>(b);
					};
			}
			if constexpr (has_gte_v<T>) {
				meta.funcs.gte =
					[](const void* a, const void* b) {
					  return *static_cast<const T*>(a) >= *static_cast<const T*>(b);
					};
			}

			comp.set<TypeMetadata>(meta);

			//} else {
				// Optional: Trigger a static_assert if you want to force 
				// the user to only call this on valid types.
				// static_assert(IsComparable<T>, "Type T does not support operator<");
			//}
		}

		std::vector<WorldCallback> _registrars;
	};

	
	// TODO: must be called after modules are registered?
	template <typename... Ts>
	static void registerTypes(flecs::world& world) { // flecs::world& world

		_typeRegistry.add<Ts...>();

		_typeRegistry.applyAll(world);

		//(registerTypeImpl<T>(world), ...);

	/*	std::string name = getTypeName<T>();

		std::cout << "Registered type: " << name << std::endl;*/
	}




	// TODO: make private
	static void initialize(flecs::world& world) {
		if (_initialized) {
			return;
		}
		// Registered test runner itself if not registered
		world.import<TestRunner>();

		// Register primitive types
		registerTypes<
			bool, char,
			int8_t, int16_t, int32_t, int64_t,
			uint8_t, uint16_t, uint32_t, uint64_t,
			float, double
		>(world);

		_initialized = true;
	}

  template <typename T>
  static void registerModule(flecs::world& world) {
		initialize(world);

    // Import module to make available for query
    auto m = world.import<T>();
    m.add<TestableModule>();

    std::string name = getTypeName<T>();

    std::cout << "Registered module: " << name << std::endl;
    // Store in map to allow for later import in test world
    _moduleImporterRegistry[name] = [](flecs::world& w) {
      w.import<T>();
    };
  }
  
  static void setLogLevel(LogLevel logLevel);

private:
  static bool compareWorlds(flecs::world& world1, flecs::world& world2);
  static void runUnitTest(flecs::entity e, UnitTest& test);
  static void runUnitTestIncomplete(flecs::entity e, UnitTest& test);

  /**
  * Maps module name to importer function.
  */
  inline static ModuleImporterMap _moduleImporterRegistry; // TODO: use function object instead of func ptr?

	//inline static TypeImporterMap _typeImporterRegistry;

	
	inline static TypeRegistry _typeRegistry;


	inline static bool _initialized = false;
};

