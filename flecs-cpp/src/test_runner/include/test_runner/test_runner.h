#pragma once

#include <flecs.h>

#include <string>
#include <functional>
#include <optional>


namespace test_runner {
    /* TODO:
    * Maybe suppor system triggered by an event?
    * */

    struct SystemInvocation {
        std::string name;
        int timesToRun;
    };

    struct UnitTest {
        // Tags
        struct Executed {
            std::string statusMessage;
        };
        struct Passed {

        };

        /*struct Result {
            std::string name;
            bool passed = false;
        };*/

        std::string name;

        std::vector<SystemInvocation> systems;

        std::string scriptActual;
        std::string scriptExpected;

        std::optional<std::string> validate() const {
            std::optional<std::string> statusMessage = std::nullopt;
            if (name.empty()) {
                statusMessage = "Name is empty";
            } else if(systems.empty()) {
                statusMessage = "No systems to run";
            } else if(scriptActual.empty()) {
                statusMessage = "Actual script is empty";
            } else if(scriptExpected.empty()) {
                statusMessage = "Expected script is empty";
            } 
            return statusMessage;
        }

        //bool passed = false;
    };

    enum class LogLevel {
        FATAL = 0,
        ERROR,
        WARN,
        INFO,
        TRACE,
    };

    

    void initializeTests(flecs::world& world, std::function<void(flecs::world&)> modulesProvider);

    void setLogLevel(LogLevel logLevel);

    // TODO: move to UnitTest project and instead use http port to get results and add entities
    /*std::vector<UnitTest::Result> getAllTestResults();*/

    template <typename... Args>
    inline static void addTestEntity(flecs::world& world, const char* name, Args&&... args) {
        world.entity(name)
            .set<UnitTest>({ std::forward<Args>(args)... });
    }
}

