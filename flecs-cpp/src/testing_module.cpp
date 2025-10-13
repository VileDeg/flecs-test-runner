#pragma once

#include "testing_module.h"

#include <string>
#include <iostream>

namespace testing {

namespace {
    bool compareWorlds(flecs::world& world1, flecs::world& world2) {
        std::cout << "\nWorld Comparison (using serialization):\n";
        std::cout << "========================================\n";

        // Serialize both worlds to JSON
        flecs::string json1 = world1.to_json();
        flecs::string json2 = world2.to_json();

        std::cout << "\nWorld 1 JSON:\n";
        std::cout << json1 << "\n";

        std::cout << "\nWorld 2 JSON:\n";
        std::cout << json2 << "\n";

        // Compare the serialized representations
        bool matches = (json1 == json2);

        std::cout << "\n";
        if(matches) {
            std::cout << "WORLDS MATCH!\n";
            std::cout << "The serialized JSON representations are identical.\n";
        } else {
            std::cout << "WORLDS DO NOT MATCH!\n";
            std::cout << "The serialized JSON representations differ.\n";

            // Show size difference as a hint
            std::cout << "\nJSON sizes: World1=" << json1.size()
                << " bytes, World2=" << json2.size() << " bytes\n";
        }

        return matches;
    }

    std::function<void(flecs::world&)> modulesProvider;
}

// TODO: split into components and systems modules?
struct moduleImpl {
    moduleImpl(flecs::world& world);
};



moduleImpl::moduleImpl(flecs::world& world) {
    // Register reflection for std::string
    world.component<std::string>()
        .opaque(flecs::String) // Opaque type that maps to string
        .serialize([](const flecs::serializer* s, const std::string* data) {
        const char* str = data->c_str();
        return s->value(flecs::String, &str); // Forward to serializer
            })
        .assign_string([](std::string* data, const char* value) {
        *data = value; // Assign new value to std::string
            });

    // TODO: maybe use some template magic to add reflection?
    world.component<UnitTest>()
        .member<std::string>("systemName")
        .member<int>("timesToRun")
        .member<std::string>("scriptActual")
        .member<std::string>("scriptExpected")
        .member<bool>("executed")
        .member<bool>("passed");


    world.system<UnitTest>("TestRunner")
        .kind(flecs::OnUpdate)
        .each([](flecs::entity e, UnitTest& test) {
            // TODO: maybe make as query? Iterate only over ones that have false
            if(test.executed) {
                return;
            }

            std::cout << "Running test: " << e.name()
                << " (system: " << test.systemName << ")\n";


            // Create ACTUAL world
            flecs::world worldActual;
            // Import testable systems & components
            // TODO: find a way to include all testable modules
            //worldActual.import<testable::movement>();
            modulesProvider(worldActual);
            worldActual.script_run("Script (Actual)", test.scriptActual.c_str());

            // Lookup the system by name
            flecs::entity systemEntity = worldActual.lookup(test.systemName.c_str());

            if(!systemEntity) {
                std::cerr << "ERROR: System '" << test.systemName
                    << "' not found!\n";
                return;
            }

            // Check if it's a system
            if(!systemEntity.has(flecs::System) || !worldActual.system(systemEntity)) {
                std::cerr << "ERROR: Entity '" << test.systemName
                    << "' is not a system!\n";
                return;
            }

            flecs::system system = worldActual.system(systemEntity);

            // Run system
            for(int i = 0; i < test.timesToRun; ++i) {
                system.run();
                std::cout << "[" << i << "] System '" << test.systemName << "' executed\n";
            }

            // Create EXPECTED world
            flecs::world worldExpected;
            // Import testable systems & components
            modulesProvider(worldExpected);
            worldExpected.script_run("Script (Expected)", test.scriptExpected.c_str());

            test.passed = compareWorlds(worldActual, worldExpected);
            test.executed = true;
        });

    
}

void testing::initializeTests(flecs::world& world, std::function<void(flecs::world&)> modulesProvider) {
    modulesProvider = modulesProvider;
    world.import<moduleImpl>();
}

}
