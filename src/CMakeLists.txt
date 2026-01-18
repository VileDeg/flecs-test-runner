set(TEST_RUNNER_LIB_NAME TestRunner)

set(MODULES_LIB_NAME Modules)
set(MODULES_DIR "modules")
set(MODULES_INCLUDE_DIR "${MODULES_DIR}/include")

include(FetchContent)

# Fetch the Flecs repository
FetchContent_Declare(
    flecs
    GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
    #GIT_TAG master # Use the latest version; replace with a specific tag if needed
)
FetchContent_MakeAvailable(flecs)

add_subdirectory(core) 
add_subdirectory(tests)
