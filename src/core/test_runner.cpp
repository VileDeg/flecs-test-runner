#pragma once

#include <test_runner/test_runner.h>

#include <string>
#include <iostream>
#include <thread>

#include "reflection.h"

using LogLevel = TestRunner::LogLevel;

static LogLevel s_LogLevel = LogLevel::WARN;

#define pfatal (s_LogLevel >= LogLevel::FATAL ? std::cerr : nullStream)
#define perror (s_LogLevel >= LogLevel::ERROR ? std::cerr : nullStream)
#define pwarn  (s_LogLevel >= LogLevel::WARN  ? std::cout : nullStream)
#define pinfo  (s_LogLevel >= LogLevel::INFO  ? std::cout : nullStream)
#define ptrace (s_LogLevel >= LogLevel::TRACE ? std::cout : nullStream)

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

void TestRunner::setLogLevel(LogLevel logLevel) {
  s_LogLevel = logLevel;
}

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

static std::optional<flecs::system> getSystemByName(
  const flecs::world& world, const std::string& systemName
) {
  // Lookup the system by name
  flecs::entity systemEntity = world.lookup(systemName.c_str());

  if(!systemEntity) {
    perror << "ERROR: System '" << systemName << "' not found!\n";
    return std::nullopt;
  }

  // Check if it's a system
  if(!systemEntity.has(flecs::System) || !world.system(systemEntity)) {
    perror << "ERROR: Entity '" << systemName << "' is not a system!\n";
    return std::nullopt;
  }

  return world.system(systemEntity);
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
  // Create EXPECTED world
  flecs::world worldExpected;
  // Import testable systems & components
 

  for(auto& sys : test.systems) {
    auto m = matchModuleFromSystemPath(sys.name);
    if(m.has_value()) {
      _moduleRegistry[*m](worldActual);
      _moduleRegistry[*m](worldExpected);
    }
  }

  // Set up world
  worldActual.script_run("ScriptActual", test.scriptActual.c_str());

  for(auto& sys : test.systems) {
    // Run system
    for(int i = 0; i < sys.timesToRun; ++i) {
      auto system = getSystemByName(worldActual, sys.name);
      if(!system.has_value()) {
        continue;
      }

      pinfo << "[" << i << "] Running system '" << sys.name << "'\n";
      system->run();
    }
  }

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


void TestRunner::runUnitTestIncomplete(flecs::entity e, UnitTest& test)
{
  pinfo << "Running test (Incomplete): " << test.name << "\n";

  auto status = test.validate();
  if(status.has_value()) {
    perror << "Failed to run test " << test.name << ": " << *status << "\n";
    e.set<UnitTest::Executed>({ *status });
    return;
  }

  // Create ACTUAL world
  flecs::world worldActual;
 
  for(auto& sys : test.systems) {
    auto m = matchModuleFromSystemPath(sys.name);
    if(m.has_value()) {
      _moduleRegistry[*m](worldActual);
    }
  }

  // Set up world
  worldActual.script_run("ScriptActual", test.scriptActual.c_str());

  for(auto& sys : test.systems) {
    // Run system
    for(int i = 0; i < sys.timesToRun; ++i) {
      auto system = getSystemByName(worldActual, sys.name);
      if(!system.has_value()) {
        continue;
      }

      pinfo << "[" << i << "] Running system '" << sys.name << "'\n";
      system->run();
    }
  }

  std::string worldExpectedSerialized = worldActual.to_json();
  e.set<UnitTest::Executed>({ "OK", worldExpectedSerialized });
}

// ================================================================================================

std::optional<std::string> TestRunner::matchModuleFromSystemPath(std::string systemFullPath) {
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
      pinfo << "[Info] System '" << systemFullPath 
        << "' belongs to module '" << current_path << "\n";
      //_moduleRegistry[current_path](world);
      return current_path;
    }
  }

  pwarn << "[Error] Could not find a registered module for system path: '" 
    << systemFullPath << "'\n";
  return std::nullopt;
}



// ================================================================================================
TestRunner::TestRunner(flecs::world& world) {
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

  world.component<UnitTest::Ready>();
  world.component<UnitTest::Incomplete>();

  world.component<UnitTest::Passed>();

  world.component<UnitTest::Executed>()
    .member<std::string>("statusMessage")
    .member<std::string>("worldExpectedSerialized");

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

  world.system<UnitTest>("TestRunner")
    .kind(flecs::OnUpdate)
    .multi_threaded()
    .with<UnitTest::Ready>()
    .without<UnitTest::Executed>()
    .without<UnitTest::Incomplete>()
    .each([this](flecs::entity e, UnitTest& test) {
      //std::thread t([this, e, &test]() {
      runUnitTest(e, test);
      //});
            
      //if(t.joinable()) {
      //t.join();
      //}
    });

  world.system<UnitTest>("TestRunnerIncomplete")
    .kind(flecs::OnUpdate)
    .multi_threaded()
    .with<UnitTest::Ready>()
    .without<UnitTest::Executed>()
    .with<UnitTest::Incomplete>()
    .each([this](flecs::entity e, UnitTest& test) {
      //std::thread t([this, e, &test]() {
      runUnitTestIncomplete(e, test);
      //});

      //if(t.joinable()) {
      //t.join();
      //}
  });
}


