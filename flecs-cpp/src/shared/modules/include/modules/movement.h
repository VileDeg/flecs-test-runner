#include <flecs.h>

namespace modules {
    struct Position {
        float x;
        float y;
    };

    struct Velocity {
        float x;
        float y;
    };
    
    struct movement {
        movement(flecs::world& world);
    };
};
