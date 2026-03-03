#pragma once

#include <test_runner/test_runner.h>
#include "test_runner_impl.h"

#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>

#include "reflection.h"
#include <test_runner/common.h>

//using Log = TestRunner::Log;

using namespace TestRunnerDetail;


// ================================================================================================
// TestRunner
// ================================================================================================

// ================================================================================================
void TestRunner::setLogLevel(LogLevel logLevel) {
  Log::setLevel(logLevel);
}

/*
void TestRunner::initialize(flecs::world& world) {
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
*/

using UnitTest = TestRunnerImpl::UnitTest;
using SystemInvocation = TestRunnerImpl::SystemInvocation;
using TestableModule = TestRunner::TestableModule;


// ================================================================================================
TestRunner::TestRunner(flecs::world& world) {
	using Operator = UnitTest::Operator;

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

	world.component<std::vector<std::string>>()
		.opaque(reflection::std_vector_support<std::string>);

	// Tags
  world.component<UnitTest::Ready>();
  world.component<UnitTest::Passed>();
  world.component<UnitTest::Executed>()
    .member<std::string>("statusMessage");
  world.component<UnitTest::Incomplete>()
    .member<std::string>("worldExpectedSerialized");
  
	// Operator
	world.component<Operator::Path>()
		.member<std::string>("path");
	world.component<Operator::Type>()
		.constant("EQ", Operator::Type::EQ)
		.constant("NEQ", Operator::Type::NEQ)
		.constant("LT", Operator::Type::LT)
		.constant("LTE", Operator::Type::LTE)
		.constant("GT", Operator::Type::GT)
		.constant("GTE", Operator::Type::GTE);
	world.component<UnitTest::Operator>()
		.member<Operator::Path>("path")
		.member<Operator::Type>("type");
	world.component<std::vector<UnitTest::Operator>>()
		.opaque(reflection::std_vector_support<UnitTest::Operator>);

	// System
  world.component<SystemInvocation>()
    .member<std::string>("name")
    .member<int>("timesToRun");

  world.component<std::vector<SystemInvocation>>()
    .opaque(reflection::std_vector_support<SystemInvocation>);

  world.component<UnitTest>()
    .member<std::string>("name")
    .member<std::vector<SystemInvocation>>("systems")
    .member<std::vector<std::string>>("initialConfiguration")
    .member<std::vector<std::string>>("expectedConfiguration")
		.member<std::vector<Operator>>("operators");

  world.component<TestableModule>();

  world.system<UnitTest>("TestRunner")
    .kind(flecs::OnUpdate)
    .multi_threaded()
    .with<UnitTest::Ready>()
    .without<UnitTest::Executed>()
    .without<UnitTest::Incomplete>()
    .each([this](flecs::entity e, UnitTest& test) {
      try {
				Log::info() << "Running test: " << test.name;

				auto status = test.validate();
				if (status.has_value()) {
					std::stringstream ss;
					ss << "Failed to run test " << test.name << ": " << *status << "\n";
					e.set<UnitTest::Executed>({ *status });
					throw Error(ss.str());
				}

				std::ostringstream ss;
				if (TestRunnerImpl::runUnitTest(test, ss)) {
					Log::info() << "Test PASSED";
					e.add<UnitTest::Passed>();
				}
				e.set<UnitTest::Executed>({ ss.str()});
      }
      catch (const std::runtime_error& e) {
        Log::error() << "Error [" << __FUNCTION__ << "]: " << e.what() << "\n";
      }
    });

  world.system<UnitTest>("TestRunnerIncomplete")
    .kind(flecs::OnUpdate)
    .multi_threaded()
    .with<UnitTest::Ready>()
    .without<UnitTest::Executed>()
    .with<UnitTest::Incomplete>()
    .each([this](flecs::entity e, UnitTest& test) {
      try {
				Log::info() << "Running test (Incomplete): " << test.name << "\n";

				auto status = test.validate(false);
				if (status.has_value()) {
					std::stringstream ss;
					ss << "Failed to run test " << test.name << ": " << *status << "\n";
					e.set<UnitTest::Executed>({ *status });
					//e.set<UnitTest::Incomplete>({ "" });
					return;
					//throw Error(ss.str());
				}

				std::string expectedSerialized = TestRunnerImpl::runUnitTestIncomplete(test);

				e.set<UnitTest::Executed>({ "OK" });
				e.set<UnitTest::Incomplete>({ expectedSerialized });
				e.add<UnitTest::Passed>();

				Log::info() << "OK test (Incomplete): " << test.name << "\n";
      }
      catch (const std::runtime_error& e) {
        Log::error() << "Error [" << __FUNCTION__ << "]: " << e.what() << "\n";
      }
  });
}

