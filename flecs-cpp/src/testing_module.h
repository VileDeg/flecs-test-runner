#pragma once

#include <flecs.h>

#include <string>
#include <functional>


namespace testing {
    /* TODO:
    * provide some way to modify system execution conditions.
    * E.g. allow to add dependency on some phase, 
    * or be triggered by an event etc.
    * */

    struct SystemInvocation {
        std::string name;
        int timesToRun;
        // TODO: extend for other run parameters, maybe add phase etc.
        // TODO: add support for custom pipeline?
    };

    struct UnitTest {
        std::vector<SystemInvocation> systems;

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

