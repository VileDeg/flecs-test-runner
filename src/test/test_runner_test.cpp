#include <gtest/gtest.h>

#include <test_runner/test_runner.h>
#include <test_runner_impl.h>

#include <modules/test.h>


#if 0
// Define a Test Suite (Calculator) and a specific Test (AddPositive)
TEST(Calculator, AddPositive) {
	flecs::world ecs;


	test_runner::initializeTests(ecs, [](flecs::world& world) {
		world.import<modules::movement>();
		});


	const char* scriptActual = R"(
        using modules.movement

        TestEntity {
            Position: {x: 10.0, y: 20.0}
            Velocity: {x: 1.0, y: 2.0}
        }
    )";

	const char* scriptExpected = R"(
        using modules.movement

        TestEntity {
            Position: {x: 11.0, y: 22.0}
            Velocity: {x: 1.0, y: 2.0}
        }
    )";


	std::vector<test_runner::SystemInvocation> sys = {
		{ "modules::movement::move", 1 }
	};


	test_runner::addTestEntity(
		ecs, "TestEntity0",
		sys,
		scriptActual,
		scriptExpected
	);

	std::cout << "Progress 0\n";
	ecs.progress();

	// Expect no tests to be run (got Executed tag)
	std::cout << "Progress 1\n";
	ecs.progress();



	ASSERT_FALSE(false);

	// TODO: check if test executed and passed
}
#endif
using tr = TestRunner;
using tri = TestRunnerImpl;

using ut = tri::UnitTest;

using Operator = ut::Operator;

template <typename T>
struct TypeInfo {
	inline static const std::string COMP_NAME;
};

#define TYPE_INFO(_Comp, _Name) \
template <>\
struct TypeInfo<_Comp> {\
	static constexpr char COMP_NAME[] = _Name;\
};
TYPE_INFO(movement::Speed, "movement.module.Speed");



// Define a test fixture class
template <typename TestComponent>
class TestFixture : public ::testing::Test {
protected:
	void SetUp() override {
		tr::setLogLevel(tr::LogLevel::WARN);

		InitWorld(ecs);
	}

	void TearDown() override {
		// teardown code comes here
	}

	void InitWorld(flecs::world& ecs) {
		TestRunner::initialize<movement::module>(ecs);
		TestRunner::registerTypes<movement::Speed>(ecs);
	}

	template <typename T = movement::Speed>
	flecs::entity AddEntity(std::optional<std::string> name = std::nullopt) {
		auto e = name ? ecs.entity(name->c_str()) : ecs.entity();
		e.add<T>();
		return e;
	}

	template <typename T = movement::Speed>
	flecs::entity AddEntity(flecs::world& world, std::optional<std::string> name = std::nullopt) {
		auto e = name ? world.entity(name->c_str()) : world.entity();
		e.add<T>();
		return e;
	}
		
	inline static const std::string COMP_NAME			= TypeInfo<TestComponent>::COMP_NAME;
	inline static const std::string COMP_NAME_SEP = COMP_NAME + tri::UnitTest::Operator::Path::DELIMETER;

	flecs::world ecs;
};


using TestFixture_Speed = TestFixture<movement::Speed>;

//TEST(Calculator, AddNegative) {
//    // Use an EXPECT_ macro to check the result
//    EXPECT_EQ(0, add(-5, 5));
//}


TEST_F(TestFixture_Speed, Basic) {
	auto e = AddEntity();

	ResolvedProperty prop;

	EXPECT_NO_THROW(
		prop = tri::resolveProperty(ecs, e, COMP_NAME_SEP);
	);

	ASSERT_TRUE(prop.ptr != nullptr);

	ASSERT_TRUE(prop.funcs.eq != nullptr);
	ASSERT_TRUE(prop.funcs.lt != nullptr);
	ASSERT_TRUE(prop.funcs.gt != nullptr);
}

TEST_F(TestFixture_Speed, EqualComponentSelf) {
	auto e = AddEntity();

	ResolvedProperty prop = tri::resolveProperty(ecs, e, COMP_NAME_SEP);

	ASSERT_TRUE(prop.funcs.eq(prop.ptr, prop.ptr));
}

TEST_F(TestFixture_Speed, EqualComponentSame) {
	auto a = AddEntity();
	ResolvedProperty propA = tri::resolveProperty(ecs, a, COMP_NAME_SEP);

	auto b = AddEntity();
	ResolvedProperty propB = tri::resolveProperty(ecs, b, COMP_NAME_SEP);

	// Must be same since come from the same metadata of component
	ASSERT_EQ(propA.funcs, propB.funcs);

	ASSERT_TRUE(propA.funcs.eq(propA.ptr, propB.ptr));
}

TEST_F(TestFixture_Speed, EqualWorlds) {
	
	
	//auto setupWorld = [](flecs::world& ecs){
	//	TestRunner::initialize<movement::module>(ecs);
	//	TestRunner::registerTypes<movement::Speed>(ecs);

	//	auto a = ecs.entity("Aboba");
	//	a.add<movement::Speed>();
	//	ResolvedProperty propA = tri::resolveProperty(ecs, a, COMP_NAME_SEP);
	//};

	std::string name = "Aboba";

	flecs::world initial, expected;
	InitWorld(initial);
	AddEntity(initial, name);

	InitWorld(expected);
	AddEntity(expected, name);

	tri::UnitTest::Operators operators = {
		{
			"Aboba/" + COMP_NAME,
			Operator::Type::EQ
		}
	};

	ASSERT_TRUE(
		tri::compareWorlds(initial, expected, operators)
	);
}

#if 0
TEST_F(TestFixture_Speed, EqualWorlds1) {
	using Operator = tri::UnitTest::Operator;

	auto setupWorld = [](flecs::world& ecs) {
		TestRunner::initialize<movement::module>(ecs);
		TestRunner::registerTypes<movement::Speed>(ecs);

		auto a = ecs.entity("Aboba");
		a.add<movement::Speed>();
		ResolvedProperty propA = tri::resolveProperty(ecs, a, COMP_NAME_SEP);
		};

	flecs::world initial, expected;
	setupWorld(initial);
	setupWorld(expected);

	tri::UnitTest::Operators operators = {
		{
			"Aboba/movement.module.Speed",
			Operator::Type::EQ
		}
	};

	ASSERT_TRUE(
		tri::compareWorlds(initial, expected, operators)
	);
}
#endif


