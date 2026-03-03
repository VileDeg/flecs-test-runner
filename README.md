# Flecs Test Runner

This the core implementation of the **Flecs Test Runner (FTR)**.

TODO

## Supported languages 

- C++ (at least C++17).
- TODO

## Build (C++)

`CMake` must be installed in order to build.

Otherwise when cmake is called directly, it will **fetch the dependencies** from remote Git repositories, using `FetchContent`. 

```ps
cmake -S . -B build/
```

Output is a **static library**.

TODO: make it a header-only library?


## Usage

- All user defined components that have operators defined need to have reflection defined using `on_compare` to make Flecs automatically derive the operators and setup hooks that will be later uses to compare individual components. TODO: also allow user to register types explicitly to avoid modifying the existing modules reflection code.

## TODO

FLECS_CREATE_MEMBER_ENTITIES
 
