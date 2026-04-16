# Flecs Test Runner

This the core implementation of the **Flecs Test Runner (FTR)**.

## Supported languages 

- C++ (at least C++17).

## Usage

To include **FTR** into your project, see this example of `CmakeLists.txt`:
```cmake
include(FetchContent)

FetchContent_Declare(
    test_runner
    GIT_REPOSITORY https://github.com/VileDeg/flecs-test-runner
)

FetchContent_MakeAvailable(
    test_runner
)

set(TEST_RUNNER_LIB_NAME "TestRunner")

set(EXE_NAME ${PROJECT_NAME})
set(SRC_DIR "./")

set(SRC_FILES
    # Your source files...
    main.cpp
)

# Add the test executable
add_executable(
  ${EXE_NAME} 
  ${SRC_FILES}
)

# Add src directory to include path so headers can be found
target_include_directories(
  ${EXE_NAME} 
  PRIVATE 
    ${SRC_DIR}
)

target_link_libraries(
  ${EXE_NAME}
  PRIVATE
    ${TEST_RUNNER_LIB_NAME}
)
```

Also see the example project: [FTR Sample Project](https://github.com/VileDeg/flecs-test-runner-sample-project).

### Run
Run your application that was built with `FTR` and go to the [FTR Client Page](https://viledeg.github.io/flecs-test-runner-client/).

For more details about the client see [FTR Client](https://github.com/VileDeg/flecs-test-runner-client).

### !! Important Notes

To enable comparison of the individual components you have to do one of the following:

- For any components you want to compare directly register reflection hooks with `on_compare` and/or `on_equals` to make Flecs automatically derive the comparison operator hooks to compare individual components. These hooks will be used to determine if the component supports comparison.

Or, if you don't want to modify your modules' source code:

- Use `TestRunner::registerOperators` function to list all the components manually. Operators metadata will be registered for each individual components this way.

--------------
Defining systems:
- When defining a system that creates/removes entities/components make sure to mark the system as `immediate` (see Flecs docs for details). Otherwise the effect of structural changes will not be immediately visible preventing them from being possible to test.

## Build (C++)

`CMake` must be installed in order to build.

When cmake is called directly, it will **fetch the dependencies** from remote Git repositories, using `FetchContent`. 

```ps
cmake -S . -B build/
```

Output is a **static library**.

## Known limitations

- Entity names in the test configurations must be unique.   
  - The names are used to define comparison operator paths, thus need to be unique.

## Future improvements

- Fix all TODO comments.
- Make a non-gui app that would just accept folder/file with tests and output test results.
- How to prevent pointers/referencies from being used in components?
- Allow matching entity name by regex (not the exact name)

