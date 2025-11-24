#pragma once

#include <test_runner/test_runner.h>

#include <string>
#include <iostream>
#include <thread>

#include "reflection.h"

using LogLevel = TestRunner::LogLevel;

std::function<void(flecs::world&)> TestRunner::_modulesProvider;
std::map<std::string, void(*)(flecs::world&)> TestRunner::_moduleRegistry;

LogLevel TestRunner::_logLevel = LogLevel::WARN;

#define pfatal (TestRunner::_logLevel >= LogLevel::FATAL ? std::cerr : nullStream)
#define perror (TestRunner::_logLevel >= LogLevel::ERROR ? std::cerr : nullStream)
#define pwarn  (TestRunner::_logLevel >= LogLevel::WARN  ? std::cout : nullStream)
#define pinfo  (TestRunner::_logLevel >= LogLevel::INFO  ? std::cout : nullStream)
#define ptrace (TestRunner::_logLevel >= LogLevel::TRACE ? std::cout : nullStream)

class NullBuffer : public std::streambuf {
protected:
    int overflow(int c) override { return c; }
};

class NullStream : public std::ostream {
public:
    NullStream() : std::ostream(&m_sb) {}
private:
    NullBuffer m_sb;
};
    
static NullStream nullStream;


// ================================================================================================
bool TestRunner::compareWorlds(flecs::world& world1, flecs::world& world2) {
    pinfo << "\nWorld Comparison (using serialization):\n";
    pinfo << "========================================\n";

    // Serialize both worlds to JSON
    flecs::string json1 = world1.to_json();
    flecs::string json2 = world2.to_json();

    pinfo << "\nWorld 1 JSON:\n";
    pinfo << json1 << "\n";

    pinfo << "\nWorld 2 JSON:\n";
    pinfo << json2 << "\n";

    // Compare the serialized representations
    bool matches = (json1 == json2);

    pinfo << "\n";
    if(matches) {
        pinfo << "WORLDS MATCH!\n";
        pinfo << "The serialized JSON representations are identical.\n";
    } else {
        pinfo << "WORLDS DO NOT MATCH!\n";
        pinfo << "The serialized JSON representations differ.\n";

        // Show size difference as a hint
        pinfo << "\nJSON sizes: World1=" << json1.size()
            << " bytes, World2=" << json2.size() << " bytes\n";
    }

    return matches;
}

void TestRunner::runUnitTest(flecs::entity e, UnitTest& test)
{
  pinfo << "Running test: " << test.name << "\n";

  auto status = test.validate();
  if(status.has_value()) {
      perror << "Failed to run test " << test.name << ": " << *status << "\n";
      e.set<UnitTest::Executed>({ *status });
      return;
  }

  // Create ACTUAL world
  flecs::world worldActual;
  // Import testable systems & components
  //_modulesProvider(worldActual);

  worldActual.script_run("ScriptActual", test.scriptActual.c_str());

  // TODO: run each system invocation in separate thread?
  // Otherwise system blocks the whole flecs application

  // Run each system invocation in a separate thread and wait for them to finish.
  // NOTE: copying 'sys' into the lambda to avoid lifetime issues.

  std::vector<std::string> usedModules;

  for(auto& sys : test.systems) {
    auto maybeModule = importBySystemName(worldActual, sys.name);
    if(maybeModule.has_value()) {
      usedModules.push_back(*maybeModule);
    }
  }

  for(auto& sys : test.systems) {
      runSystem(worldActual, sys);
  }

  // Create EXPECTED world
  flecs::world worldExpected;
  // Import same modules
  for(auto& m : usedModules) {
    _moduleRegistry[m](worldExpected);
  }
  //_modulesProvider(worldExpected);
  worldExpected.script_run("ScriptExpected", test.scriptExpected.c_str());

  std::string statusMessage;
  if(compareWorlds(worldActual, worldExpected)) {
      e.add<UnitTest::Passed>();
      statusMessage = "OK";
  } else {
      statusMessage = "Worlds do not match. Actual vs. expected";
  }
  e.set<UnitTest::Executed>({ statusMessage });
}

// ================================================================================================
void TestRunner::registerReflection(flecs::world& world) {
  // TODO: reflection for string and vector built-in by flecs?
  world.component<std::string>()
      .opaque(flecs::String) // Opaque type that maps to string
      .serialize([](const flecs::serializer* s, const std::string* data) {
      const char* str = data->c_str();
      return s->value(flecs::String, &str); // Forward to serializer
  })
      .assign_string([](std::string* data, const char* value) {
      *data = value; // Assign new value to std::string
  });

  world.component<UnitTest::Passed>();
        
  world.component<UnitTest::Executed>()
      .member<std::string>("statusMessage");

  world.component<SystemInvocation>()
      .member<std::string>("name")
      .member<int>("timesToRun");

  // Register reflection for std::vector<int>
  world.component<std::vector<SystemInvocation>>()
      .opaque(reflection::std_vector_support<SystemInvocation>);


  // TODO: maybe use some template magic to add reflection?
  world.component<UnitTest>()
      .member<std::string>("name")
      .member<std::vector<SystemInvocation>>("systems")
      .member<std::string>("scriptActual")
      .member<std::string>("scriptExpected");

  world.component<TestableModule>();
}

std::optional<std::string> TestRunner::importBySystemName(flecs::world& world, std::string systemFullPath) {
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
    if (_moduleRegistry.count(current_path)) {
      std::cout << "[Info] System '" << systemFullPath 
        << "' belongs to module '" << current_path << "'. Importing...\n";
      _moduleRegistry[current_path](world);
      return current_path;
    }
  }

  std::cerr << "[Error] Could not find a registered module for system path: '" 
    << systemFullPath << "'\n";
  return std::nullopt;
}

// ================================================================================================
void TestRunner::runSystem(flecs::world& world, const SystemInvocation& sys) {
  //importBySystemName(world, sys.name);

  // Lookup the system by name
  flecs::entity systemEntity = world.lookup(sys.name.c_str());

  if(!systemEntity) {
      perror << "ERROR: System '" << sys.name << "' not found!\n";
      return;
  }

  // Check if it's a system
  if(!systemEntity.has(flecs::System) || !world.system(systemEntity)) {
      perror << "ERROR: Entity '" << sys.name << "' is not a system!\n";
      return;
  }

  flecs::system system = world.system(systemEntity);

  // Run system
  for(int i = 0; i < sys.timesToRun; ++i) {
      system.run();
      pinfo << "[" << i << "] Running system '" << sys.name << "'\n";
  }
}



// ================================================================================================
void TestRunner:: initialize(
  flecs::world& world, 
  std::function<void(flecs::world&)> modulesProvider
) {
  _modulesProvider = modulesProvider;
  world.import<TestRunner>();
  modulesProvider(world);

  /* TODO:
  * Maybe automatically assign kind 0 to all systems here? 
  */
}


// ================================================================================================
TestRunner::TestRunner(flecs::world& world) {
    registerReflection(world);

    world.system<UnitTest>("TestRunner")
        .kind(flecs::OnUpdate)
        .multi_threaded()
        .without<UnitTest::Executed>()
        .each([this](flecs::entity e, UnitTest& test) {
            std::thread t([this, e, &test]() {
                runUnitTest(e, test);
            });
            
            if(t.joinable()) t.join();
        });
}


