1. Allow testing for <, >, != in tests
   1. Specify in UI the comparison operator (in expected config)
   2. Use flecs reflection on cpp side to deserialize the component from json, iterate over its attributes and compare the one required
   3. If flecs reflection is not accessible just get the attribute from json and knowing its type (from client) cast to correct type and compare
   4. Maybe specify in test config which components to compare with which equality operators. Then on cpp side worlds will be serialized, then parsed from json and compared as strings. 
2. Make a non-gui app that would just accept folder/file with tests and output test results.
3. What to do with compound types in components? I.e. structs
   1. Maybe is only possible to support compound types that are registered as components themselves.
4. What to do with pointers in components? 
   1. If unsupported, how to prevent them from being used?

   
5. Wait for establishing connection?
6. Use observer instead of system for test runner?
7. Add "pending" tests display by saving the list of "to-be-run" tests on client side
8. Allow to modify the uploaded test.
    1.  Maybe make the view switch to test builder after upload instead of running right away? 
    2.  Or on upload page add buttons "Modify", "Run".

9. Run all systems in a pipeline to correctly handle the staging, merging of the state?
10. What if a system modifies world state? Would it work?
11. Pass hardcoded delta time when calling the system
12. What if a system uses context (user data for system)?


TODO:
  workspace - load whole folder and have a list of tests in categories
    allow running / modifying selected tests
  support compound types
    components are allowed to only have attributes of primitive or another component type
    pointers are unsupported (reflection for them does not exist in flecs)
  make UI for compact and user friendly
  make script preview?
  
Issues:
  fix failing incomplete test
  hide incomplete tests from results preview
  when all modules deselected they flip to all selected
  
Client issues:
  fix the typescript errors
  think about persistent storage best practices
  split test builder into more modules
  fix toast messages spam


Multi-threading proposal

Main thread
  |_ A. Test runner system
  |_ B. Test status checker system
  
Thread pool (N threads)
  |_ Running test 0.
  |_ Running test 1.
  ...
  |_ Running test N-1.

<!-- Monitor thread (checks if N threads have finished) -->

Make each test run on a separate thread. Set executed tag right away on main thread to not run test again. 

Use a thread pool of N threads for tests. Each thread when finished will write the status (passed/failed) to shared memory.

<!-- Meanwhile have a thread that periodically checks all the threads if they had finished. -->

1. Approach
Have another system on main thread periodically checking the shared memory to update the status of Executed component to let the client retrieve the passed/failed tests.

Pros:
This way we avoid sharing the world between threads

Cons:
If the system B does lazy wait it will block system A cause they are on same thread. So system B essentially does busy wait which is not ideal.

2. Approach

