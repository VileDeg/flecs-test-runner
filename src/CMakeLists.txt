set(TEST_RUNNER_LIB_NAME TestRunner)
#set(TEST_RUNNER_DIR "test_runner")
#set(TEST_RUNNER_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_RUNNER_DIR}/include/")

#set(SHARED_DIR_ROOT "shared")

#set(SHARED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${SHARED_DIR_ROOT}")

set(MODULES_LIB_NAME Modules)
set(MODULES_DIR "modules")
set(MODULES_INCLUDE_DIR "${MODULES_DIR}/include")


function(discover_sources SRC_DIR SRC_FILES)
    # Define the file patterns
    file(GLOB_RECURSE FOUND_FILES
        "${SRC_DIR}/*.cpp"
        "${SRC_DIR}/*.h"
        "${SRC_DIR}/*.cc" # Added .cc for common C++ extensions
    )

    # Debugging Output (optional, remove once stable)
    message(STATUS "Discovering sources in: ${SRC_DIR}")
    if(FOUND_FILES)
        message(STATUS "  Found ${CMAKE_CURRENT_FUNCTION}:")
        foreach(file ${FOUND_FILES})
            get_filename_component(filename ${file} NAME)
            message(STATUS "  - ${filename}")
        endforeach()
    else()
        message(WARNING "  No source files found in: ${SRC_DIR}")
    endif()

    # Pass the result back to the caller using the PARENT_SCOPE
    set(${SRC_FILES} "${FOUND_FILES}" PARENT_SCOPE)
    #return(${SRC_FILES} "${FOUND_FILES}")

endfunction()


include(FetchContent)
#include(../Functions.cmake)

# Fetch the Flecs repository
FetchContent_Declare(
    flecs
    GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
    #GIT_TAG master # Use the latest version; replace with a specific tag if needed
)
FetchContent_MakeAvailable(flecs)

add_subdirectory(core) 
add_subdirectory(tests)
