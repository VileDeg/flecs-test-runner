#include <iostream>
#include <unordered_set>

#include <flecs.h>

#include <test_runner/test_runner.h>
#include <modules/movement.h>
#include <modules/time.h>

using mass = flecs::units::mass;

struct Mass {
  double value;
};

struct Count {
  int count;
};


int remote(int argc, char* argv[]) {
  // Passing in the command line arguments will allow the explorer to display
  // the application name.
  flecs::world ecs(argc, argv);

  TestRunner::setLogLevel(TestRunner::LogLevel::INFO);
  TestRunner::initialize(ecs, [](flecs::world& world) {
    world.import<modules::movement>();
    world.import<modules::time>();
  });


  ecs.import<flecs::units>();
  ecs.import<flecs::stats>(); // Collect statistics periodically


  // Mass component
  ecs.component<Mass>()
    .member<double, mass::KiloGrams>("value");



  // Simple hierarchy
  flecs::entity Sun = ecs.entity("Sun")
    .set<Mass>({ 1.988500e31 });

  flecs::entity Earth = ecs.scope(Sun).entity("Earth")
    .set<Mass>({ 5.9722e24 });

  ecs.scope(Earth).entity("Moon")
    .set<Mass>({ 7.34767309e22 });


#if 0
   
  std::cout << "All entities: \n";


  // Use the C API to get the highest issued id, then test each id
  ecs_entity_t max_id = ecs_get_max_id(ecs.c_ptr());

  for(ecs_entity_t e = 1; e <= max_id; ++e) {
  if(!ecs.is_alive(e)) continue;   // skip dead / unused ids

  // Get the name (returns nullptr if not named)
  const char* name = ecs_get_name(ecs.c_ptr(), e);
  if(name && name[0] != '\0') {
    std::cout << "Entity " << e << " : " << name << '\n';
  } else {
    std::cout << "Entity " << e << " : (unnamed)\n";
  }
  }
#endif


  std::cout << "Running HTTP server on port 27750 ...\n";
  // Run application with REST interface. When the application is running,
  // navigate to https://flecs.dev/explorer to inspect it!
  //
  // See docs/FlecsRemoteApi.md#explorer for more information.
  return ecs.app()
    .enable_rest()
    .enable_stats()
    .run();
}


int main(int argc, char *argv[]) {
  int ret = remote(argc, argv);

  return ret;
}
