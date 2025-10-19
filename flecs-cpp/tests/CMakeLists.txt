set(EXE_NAME "Tests")
set(SRC_DIR "./")
#set(BIN_DIR "bin")

include(../Functions.cmake)
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
    "../src"
)


target_link_libraries(
  ${EXE_NAME}
  PRIVATE
    GTest::gtest_main   
    flecs::flecs_static
    
)


# Tell CTest to discover all tests inside the executable.
# This scans the compiled executable for all TEST() and TEST_F() macros.
include(GoogleTest)
gtest_discover_tests(${EXE_NAME})
