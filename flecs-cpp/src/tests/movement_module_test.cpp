#include <gtest/gtest.h>
#include <test_runner/test_runner.h>

#include <modules/movement.h>

int add(int a, int b) {
    return a + b;
}

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

  // TODO: check if test executed and passed
}

TEST(Calculator, AddNegative) {
    // Use an EXPECT_ macro to check the result
    EXPECT_EQ(0, add(-5, 5));
}

// NOTE: You do NOT need a main() function if you link to GTest::gtest_main