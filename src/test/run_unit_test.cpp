#include "common.h"

struct TestCaseParam {
	UnitTest test;
	bool result;
};

class RunUnitTest
	: public ::testing::Test
	,	public ::testing::WithParamInterface<TestCaseParam>
{
protected:
	void SetUp() override {
		tr::setLogLevel(tr::LogLevel::TRACE);
		InitWorld(_ecs);
	}

	void InitWorld(flecs::world& ecs) {
		TestRunner::initialize<movement::module>(ecs);
	}

	flecs::world _ecs;
};

static const char* move_Speed_Vector = "movement::module::moveVector_Speed";

static const std::string POS_VECTOR = TypeInfo<PositionVector>::getName();
static const std::string POS_VECTOR_DATA = "data";

const std::vector<std::string> testPaths = { "", "data", "data/0", "data/0/x" };

static const std::string ENTITY_NAME = "Aboba";

const char* entityInitial = R"(
{
	"name": "Aboba",
	"components" : {
		"movement.module.Speed": {
      "value": 1
    },
		"movement.module.PositionVector" : {
			"data": [
				{
					"x"	: 1,
					"y" : 1
				},
				{
					"x"	: 42,
					"y" : 42
				}
			]
		}
	}
}
)";

// Positions increased by speed
const char* entityExpected = R"(
{
	"name": "Aboba",
	"components" : {
		"movement.module.Speed": {
      "value": 1
    },
		"movement.module.PositionVector" : {
			"data": [
				{
					"x"	: 2,
					"y" : 2
				},
				{
					"x"	: 43,
					"y" : 43
				}
			]
		}
	}
}
)";

static std::string createPath(
	const std::string& entity, 
	const std::string& component = "",
	const std::string& subPath = ""
) {
	return entity + 
		(component.empty() ? "" : "/" + component) +
		(subPath.empty() ? "" : "/" + subPath);
}

struct OperatorResult {
	OperatorResult(OperatorType type)
		: type(type), result(true) {
	}
	OperatorResult(OperatorType type, bool r)
		: type(type), result(r) {
	}

	OperatorType type;
	bool result;
};

struct Terms {
	std::vector<std::string> paths;
	std::vector<OperatorResult> ops;
};

template <typename T = TestCaseParam>
static std::vector<T> generateTests(
	const Terms& terms
) {
	std::vector<T> generated;

	// Cartesian product of paths X operators
	for (const auto& path : terms.paths) {
		for (const auto& op : terms.ops) {
			generated.push_back(
				{
					UnitTest{
						"Test_" + path + "_" + std::to_string(static_cast<int>(op.type)),
						{{ move_Speed_Vector, 1 }},
						{ entityInitial },
						{ entityExpected },
						{{ path, op.type }}
					},
					op.result
				}
			);
		}
	}
	return generated;
}

const Terms terms_Entity = {
	{ 
		createPath(ENTITY_NAME) 
	},
	{ 
		OperatorType::EQ,
		{ OperatorType::NEQ, false }
	}
};

const Terms terms_Properties = {
	{
		createPath(ENTITY_NAME, POS_VECTOR),
		createPath(ENTITY_NAME, POS_VECTOR, POS_VECTOR_DATA),
		createPath(ENTITY_NAME, POS_VECTOR, POS_VECTOR_DATA + "/0"),
		createPath(ENTITY_NAME, POS_VECTOR, POS_VECTOR_DATA + "/0/x"),
	},
	{
		OperatorType::EQ,
		{ OperatorType::LT, false },
		{ OperatorType::LTE, true }
	}
};

TEST_P(RunUnitTest, Success) {
	const auto& param = GetParam();
	UnitTest test = param.test;

	ASSERT_EQ(
		tri::runUnitTest(_ecs, test),
		param.result
	);
}

INSTANTIATE_TEST_SUITE_P(
	RunUnitTests,
	RunUnitTest,
	testing::ValuesIn(
		MergeTests<TestCaseParam>(
			generateTests(terms_Entity),
			generateTests(terms_Properties)
		)
	)
);
