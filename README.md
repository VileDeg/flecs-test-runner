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
