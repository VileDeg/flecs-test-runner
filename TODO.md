1. Run each system invocation (or system?) in a separate thread to prevent blocking on flecs app and let it answer HTTP requests.
  Need to be able to check polling tests.
2. Allow testing for <, >, != in tests. 
  Maybe specify in test config which components to compare with which equality operators.
3. Provide user a way to create tests graphically.
  Maybe query for all available components and systems and let user assemble the test by selecting components, systems etc.
  Based on user selection generate actual and expected script and comparison operators.
  Fill in UnitTest object and proceed as usual.
4. Export tests
5. Make a non-gui app that would just accept folder/file with tests and output test results.
6. Wait for establishing connection?
7. Add a wait to clear test results.
8. Use observer instead of system for test runner?


9. Allow defining tests by running the "actual" script and using the result as "expected".
10. Allow the user to define the systems to be ran and components configuration
11. Add "pending" tests display by saving the list of "to-be-run" tests on client side


Multi-threading proposal

Main thread
  |_ A. Test runner systen
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

