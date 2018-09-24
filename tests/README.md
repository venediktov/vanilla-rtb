# Vanilla-RTB Automatic Tests

Tests are written with [Boost.Test] framework.
The framework is sufficiently powerful (hard/soft assertions, tests, suites, fixtures, exclusions, labels, etc.) and [Boost] is already a dependency of Vanilla-RTB.

## Building & Running

### ...via ctest

Useful within the integration build.

```bash
# also `make -j$(nproc) test` if conf.-ed with `cmake -G "Unix Makefiles"`
$ cmake --build . -j $(nproc) --target test
```

Output:

```
Running tests...
Test project /home/bobah/code/vanilla-rtb/Release
    Start 1: ctest_vanilla_tests
1/1 Test #1: ctest_vanilla_tests ..............   Passed    0.16 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.16 sec
```

### ...directly

Useful when using specific features of Boost.Test ([verbose output], [tests filtering], etc.).

```bash
# also `make BOOST_TEST_LOG_LEVEL=all BOOST_TEST_RUN_FILTERS=vanilla_all/* unit-test` if conf.-ed with `cmake -G "Unix Makefiles"`
$ cmake --build . -j $(nproc) --target unit-test -- BOOST_TEST_LOG_LEVEL=all BOOST_TEST_RUN_FILTERS=vanilla_all/*
```

Output (fails because disabled tests are activated with the wildcard on the command line):

```
[ 33%] Built target vanilla_all
[100%] Built target vanilla_tests
Running 2 test cases...
Entering test module "vanilla"
/home/bobah/code/vanilla-rtb/tests/vanilla_all.cpp(8): Entering test suite "vanilla_all"                                                                    
/home/bobah/code/vanilla-rtb/tests/vanilla_all.cpp(10): Entering test case "green_test"                                                                     
I am a good test, I am passing                                                                                                                              
/home/bobah/code/vanilla-rtb/tests/vanilla_all.cpp(12): info: check 'exception "std::runtime_error" raised as expected' has passed
/home/bobah/code/vanilla-rtb/tests/vanilla_all.cpp(13): info: check 42 == 42 has passed
/home/bobah/code/vanilla-rtb/tests/vanilla_all.cpp(10): Leaving test case "green_test"; testing time: 221us
/home/bobah/code/vanilla-rtb/tests/vanilla_all.cpp(18): Entering test case "red_test"                                                                       
I am a bad test, I am failing                                                                                                                               
/home/bobah/code/vanilla-rtb/tests/vanilla_all.cpp(20): error: in "vanilla_all/red_test": check 42 == 43 has failed [42 != 43]
/home/bobah/code/vanilla-rtb/tests/vanilla_all.cpp(21): fatal error: in "vanilla_all/red_test": critical check 43 == 44 has failed [43 != 44]
/home/bobah/code/vanilla-rtb/tests/vanilla_all.cpp(18): Leaving test case "red_test"; testing time: 210us
/home/bobah/code/vanilla-rtb/tests/vanilla_all.cpp(8): Leaving test suite "vanilla_all"; testing time: 465us                                                
Leaving test module "vanilla"; testing time: 483us                                                                                                          
                                                                                                                                                            
*** 2 failures are detected in the test module "vanilla"
make[3]: *** [tests/CMakeFiles/unit-test.dir/build.make:57: tests/CMakeFiles/unit-test] Error 201                                                           
make[2]: *** [CMakeFiles/Makefile2:1093: tests/CMakeFiles/unit-test.dir/all] Error 2
make[1]: *** [CMakeFiles/Makefile2:1100: tests/CMakeFiles/unit-test.dir/rule] Error 2
make: *** [Makefile:435: unit-test] Error 2
```

### Miscelanea

For `make test` target to work in `CMake`, the top level `CMakeLists.txt` needs to have `enable_testing()` before `add_subdirectory(tests)`.

Tests (`BOOST_AUTO_TEST_CASE()`) are organized into suites (`BOOST_AUTO_TEST_SUITE()`).
Suites are grouped into `*.cpp` files, and files are grouped into `CMake` archives (`OBJECT` type libraries).
Individual suites' archives a linked directly into the launcher executable `vanilla_tests` (made of `main.cpp`).

[Boost]: https://www.boost.org
[Boost.Test]: https://www.boost.org/doc/libs/1_68_0/libs/test/doc/html/index.html
[verbose output]: https://www.boost.org/doc/libs/1_68_0/libs/test/doc/html/boost_test/test_output.html
[tests filtering]: https://www.boost.org/doc/libs/1_68_0/libs/test/doc/html/boost_test/runtime_config/test_unit_filtering.html
