#pragma once

#include <flecs.h>

#include <string>
#include <functional>


namespace testing {
    /* TODO:
    * Maybe suppor system triggered by an event?
    * */

    struct SystemInvocation {
        std::string name;
        int timesToRun;
    };

    struct UnitTest {
        struct Executed {};

        std::vector<SystemInvocation> systems;

        std::string scriptActual;
        std::string scriptExpected;

        bool passed = false;
    };

    void initializeTests(flecs::world& world, std::function<void(flecs::world&)> modulesProvider);

    template <typename... Args>
    inline static void addTestEntity(flecs::world& world, const char* name, Args&&... args) {
        world.entity(name)
            .set<UnitTest>({ std::forward<Args>(args)... });
    }
}

