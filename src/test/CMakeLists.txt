set(EXE_NAME "Tests")
set(SRC_DIR "./")

# Download Google Test from GitHub
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        52eb8108c5bdec04579160ae17225d66034bd723 # release 1.17.0
)

FetchContent_Declare(
    modules
    GIT_REPOSITORY https://github.com/VileDeg/flecs-test-runner-modules
    #GIT_TAG master # Use the latest version; replace with a specific tag if needed
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# Make the targets available (downloads the source and runs its CMake)
FetchContent_MakeAvailable(
    googletest
    modules
)

set(SRC_FILES
    common.h
    compare_property_test.cpp
    run_unit_test.cpp
    component_metadata_test.cpp
)

enable_testing()

# Add the test executable
add_executable(
    ${EXE_NAME} 
    ${SRC_FILES}
)


# Add src directory to include path so headers can be found
target_include_directories(
  ${EXE_NAME} 
  PRIVATE 
    ${SRC_DIR}
    ../core
)


target_link_libraries(
  ${EXE_NAME}
  PRIVATE
    GTest::gtest_main
    #flecs::flecs_static
    ${TEST_RUNNER_LIB_NAME}
    ${MODULES_LIB_NAME}
)


# Tell CTest to discover all tests inside the executable.
# This scans the compiled executable for all TEST() and TEST_F() macros.
include(GoogleTest)
gtest_discover_tests(${EXE_NAME})
