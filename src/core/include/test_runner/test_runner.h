#pragma once

#include <flecs.h>

#include <string>
#include <functional>
#include <optional>
#include <map>
#include <stdexcept>

class TestRunner {
public:
  using ModuleRegistry = std::map<std::string, void(*)(flecs::world&)>;

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
      std::string worldExpectedSerialized;
    };
    struct Passed {};

    struct Incomplete {};


    std::string name; // TODO: use some ID (hash) instead of name?

    std::vector<SystemInvocation> systems;

    std::string scriptActual;
    std::string scriptExpected;

    std::optional<std::string> validate(bool complete = true) const {
      std::optional<std::string> statusMessage = std::nullopt;
      if(name.empty()) {
        statusMessage = "Name is empty";
      } else if(systems.empty()) {
        statusMessage = "No systems to run";
      } else if(scriptActual.empty()) {
        statusMessage = "Actual script is empty";
      } else if(complete && scriptExpected.empty()) {
        statusMessage = "Expected script is empty";
      }
      return statusMessage;
    }

    // TODO: define elsewhere
    /*using ModuleRegistry = std::map<std::string, void(*)(flecs::world&)>;*/

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

  template <typename T>
  static void registerModule(flecs::world& world) {
    // Registered test runner itselft if not registered
    world.import<TestRunner>();


    // Import module to make available for query
    auto m = world.import<T>();
    m.add<TestableModule>();


    // Automatically get the string name from the type T
    std::string name = getTypeName<T>();

    std::cout << "Registered module: " << name << std::endl;
    // Store in map to allow for later import in test world
    _moduleRegistry[name] = [](flecs::world& w) {
      w.import<T>();
    };
  }

  static void setLogLevel(LogLevel logLevel);

private:

  

  /*static std::optional<std::string> matchModuleFromSystemPath(std::string systemFullPath);*/
  static bool compareWorlds(flecs::world& world1, flecs::world& world2);
  static void runUnitTest(flecs::entity e, UnitTest& test);
  static void runUnitTestIncomplete(flecs::entity e, UnitTest& test);

  inline static ModuleRegistry _moduleRegistry;
};

