#pragma once

#include <test_runner/test_runner.h>
#include "test_runner_impl.h"

#include <string>
#include <iostream>
#include <algorithm>
#include <sstream>

#include <test_runner/common.h>

using namespace TestRunnerDetail;

// ================================================================================================
// TestRunner
// ================================================================================================

// ================================================================================================
void TestRunner::setLogLevel(LogLevel logLevel) {
  Log::setLevel(logLevel);
}

using UnitTest = TestRunnerImpl::UnitTest;
using SystemInvocation = TestRunnerImpl::SystemInvocation;
using TestableModule = TestRunner::TestableModule;

#define TEST_RUNNER_SYSTEM_NAME "TestRunner"
#define TEST_RUNNER_INCOMPLETE_SYSTEM_NAME TEST_RUNNER_SYSTEM_NAME "Incomplete"

// ================================================================================================
template <typename Elem, typename Vector = std::vector<Elem>>
flecs::opaque<Vector, Elem> vectorReflectionSupport(flecs::world& world) {
	return flecs::opaque<Vector, Elem>()
		.as_type(world.vector<Elem>())

		// Forward elements of std::vector value to serializer
		.serialize([](const flecs::serializer* s, const Vector* data) {
		for (const auto& el : *data) {
			s->value(el);
		}
		return 0;
			})

		// Return vector count
		.count([](const Vector* data) {
		return data->size();
			})

		// Resize contents of vector
		.resize([](Vector* data, size_t size) {
		data->resize(size);
			})

		// Ensure element exists, return pointer
		.ensure_element([](Vector* data, size_t elem) {
		if (data->size() <= elem) {
			data->resize(elem + 1);
		}

		return &data->data()[elem];
			});
}

// ================================================================================================
TestRunner::TestRunner(flecs::world& world) {
	using Operator = UnitTest::Operator;

	// String
	world.component<std::string>()
    .opaque(flecs::String)
    .serialize([](const flecs::serializer* s, const std::string* data) {
			const char* str = data->c_str();
			return s->value(flecs::String, &str);
		})
    .assign_string([](std::string* data, const char* value) {
			*data = value;
		});
	world.component<std::vector<std::string>>()
		.opaque(vectorReflectionSupport<std::string>);


	// Tags
	world.component<TestRunnerImpl::TestedEntity>();

  world.component<UnitTest::Ready>();
  world.component<UnitTest::Passed>();
  world.component<UnitTest::Executed>()
    .member<std::string>("statusMessage");
  world.component<UnitTest::Incomplete>()
    .member<std::string>("worldExpectedSerialized");
  
	// Operator
	world.component<Operator::Path>()
		.member<std::string>("path");
	world.component<OperatorType>()
		.constant("EQ", OperatorType::EQ)
		.constant("NEQ", OperatorType::NEQ)
		.constant("LT", OperatorType::LT)
		.constant("LTE", OperatorType::LTE)
		.constant("GT", OperatorType::GT)
		.constant("GTE", OperatorType::GTE);
	world.component<UnitTest::Operator>()
		.member<Operator::Path>("path")
		.member<OperatorType>("type");
	world.component<std::vector<UnitTest::Operator>>()
		.opaque(vectorReflectionSupport<UnitTest::Operator>);
	world.component<SupportedOperators>()
		.member<bool>("equals")
		.member<bool>("cmp");

	// System
  world.component<SystemInvocation>()
    .member<std::string>("name")
    .member<int>("timesToRun");

  world.component<std::vector<SystemInvocation>>()
    .opaque(vectorReflectionSupport<SystemInvocation>);

  world.component<UnitTest>()
    .member<std::string>("name")
    .member<std::vector<SystemInvocation>>("systems")
    .member<std::vector<std::string>>("initialConfiguration")
    .member<std::vector<std::string>>("expectedConfiguration")
		.member<std::vector<Operator>>("operators");

  world.component<TestableModule>();

  world.system<UnitTest>(TEST_RUNNER_SYSTEM_NAME)
    .kind(flecs::OnUpdate)
    .with<UnitTest::Ready>()
    .without<UnitTest::Executed>()
    .without<UnitTest::Incomplete>()
    .each([this](flecs::entity e, UnitTest& test) { // TODO: why need capture this?
			std::ostringstream ss;
			try {
				auto world = e.world();
				Log::info() << "[" << TEST_RUNNER_SYSTEM_NAME << "] Running test: " << test.name;

				auto status = test.validate();
				if (status.has_value()) {
					std::ostringstream sse;
					sse << "Validation failed for test " << test.name << ": " << *status << "\n";
					throw Error(sse.str());
				}

				if (TestRunnerImpl::runUnitTest(world, test, ss)) {
					ss << "[Result]: PASS";
					e.add<UnitTest::Passed>();
				} else {
					ss << "[Result]: FAIL";
				}

				Log::info() << ss.str();
			} catch (const std::runtime_error& error) {
				ss << "Error thrown in system [" << TEST_RUNNER_SYSTEM_NAME << "]: " << error.what() << "\n";
				Log::error() << ss.str();
				Log::info() << "[Result]: FAIL";
			}
			e.set<UnitTest::Executed>({ ss.str()});
		});

  world.system<UnitTest>(TEST_RUNNER_INCOMPLETE_SYSTEM_NAME)
    .kind(flecs::OnUpdate)
    .with<UnitTest::Ready>()
    .without<UnitTest::Executed>()
    .with<UnitTest::Incomplete>()
    .each([this](flecs::entity e, UnitTest& test) {
			std::ostringstream ss;
			try {
				auto world = e.world();
				Log::info() << "[" << TEST_RUNNER_INCOMPLETE_SYSTEM_NAME << "] Running test: " << test.name;

				auto status = test.validate(false);
				if (status.has_value()) {
					std::ostringstream sse;
					sse << "Validation failed for test " << test.name << ": " << *status << "\n";
					throw Error(sse.str());
				}

				std::string expectedSerialized = TestRunnerImpl::runUnitTestIncomplete(world, test);

				ss << "OK";
				e.set<UnitTest::Incomplete>({ expectedSerialized });
				e.add<UnitTest::Passed>();

				Log::info() << "[Result]: PASS";
			} catch (const std::runtime_error& error) {
				ss << "Error thrown in system [" << TEST_RUNNER_INCOMPLETE_SYSTEM_NAME << "]: " << error.what() << "\n";
				Log::error() << ss.str();
				Log::info() << "[Result]: FAIL";
			}
			e.set<UnitTest::Executed>({ ss.str() });
	  });
}

