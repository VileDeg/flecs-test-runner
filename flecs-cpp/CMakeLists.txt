cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME "FlecsSample")
set(SRC_DIR "src")
set(BIN_DIR "bin")


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

FetchContent_MakeAvailable(flecs)


# Recursively find all .cc and .h files in src directory
file(GLOB_RECURSE SOURCES
  "${SRC_DIR}/*.cpp"
  "${SRC_DIR}/*.h"
)

# Print found files (optional, for debugging)
message(STATUS "Found source files:")
foreach(source ${SOURCES})
  get_filename_component(filename ${source} NAME)
  message(STATUS "  ${filename}")
endforeach()


add_executable(${PROJECT_NAME} ${SOURCES})



# Enable C++20
# set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
# set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)


# Add src directory to include path so headers can be found
target_include_directories(${PROJECT_NAME} PRIVATE ${SRC_DIR})

#add_subdirectory("C:/dev/flecs/flecs" "C:/dev/flecs/flecs/bin")
#target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} flecs::flecs_static)

