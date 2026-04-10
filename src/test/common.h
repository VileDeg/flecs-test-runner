#include <gtest/gtest.h>

#include <test_runner/test_runner.h>
#include <test_runner_impl.h>

#include <modules/test.h>

using namespace movement;

using tr = TestRunner;
using tri = TestRunnerImpl;

using UnitTest = tri::UnitTest;

using Operator = UnitTest::Operator;
using OpType = OperatorType;

static const std::string TEST_MODULE_NAME = "movement.module";

template <typename T>
struct TypeInfo;

#define TYPE_INFO(_Comp, _Name) \
template <>\
struct TypeInfo<_Comp> {\
    static const std::string& getName() {\
        static const std::string name = TEST_MODULE_NAME + "." + _Name;\
        return name;\
    }\
};
TYPE_INFO(movement::Speed, "Speed");
TYPE_INFO(movement::PositionVector, "PositionVector");
TYPE_INFO(movement::PositionArray, "PositionArray");

template <typename T, typename... Vecs>
std::vector<T> MergeTests(Vecs&&... groups) {
	std::vector<T> allCases;

	// Calculate total size upfront to avoid multiple reallocations
	size_t totalSize = (groups.size() + ... + 0);
	allCases.reserve(totalSize);

	// Fold expression to insert each vector
	([&allCases](auto& group) {
		allCases.insert(
			allCases.end(),
			std::make_move_iterator(group.begin()),
			std::make_move_iterator(group.end())
		);
		}(groups), ...);

	return allCases;
}
