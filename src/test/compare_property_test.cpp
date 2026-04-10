#include "common.h"

template <typename T>
struct OpTestCase {
	struct Components {
		T a, b;
	};

	OperatorType op;
	Components components;
	Operator::Path propertyPath;
	bool expectedResult;
	UnitTest::Systems systems = {};
};

template <typename TestComponent>
class ComparePropertyTest 
	: public ::testing::Test, 
		public ::testing::WithParamInterface<OpTestCase<TestComponent>> 
{
public:
	using ComponentType = TestComponent;

protected:
	void SetUp() override {
		tr::setLogLevel(tr::LogLevel::TRACE);

		InitWorld(_ecs);
	}

	void InitWorld(flecs::world& ecs) {
		TestRunner::initialize<movement::module>(ecs);
	}

	flecs::entity AddEntity(std::optional<std::string> name = std::nullopt) {
		auto e = name ? _ecs.entity(name->c_str()) : _ecs.entity();
		e.add<TestComponent>();
		return e;
	}

	flecs::entity AddEntity(const TestComponent& comp, std::optional<std::string> name = std::nullopt) {
		auto e = name ? _ecs.entity(name->c_str()) : _ecs.entity();
		e.set<TestComponent>(comp);
		return e;
	}

	flecs::entity AddEntity(flecs::world& world, std::optional<std::string> name = std::nullopt) {
		auto e = name ? world.entity(name->c_str()) : world.entity();
		e.add<TestComponent>();
		return e;
	}

	using PropertyPair = std::pair<ResolvedProperty, ResolvedProperty>;

	ResolvedProperty resolveProperty(const flecs::entity& entity, const std::string& propertyPath) {
		return tri::resolveProperty(_ecs, entity, tri::getComponentByName(_ecs, COMP_NAME), propertyPath);
	}

	PropertyPair createProperties(
		const TestComponent& compA, 
		const TestComponent& compB, 
		const std::string& propertyPath
	) {
		auto a = AddEntity(compA);
		auto b = AddEntity(compB);

		ResolvedProperty propA = resolveProperty(a, propertyPath);
		ResolvedProperty propB = resolveProperty(b, propertyPath);

		return { propA, propB };
	}

	PropertyPair createEqualProperties(const TestComponent& comp, const std::string& propertyPath) {
		return createProperties(comp, comp, propertyPath);
	}

	PropertyPair createEqualProperties(const std::string& propertyPath) {
		return createEqualProperties({}, propertyPath);
	}

	void checkCompareProperties(
		const ComponentType& compA, 
		const ComponentType& compB,
		const Operator::Path& propertyPath, 
		OperatorType op,
		bool comparisonResult = true
	) {
		auto [init, exp] = createProperties(compA, compB, propertyPath);

		ASSERT_EQ(
			tri::compareProperty(_ecs, init.type, init.ptr, exp.ptr, op),
			comparisonResult
		);
	}

	void runSystems(const tri::UnitTest::Systems& systems) {
		tri::runSystems(_ecs, systems);
	}

	void runSystemsCheckCompareProperties(
		const ComponentType& initial,
		const ComponentType& expected,
		const Operator::Path& propertyPath,
		OperatorType op,
		const tri::UnitTest::Systems& systems,
		bool comparisonResult = true
	) {
		auto a = AddEntity(initial);

		runSystems(systems);

		ResolvedProperty propInitial = resolveProperty(a, propertyPath);

		auto b = AddEntity(expected);
		ResolvedProperty propExpected = resolveProperty(b, propertyPath);
		
		ASSERT_EQ(propInitial.type, propExpected.type);
		ASSERT_EQ(
			tri::compareProperty(_ecs, propInitial.type, propInitial.ptr, propExpected.ptr, op),
			comparisonResult
		);
	}

	inline static const std::string COMP_NAME			= TypeInfo<TestComponent>::getName();
	inline static const std::string COMP_NAME_SEP = COMP_NAME + tri::UnitTest::Operator::Path::DELIMETER;

	flecs::world _ecs;
};

using ComparePropertyTest_Speed = ComparePropertyTest<movement::Speed>;
using ComparePropertyTest_PositionVector = ComparePropertyTest<movement::PositionVector>;
using ComparePropertyTest_PositionArray = ComparePropertyTest<movement::PositionArray>;

using PositionVectorTest = ComparePropertyTest<PositionVector>;

using TestCaseVector = OpTestCase<PositionVector>;

using PositionArrayTest = ComparePropertyTest<PositionArray>;

TEST_P(PositionVectorTest, CompareProperties) {
	const auto& param = GetParam();

	if (param.systems.empty()) {
		checkCompareProperties(
			param.components.a,
			param.components.b,
			param.propertyPath,
			param.op,
			param.expectedResult
		);
	} else {
		runSystemsCheckCompareProperties(
			param.components.a, 
			param.components.b, 
			param.propertyPath, 
			param.op, 
			param.systems,
			param.expectedResult
		);
	}
}

static const Position pos11 = Position{ 1, 1 };
static const Position pos22 = Position{ 2, 2 };
static const Position pos4242 = Position{ 42, 42 };

static const PositionVector posVector_11_4242 = { { pos11, pos4242 } };
static const PositionVector posVector_22_4242 = { { pos22, pos4242 } };

static const TestCaseVector::Components posVector_11_4242_Same		= { posVector_11_4242, posVector_11_4242 };
static const TestCaseVector::Components posVector_11_4242_22_4242 = { posVector_11_4242, posVector_22_4242 };

static const char* move_Speed_Vector = "movement::module::moveVector_Speed";

template <typename T>
OpTestCase<T> MakeTestCase(
	OpType op, 
	const typename OpTestCase<T>::Components& comps,
	std::string path, 
	bool expected
) {
	return { op, comps, std::move(path), expected };
}

template <typename T>
auto GenerateEqualityTests(
	const typename OpTestCase<T>::Components& comps,
	const std::vector<std::string>& paths
) {
	std::vector<OpTestCase<T>> cases;
	cases.reserve(paths.size());
	for (const auto& path : paths) {
		cases.push_back({ OpType::EQ, comps, path, true });   // Expected match
		cases.push_back({ OpType::LT, comps, path, false });  // Expected fail for same values
	}
	return cases;
}

template <typename T>
auto GenerateCompareTests(
	const typename OpTestCase<T>::Components& comps,
	const std::vector<std::string>& paths,
	OpType type,
	bool result = true
) {
	std::vector<OpTestCase<T>> cases;
	cases.reserve(paths.size());
	for (const auto& path : paths) {
		cases.push_back({ type, comps, path, result });
		cases.push_back({ OpType::EQ, comps, path, false });
	}
	return cases;
}

const std::vector<std::string> testPaths = { "", "data", "data/0", "data/0/x" };

INSTANTIATE_TEST_SUITE_P(
	EqualityTests,
	PositionVectorTest,
	testing::ValuesIn(
		GenerateEqualityTests<PositionVector>(posVector_11_4242_Same, testPaths)
	)
);

INSTANTIATE_TEST_SUITE_P(
	InequalityTests,
	PositionVectorTest,
	testing::ValuesIn(
		MergeTests<OpTestCase<PositionVector>>(
			GenerateCompareTests<PositionVector>(
				posVector_11_4242_22_4242, 
				testPaths, 
				OpType::LT
			),
			GenerateCompareTests<PositionVector>(
				posVector_11_4242_22_4242, 
				testPaths, 
				OpType::GT, 
				false
			)
		)
	)
);

