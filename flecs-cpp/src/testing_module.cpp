#pragma once

#include "testing_module.h"

#include <string>
#include <iostream>

#include "utils/reflection.h"

namespace testing {

namespace {

    // ============================================================================================
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

    // ============================================================================================
    void registerReflection(flecs::world& world) {
        world.component<std::string>()
            .opaque(flecs::String) // Opaque type that maps to string
            .serialize([](const flecs::serializer* s, const std::string* data) {
            const char* str = data->c_str();
            return s->value(flecs::String, &str); // Forward to serializer
        })
            .assign_string([](std::string* data, const char* value) {
            *data = value; // Assign new value to std::string
        });

        world.component<SystemInvocation>()
            .member<std::string>("name")
            .member<int>("timesToRun");

        // Register reflection for std::vector<int>
        world.component<std::vector<SystemInvocation>>()
            .opaque(reflection::std_vector_support<SystemInvocation>);


        // TODO: maybe use some template magic to add reflection?
        world.component<UnitTest>()
            .member<std::vector<SystemInvocation>>("systems")
            .member<std::string>("scriptActual")
            .member<std::string>("scriptExpected")
            .member<bool>("passed");

    }

    // ============================================================================================
    void runSystem(flecs::world& world, const SystemInvocation& sys) {
        // Lookup the system by name
        flecs::entity systemEntity = world.lookup(sys.name.c_str());

        if(!systemEntity) {
            std::cerr << "ERROR: System '" << sys.name << "' not found!\n";
            return;
        }

        // Check if it's a system
        if(!systemEntity.has(flecs::System) || !world.system(systemEntity)) {
            std::cerr << "ERROR: Entity '" << sys.name << "' is not a system!\n";
            return;
        }

        flecs::system system = world.system(systemEntity);

        // Run system
        for(int i = 0; i < sys.timesToRun; ++i) {
            system.run();
            std::cout << "[" << i << "] Running system '" << sys.name << "'\n";
        }
    }
}

// ================================================================================================
struct moduleImpl {
    moduleImpl(flecs::world& world);

    static std::function<void(flecs::world&)> modulesProvider;
};

std::function<void(flecs::world&)> moduleImpl::modulesProvider;


// ================================================================================================
moduleImpl::moduleImpl(flecs::world& world) {
    registerReflection(world);

    world.system<UnitTest>("TestRunner")
        .kind(flecs::OnUpdate)
        .without<UnitTest::Executed>()
        .each([this](flecs::entity e, UnitTest& test) {
            std::cout << "Running test: " << e.name() << "\n";

            // Create ACTUAL world
            flecs::world worldActual;
            // Import testable systems & components
            modulesProvider(worldActual);
            worldActual.script_run("Script (Actual)", test.scriptActual.c_str());

            for(auto& sys : test.systems) {
                runSystem(worldActual, sys);
            }

            // Create EXPECTED world
            flecs::world worldExpected;
            modulesProvider(worldExpected);
            worldExpected.script_run("Script (Expected)", test.scriptExpected.c_str());

            test.passed = compareWorlds(worldActual, worldExpected);

            e.add<UnitTest::Executed>();
        });
}

// ================================================================================================
void testing::initializeTests(
    flecs::world& world, 
    std::function<void(flecs::world&)> modulesProvider
) {
    moduleImpl::modulesProvider = modulesProvider;
    world.import<moduleImpl>();

    /* TODO:
    * Maybe automatically assign kind 0 to all systems here? 
    */
}

}
