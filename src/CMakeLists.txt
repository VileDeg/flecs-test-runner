set(TEST_RUNNER_LIB_NAME TestRunner)

set(MODULES_LIB_NAME Modules)

include(FetchContent)

# Fetch the Flecs repository
FetchContent_Declare(
    flecs
    GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
    GIT_TAG d7d0c4f7afb4518a6bae749efdc52c7cb5cffee6 # release v4.1.5
)
FetchContent_MakeAvailable(flecs)

add_subdirectory(core) 
add_subdirectory(test)
