set(LIB_NAME ${TEST_RUNNER_LIB_NAME})
set(SRC_DIR "./")

set(SRC_FILES
    test_runner.cpp
    test_runner_impl.h
    test_runner_impl.cpp
    reflection.h
    include/test_runner/test_runner.h
    include/test_runner/common.h
)

# Create a static library
add_library(
  ${LIB_NAME} STATIC 
  ${SRC_FILES}
)

target_include_directories(
  ${LIB_NAME} 
  PRIVATE 
    ${SRC_DIR}
  PUBLIC
    ${SRC_DIR}/include
)

target_link_libraries(
  ${LIB_NAME}
  PUBLIC
    flecs::flecs_static
)
