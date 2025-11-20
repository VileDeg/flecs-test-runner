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

#[[
TODO: better impl


function(discover_sources_from_git SEARCH_DIR OUT_VAR)
    # Define the core git command.
    # --cached: includes staged files
    # --others: includes untracked files (optional, can be removed to be stricter)
    # --exclude-standard: ignores files in .gitignore
    # --full-name: outputs paths relative to the repository root
    # -- ':path/to/search/': limits the search to a specific directory

    execute_process(
        COMMAND git ls-files --full-name -- "${SEARCH_DIR}"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_FILES
        RESULT_VARIABLE GIT_RESULT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(GIT_RESULT EQUAL 0)
        # Convert the multiline output into a CMake list
        string(REPLACE "\n" ";" FOUND_FILES "${GIT_FILES}")
    
        # Filter the list to only include desired extensions (.cpp, .cc, .h)
        set(FILTERED_FILES "")
        foreach(file ${FOUND_FILES})
            if(file MATCHES "\\.(cpp|cc|h)$")
                list(APPEND FILTERED_FILES "${file}")
            endif()
        endforeach()
        
        # Debugging Output (optional)
        message(STATUS "Discovering sources in: ${SEARCH_DIR} using git ls-files")
        foreach(file ${FILTERED_FILES})
            message(STATUS "  - ${file}")
        endforeach()
        
        # Pass the result back to the caller using PARENT_SCOPE
        set(${OUT_VAR} "${FILTERED_FILES}" PARENT_SCOPE)

    else()
        message(FATAL_ERROR "Failed to run 'git ls-files' in ${CMAKE_SOURCE_DIR}. Is it a Git repository?")
    endif()

    # --- Crucial step for reliability! ---
    # This tells CMake to re-run the configuration step whenever a tracked file
    # is added or removed, which keeps the list of sources up-to-date.
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" APPEND PROPERTY 
        CMAKE_CONFIGURE_DEPENDS 
        "${CMAKE_SOURCE_DIR}/.git/index"
    )

endfunction()
]]
