set(EXE_NAME "TestRunnerCore")
set(SRC_DIR "./")


#include(../Functions.cmake)
discover_sources(${SRC_DIR} SRC_FILES)

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
    ${SHARED_DIR}
    ${TEST_RUNNER_INCLUDE_DIR}
    ${MODULES_INCLUDE_DIR}
)

target_link_libraries(
  ${EXE_NAME}
  PRIVATE
    flecs::flecs_static
    ${TEST_RUNNER_LIB_NAME}
    ${MODULES_LIB_NAME}
)
