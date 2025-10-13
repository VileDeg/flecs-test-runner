#pragma once

#include <flecs.h>

#include <string>
#include <functional>


namespace testing {
    struct UnitTest {
        std::string systemName;
        int timesToRun;

        std::string scriptActual;
        std::string scriptExpected;

        bool executed = false;
        bool passed = false;
    };

    void initializeTests(flecs::world& world, std::function<void(flecs::world&)> modulesProvider);

    template <typename... Args>
    inline static void addTestEntity(flecs::world& world, const char* name, Args&&... args) {
        world.entity(name)
            .set<UnitTest>({ std::forward<Args>(args)... });
    }
}

