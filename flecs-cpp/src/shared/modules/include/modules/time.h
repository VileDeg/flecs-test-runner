#include <flecs.h>

namespace modules {
  struct Timer {
    int time;
  };

  struct TimeStep{
    int step;
  };
  
  struct time {
    time(flecs::world& world);
  };
}