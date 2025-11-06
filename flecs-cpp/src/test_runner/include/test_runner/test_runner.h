#pragma once

#include <flecs.h>

#include <string>
#include <functional>
#include <optional>



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
    struct Passed {

    };

    /*struct Result {
    std::string name;
    bool passed = false;
    };*/

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
  

  TestRunner(flecs::world& world);


  // TODO: move to UnitTest project and instead use http port to get results and add entities
  /*std::vector<UnitTest::Result> getAllTestResults();*/

  template <typename... Args>
  static void addTestEntity(flecs::world& world, const char* name, Args&&... args) {
    world.entity(name)
      .set<UnitTest>({ std::forward<Args>(args)... });
  }
  

  static void initialize(flecs::world& world, std::function<void(flecs::world&)> modulesProvider);

  static void setLogLevel(LogLevel logLevel) {
    _logLevel = logLevel;
  }
private:
  static void registerReflection(flecs::world& world);
  static  bool compareWorlds(flecs::world& world1, flecs::world& world2);
  static void runSystem(flecs::world& world, const SystemInvocation& sys);

  static LogLevel _logLevel;
  static std::function<void(flecs::world&)> _modulesProvider;
};






