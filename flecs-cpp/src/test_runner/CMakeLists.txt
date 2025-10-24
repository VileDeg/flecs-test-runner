set(LIB_NAME ${TEST_RUNNER_LIB_NAME})
set(SRC_DIR "./")
#set(SHARED_DIR "../shared/")

#include(../Functions.cmake) # TODO: is enough to include at the top level?
discover_sources(${SRC_DIR} SRC_FILES)

# Create a static library
add_library(
  ${LIB_NAME} STATIC 
  ${SRC_FILES}
)

# Add src directory to include path so headers can be found
target_include_directories(
  ${LIB_NAME} 
  PRIVATE 
    ${SRC_DIR}
    ${SRC_DIR}/include
    ${SHARED_DIR}
)

target_link_libraries(
  ${LIB_NAME}
  PRIVATE
    flecs::flecs_static
)
