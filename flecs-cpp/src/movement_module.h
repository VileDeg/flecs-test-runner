#include <flecs.h>

namespace testable {
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
