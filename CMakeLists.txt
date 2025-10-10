cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME FlecsSample)
project(${PROJECT_NAME})

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



add_executable(${PROJECT_NAME} main.cpp)


#add_subdirectory("C:/dev/flecs/flecs" "C:/dev/flecs/flecs/bin")
#target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} flecs::flecs_static)



#target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt flecs::flecs)
