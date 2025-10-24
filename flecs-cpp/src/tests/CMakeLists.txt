set(EXE_NAME "Tests")
set(SRC_DIR "./")
#set(SHARED_DIR "../shared/")

#include(../Functions.cmake)
discover_sources(${SRC_DIR} SRC_FILES)

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
    ${TEST_RUNNER_INCLUDE_DIR}
    ${MODULES_INCLUDE_DIR}
    #${SHARED_DIR}
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
