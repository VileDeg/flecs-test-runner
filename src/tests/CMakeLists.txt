set(EXE_NAME "Tests")
set(SRC_DIR "./")

# Download Google Test from GitHub
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        52eb8108c5bdec04579160ae17225d66034bd723 # release 1.17.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# Make the targets available (downloads the source and runs its CMake)
FetchContent_MakeAvailable(googletest)

# Explicit source file list (no GLOB_RECURSE)
set(SRC_FILES
    movement_module_test.cpp
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
    ../../../${MODULES_INCLUDE_DIR}
)


target_link_libraries(
  ${EXE_NAME}
  PRIVATE
    GTest::gtest_main   
    flecs::flecs_static # TODO: remove?
    ${TEST_RUNNER_LIB_NAME}
    ${MODULES_LIB_NAME}
)


# Tell CTest to discover all tests inside the executable.
# This scans the compiled executable for all TEST() and TEST_F() macros.
include(GoogleTest)
gtest_discover_tests(${EXE_NAME})
