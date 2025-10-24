#include <modules/time.h>
#include <thread>
#include <chrono>

namespace modules {
    time::time(flecs::world& world) {
        world.component<Timer>()
            .member<int>("time");

        world.component<TimeStep>()
            .member<int>("step");

        world.system<Timer, const TimeStep>("wait")
            .kind(0) // Prevent being registered to a pipeline
            .each([](Timer& t, const TimeStep& s) {
                // This intentionally blocks the system thread for the duration.
                std::this_thread::sleep_for(std::chrono::milliseconds(s.step));
                t.time += s.step;
            });
    }
};