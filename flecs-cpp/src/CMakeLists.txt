set(TEST_RUNNER_LIB_NAME TestRunner)
set(TEST_RUNNER_DIR "test_runner")
set(TEST_RUNNER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_RUNNER_DIR}/include/")

set(SHARED_DIR_ROOT "shared")

set(SHARED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${SHARED_DIR_ROOT}")

set(MODULES_LIB_NAME Modules)
set(MODULES_DIR "MODULES")
set(MODULES_INCLUDE_DIR "${SHARED_DIR}/${MODULES_DIR}/include")


add_subdirectory(${SHARED_DIR_ROOT}) 
add_subdirectory(test_runner) 
add_subdirectory(core) 
add_subdirectory(tests)
