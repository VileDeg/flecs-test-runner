#include "movement_module.h"

namespace testable {
    movement::movement(flecs::world& world) {
        world.component<Position>()
            .member<float>("x")
            .member<float>("y");

        world.component<Velocity>()
            .member<float>("x")
            .member<float>("y");

        world.system<Position, const Velocity>("move")
            /* TODO: is there a better way ?
            * Maybe automatically assign kind 0 after import by testing logic? 
            */
            .kind(0) // Prevent being registered to a pipeline
            .each([](Position& p, const Velocity& v) {
                p.x += v.x;
                p.y += v.y;
            });
    }
};