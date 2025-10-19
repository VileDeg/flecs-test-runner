cmake_minimum_required(VERSION 3.14)

set(PROJECT_NAME "FlecsSample")
#set(SRC_DIR "src")
#set(BIN_DIR "bin")


# Set the project name and version
project(${PROJECT_NAME} VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



#find_package(fmt CONFIG REQUIRED)
#find_package(flecs CONFIG REQUIRED)


include(FetchContent)

# Fetch the Flecs repository
FetchContent_Declare(
    flecs
    GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
    #GIT_TAG master # Use the latest version; replace with a specific tag if needed
)



# Declare the dependency: Download Google Test from GitHub
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        52eb8108c5bdec04579160ae17225d66034bd723 # release 1.17.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# Make the targets available (downloads the source and runs its CMake)
FetchContent_MakeAvailable(flecs googletest)
# Now targets like GTest::gtest_main are available

# --- Add Application and Tests ---

# 1. Add your application's source code/library
add_subdirectory(src) 

# 2. Include the tests directory
add_subdirectory(tests)




#add_executable(${PROJECT_NAME} ${SOURCES})



# Enable C++20
# set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
# set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)




