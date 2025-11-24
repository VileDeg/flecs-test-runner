#pragma once

#include <flecs.h>

#include <string>
#include <functional>
#include <optional>
#include <map>

struct TestRunner {
  /* TODO:
  * Maybe suppor system triggered by an event?
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
    // Tags
    struct Executed {
      std::string statusMessage;
    };
    struct Passed {};

    std::string name;

    std::vector<SystemInvocation> systems;

    std::string scriptActual;
    std::string scriptExpected;

    std::optional<std::string> validate() const {
      std::optional<std::string> statusMessage = std::nullopt;
      if(name.empty()) {
        statusMessage = "Name is empty";
      } else if(systems.empty()) {
        statusMessage = "No systems to run";
      } else if(scriptActual.empty()) {
        statusMessage = "Actual script is empty";
      } else if(scriptExpected.empty()) {
        statusMessage = "Expected script is empty";
      }
      return statusMessage;
    }
  };

  // Module tag
  struct TestableModule {
  };

  TestRunner(flecs::world& world);


  // TODO: move to UnitTest project and instead use http port to get results and add entities
  /*std::vector<UnitTest::Result> getAllTestResults();*/

  template <typename... Args>
  static void addTestEntity(flecs::world& world, const char* name, Args&&... args) {
    world.entity(name)
      .set<UnitTest>({ std::forward<Args>(args)... });
  }

  static void initialize(flecs::world& world, std::function<void(flecs::world&)> modulesProvider);

  // This handles the difference between Windows (MSVC) and Linux/Mac (GCC/Clang)
#if defined(_MSC_VER) // Windows
#include <typeinfo>
  template <typename T>
  static std::string get_type_name() {
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
  static std::string get_type_name() {
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
    // Automatically get the string name from the type T
    std::string name = get_type_name<T>();

    // Store in map to allow for later import in test world
    _moduleRegistry[name] = [](flecs::world& w) {
      w.import<T>();
    };

    // Import module to make available for query
    auto m = world.import<T>();
    m.add<TestableModule>();

    std::cout << "Registered module: " << name << std::endl;
  }

  static void setLogLevel(LogLevel logLevel) {
    _logLevel = logLevel;
  }
private:
  static void registerReflection(flecs::world& world);
  static bool compareWorlds(flecs::world& world1, flecs::world& world2);
  static void runUnitTest(flecs::entity e, UnitTest& test);
  static std::optional<std::string> importBySystemName(flecs::world& world, std::string systemFullPath);
  static void runSystem(flecs::world& world, const SystemInvocation& sys);

  static LogLevel _logLevel;
  static std::function<void(flecs::world&)> _modulesProvider;
  
  static std::map<std::string, void(*)(flecs::world&)> _moduleRegistry;
};






